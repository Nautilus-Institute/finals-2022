#include "ctx.hpp"
#include "loader.hpp"
#include "flaghasher.hpp"

#include <sched.h>
#include <cstring>
#include <signal.h>
#include <stdarg.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>


// dummy values
extern void * _stubStart;
extern void *_stubEnd;
void *stubStart = NULL;
typedef void stubCaller(void);
extern SyscallInfo_t kDefaultSyscallTable;    
extern size_t kDefaultSyscallTableLength;    

ThreadContext::ThreadContext(void)
{
  m_mm = std::make_shared<MemoryManager>();
  m_fm = std::make_shared<FileManager>();
  memset(&m_init_regs, 0, sizeof(m_init_regs));
  //vruntime = 0;

#ifdef CFS
  m_se = NULL;
  //se = new SchedEntity;
  //se->my_rq.push(this);
#endif
}

ThreadContext::~ThreadContext(void)
{
  DPRINTF("In ThreadContext destructor\n");

  // TODO deref m_mm
  // TODO deref m_scheduler
}

ThreadContext::ThreadContext(const ThreadContext & rctx)
{
  // will get overwritten in soon in the case of clone
  m_pid = rctx.m_pid;
  m_spid = rctx.m_spid;

  // will need to change if clone specifies a new VA space
  m_mm = rctx.m_mm;
  m_fm = rctx.m_fm;
  m_scheduler = rctx.m_scheduler;
  m_fs = rctx.m_fs;

  memcpy(&m_init_regs, &rctx.m_init_regs, sizeof(struct user_regs_struct));

#ifdef CFS
  // TODO : is this what we want?
  m_se = rctx.m_se;
  //se->my_rq.push(this);
  //this->GetScheduler()->m_num_tasks++;
  this->GetScheduler()->high_mark_task++;
  // TODO: do we need to ++ m_num_tasks here?
#endif

}

void ThreadContext::LoadStub(void)
{
  stubStart = (void *)NIVISOR_STUB_ADDR;
  size_t stubLength = (uintptr_t)&_stubEnd - (uintptr_t)&_stubStart;

  DPRINTF("&_stubStart %p\n", &_stubStart);
  DPRINTF("&_stubEnd %p\n", &_stubEnd);
  DPRINTF("stubLength %ld\n", stubLength);

  void *addr;
  while (stubStart > 0)
  {
    addr = mmap((void *)stubStart, stubLength, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
    if (addr != (void *)stubStart && errno != 0)
    {
      DPRINTF("Stub map unsuccessful %s\n", strerror(errno));
      munmap(addr, stubLength);
      stubStart = (void *)(((uint64_t)stubStart) - NIVISOR_PAGE_SIZE);
      continue;
    }

    memcpy(stubStart, &_stubStart, stubLength);

    if (mprotect(stubStart, stubLength, PROT_EXEC) != 0)
    {
      DPRINTF("Mprotect unsuccessful: %s\n", strerror(errno));
      fatal("Failed to LoadStub");
    }

    DPRINTF("Successfully loaded the stub at %p\n", stubStart);
    return;
  }

  fatal("Failed to find suitable address in LoadStub");
}

NIVISOR_STATUS ThreadContext::GetRegs(struct user_regs_struct *regs)
{
  if (!m_pid)
  {
    return NIVISOR_BAD_CHILD;
  }

  if (ptrace(PTRACE_GETREGS, m_pid, 0, regs) != 0)
  {
    DPRINTF("Failed to get regs from target %s\n", strerror(errno));
    return NIVISOR_INVALID_ARG;
  }

  return NIVISOR_SUCCESS;
}

NIVISOR_STATUS ThreadContext::SetRegs(struct user_regs_struct *regs)
{
  if (!m_pid)
  {
    return NIVISOR_BAD_CHILD;
  }

  if (ptrace(PTRACE_SETREGS, m_pid, 0, regs) != 0)
  {
    DPRINTF("Failed to set regs on target\n");
    return NIVISOR_INVALID_ARG;
  }

  return NIVISOR_SUCCESS;
}

NIVISOR_STATUS ThreadContext::GetSigInfo(siginfo_t *siginfo)
{
  if (!m_pid)
  {
    return NIVISOR_BAD_CHILD;
  }

  if (ptrace(PTRACE_GETSIGINFO, m_pid, 0, siginfo) != 0)
  {
    DPRINTF("Failed to get siginfo on target\n");
    return NIVISOR_INVALID_ARG;
  }

  return NIVISOR_SUCCESS;
}

NIVISOR_STATUS ThreadContext::HandlePageFault()
{
  siginfo_t siginfo;
  NIVISOR_STATUS rc = GetSigInfo(&siginfo);
  if (rc != NIVISOR_SUCCESS)
  {
      DPRINTF("Failed to get signal info for fault\n");
      return rc;
  }

  DPRINTF("Got fault at %p\n", siginfo.si_addr);

#ifdef NIVISOR_HANDLE_EXEC_FAULTS
  struct user_regs_struct regs;
  rc = GetRegs(&regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to get register for fault\n");
    return rc;
  }

  if (regs.rip == (int64_t)siginfo.si_addr)
  {
    uint64_t align_addr = ((uint64_t)siginfo.si_addr) & ~(NIVISOR_PAGE_SIZE-1);
    Vma *vma = GetMm()->VmaForPageIn((void *)align_addr);
    if (!(vma->GetProt() & PROT_EXEC))
    {
      DPRINTF("Tried to exec non-executable page\n");
      return NIVISOR_FAULT;
    }
  }
#endif

  rc = HandleFaultForPage(siginfo.si_addr);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to page in memory at %p\n", siginfo.si_addr);
  }

  return rc;
}

NIVISOR_STATUS ThreadContext::HandleFaultForPage(void *fault_addr)
{
  // we don't take ownership here so we take the raw pointer
  uint64_t align_addr = ((uint64_t)fault_addr) & ~(NIVISOR_PAGE_SIZE-1);
  Vma *vma = GetMm()->VmaForPageIn((void *)align_addr);
  NIVISOR_STATUS rc = NIVISOR_SUCCESS;
  uint64_t delta;
  int64_t res_addr;
  int prot;
  Pma *pma;

  if (vma == NULL)
  {
    rc = NIVISOR_FAULT;
    DPRINTF("Failed to find corresponding VMA for fault address %p\n", fault_addr);
    goto out;
    //return NIVISOR_FAULT;
  }

  if (!vma->MappedInBacking())
  {
    DPRINTF("VMA returned has no backing and cannot be mapped\n");
    return NIVISOR_INVALID_ARG;
  }

  prot = vma->GetProt();
  if (m_last_fault_addr == (uint64_t)fault_addr)
  {
    // Need to assume here something about VMA state that goes away
    // at exec time, like we assume the VMA is now COW?
#ifdef COWUNINIT_BUG_PATCHED
    pma = NULL;
#endif

    GetMm()->CopyPmaForWrite(vma, &pma);
    if (pma == NULL)
    {
      DPRINTF("Failed to split PMA for COW write\n");
      return NIVISOR_INVALID_ARG;
    }
    //DPRINTF("Setting PMA to %p [from %p]\n", pma, &pma);
    vma->SetPma(pma);
    vma->SetCOW(false);
    prot = vma->GetProt();
  }
  else if (vma->GetCOW())
  {
    DPRINTF("Fault on COW page %lx detected (pid %d)\n", align_addr, m_pid);
    // when this happens our objective is to first serve the read-only
    // version (if we had a bitmap we could see if this has happened and
    // skip this step in certain cases)
    prot = PROT_READ;

    // in the case where the last fault addr is fault addr we know we have
    // to make the mapping writable, but we also have to allocate a new
    // mapping
    // if (m_last_fault_addr == (uint64_t)fault_addr)
    // {
    //   DPRINTF("Fault of COW write, splitting page!\n");
    //   Pma *pma = GetMm()->CopyPmaForWrite(vma);
    //   vma->SetPma(pma);
    //   vma->SetCOW(false);
    //   prot = vma->GetProt();
    // }

    DPRINTF("[%d] Last fault addr %lx\n", GetSPid(), fault_addr);
    m_last_fault_addr = (uint64_t) fault_addr;
  }

  DPRINTF("Mapping %lx %lx (%x) into guest %d\n", align_addr, vma->GetBackingOffset(), vma->GetProt(), m_pid);
  // map in the page fault
  delta = align_addr - vma->GetAddr();
  res_addr = 0;
  if (DoSyscall(SYS_mmap,
                &res_addr,
                6,
                align_addr,
                NIVISOR_PAGE_SIZE,
                prot,
                MAP_FIXED|MAP_SHARED,
                GetMm()->GetMemFd(),
                vma->GetBackingOffset() + delta) != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to inject mmap syscall during page fault\n");
    return NIVISOR_INVALID_ARG;
  }

  DPRINTF("Res addr %lx %s\n", res_addr, strerror(abs(res_addr)));
  // MAP_FAILED doesn't work here because mmap syscall just returns errno
  if (res_addr < 0)
  {
    DPRINTF("Injected mmap into guest was unsuccessful\n");
    return NIVISOR_IO_ERROR;
  }

  //m_last_fault_addr = (uint64_t)fault_addr;
  vma->SetGuestMapped(true);

 out:
  return rc;
}

size_t ThreadContext::ReadFromUser(char *dest, char *user_src, size_t n)
{
  void *host_addr = NULL;
  size_t copy_amt = 0;
  size_t page_left = 0;
  while (n > 0)
  {
    uint64_t page_top = NIVISOR_PAGE_ALIGN((uint64_t)user_src);
    size_t page_left = page_top - (uint64_t)user_src;
    if (n > page_left)
    {
      copy_amt = page_left;
    }
    else
    {
      copy_amt = n;
    }

    DPRINTF("Attempting to copy from %p\n", user_src);
    host_addr = GetMm()->TranslateAddr(user_src, PROT_READ);
    if (host_addr == NULL)
    {
      DPRINTF("Found untranslatable request %p\n", user_src);
      goto out;
    }
    DPRINTF("Found backing at %p\n", host_addr);

    memcpy(dest, host_addr, copy_amt);
    dest += copy_amt;
    user_src += copy_amt;
    n -= copy_amt;
  }

  // TODO
 out:
  return n;
}

NIVISOR_STATUS ThreadContext::WriteToUser(char *user_dest, char *src, size_t n)
{
  void *host_addr = NULL;
  size_t copy_amt = 0;
  size_t page_left = 0;
  while (n > 0)
  {
    uint64_t page_top = NIVISOR_PAGE_ALIGN((uint64_t)user_dest);
    size_t page_left = page_top - (uint64_t)user_dest;
    if (n > page_left)
    {
      copy_amt = page_left;
    }
    else
    {
      copy_amt = n;
    }

    DPRINTF("Attempting WriteToUser to %p\n", user_dest);
    host_addr = GetMm()->TranslateAddr(user_dest, PROT_WRITE);
    if (host_addr == NULL)
    {
      DPRINTF("Found untranslatable request %p\n", user_dest);
      return NIVISOR_FAULT;
    }

    // TODO check permissions here

    DPRINTF("Writing to %p %lx from %p\n", host_addr, copy_amt, src);

    memcpy(host_addr, src, copy_amt);
    src += copy_amt;
    user_dest += copy_amt;
    n -= copy_amt;
  }

  return NIVISOR_SUCCESS;
}

ThreadContext *ThreadContext::Clone(int flags)
{
  struct user_regs_struct regs = {};

  if (GetRegs(&regs) != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to get regs for clone\n");
    return NULL;
  }

  int clone_flags = CLONE_FILES|
    CLONE_FS|
    CLONE_SIGHAND|
    CLONE_THREAD|
    CLONE_PTRACE|
    CLONE_VM;

  DPRINTF("BEFORE clone_flags %x\n", clone_flags);
  if (flags & NIVISOR_CLONE_UNSHARE_VM)
  {
    clone_flags &= ~CLONE_VM;
    clone_flags &= ~CLONE_THREAD;
    clone_flags &= ~CLONE_SIGHAND;
  }
  if (flags & NIVISOR_CLONE_UNSHARE_FS)
  {
    clone_flags &= ~CLONE_FILES;
  }

  DPRINTF("AFTER clone_flags %x\n", clone_flags);

  int64_t _new_tid = 0;
  if (DoSyscall(SYS_clone,\
                &_new_tid,\
                5,\
                clone_flags,
                regs.rsp,\
                0,\
                0,\
                0) != NIVISOR_SUCCESS)
  {
    DPRINTF("Injecting clone failed\n");
    return NULL;
  }
  pid_t new_tid = _new_tid;

  if (new_tid < 0)
  {
    DPRINTF("Clone ran but returned: %s\n", strerror(abs(new_tid)));
    return NULL;
  }

  int wstatus = 0;
  if (waitpid(new_tid, &wstatus, __WALL|WUNTRACED) != new_tid)
  {
    DPRINTF("Failed waiting for new child\n");
    return NULL;
  }

  // TODO this will throw on failure, handle it
  ThreadContext *ctx = new ThreadContext(*this);

  ctx->m_pid = new_tid;
  // consider flags here we may need to create a new file table,
  // a new VA space, a new cred struct?
  //std::unique_ptr<MemoryManager> new_mm = m_mm->CloneCOW();
  if (flags & NIVISOR_CLONE_UNSHARE_VM)
  {
    auto mm = m_mm->CloneCOW(this, ctx);
    ctx->m_mm = std::shared_ptr<MemoryManager>(mm);
    if (ctx->m_mm == nullptr)
    {
      DPRINTF("Failed to allocated new MM for child\n");
      return NULL;
    }
  }
  if (flags & NIVISOR_CLONE_UNSHARE_FS)
  {
    ctx->m_fm = std::make_shared<FileManager>();
    if (ctx->m_fm == nullptr)
    {
      DPRINTF("Failed to allocate new FM for child\n");
      return NULL;
    }
  }

  return ctx;
}

NIVISOR_STATUS ThreadContext::AllocateStack(size_t stack_size, char **args)
{
  int64_t s_addr = GetMm()->NewVma(NIVISOR_STACK_ADDR, stack_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, -1);
  if (s_addr != NIVISOR_STACK_ADDR)
  {
    DPRINTF("Failed to create VMA for stack\n");
    return NIVISOR_MEMORY_ERR;
  }

  // calculate size need to hold args and auxv
  size_t prologue_size = 0;

  size_t argc = 0;
  size_t string_space = 0;
  if (args != NULL)
  {
    for(argc = 0; args[argc] != NULL; argc++)
    {
      string_space += strlen(args[argc]) + 1;
    }
  }

  // argc argument
  prologue_size += sizeof(argc);

  // room for array pointer
  prologue_size += sizeof(void *);

  // room for arg pointers
  prologue_size += sizeof(void *) * (argc + 1);

  prologue_size += string_space;

  // TODO populate this
  FlagHasher fhasher;
  size_t auxv_size = 0;

  uint8_t flag_hash[20] = {0};
  NIVISOR_STATUS hash_rc = fhasher.GenerateHash(flag_hash);
  if (hash_rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to get flag hash\n");
    return hash_rc;
  }

  size_t auxv_struct_size = 0;

  // size for auxv value of AT_RAND
  auxv_struct_size += sizeof(auxv_t);

  // size for auxv value of AT_NULL;
  auxv_struct_size += sizeof(auxv_t);

  // size required for hash value itself
  auxv_size += auxv_struct_size + sizeof(flag_hash);

  prologue_size += auxv_size;

  prologue_size = NIVISOR_STACK_ALIGN(prologue_size);

  size_t req_pages = NIVISOR_PAGE_ALIGN(prologue_size);
  DPRINTF("Args are of size %ld and require %ld pages\n", prologue_size, req_pages);

  char *stack_start = (char *)(s_addr + stack_size - prologue_size);
  char *host_addr = (char *)GetMm()->TranslateAddr((void *)stack_start, PROT_READ|PROT_WRITE);
  if (host_addr == NULL)
  {
    DPRINTF("Failed to translate stack into host\n");
    return NIVISOR_MEMORY_ERR;
  }

  DPRINTF("Stack will be set to %p\n", stack_start);

  stack_vector_t *sv = (stack_vector_t *)host_addr;

  sv->argc = argc;
  sv->argv = (char **)(stack_start + ((char *)&sv->args - host_addr));

  DPRINTF("argc: %x\n", sv->argc);
  DPRINTF("argv: %p\n", sv->argv);

  int i = 0;
  size_t copied_len = 0;
  for (i = 0; i < argc; i++)
  {
    char *next_ptr = (char *)(stack_start +\
                              ((char *)&sv->args[argc+1] +\
                               auxv_struct_size +
                               copied_len -\
                               host_addr));

    char *next_copy = (((char *)&sv->args[argc+1]) + auxv_struct_size + copied_len);
    strcpy(next_copy, args[i]);
    copied_len += strlen(args[i]) + 1;

    sv->args[i] = next_ptr;

    DPRINTF("args[%d]: %p\n", i, next_ptr);
  }

  sv->args[argc] = NULL;

  auxv_t *auxv = (auxv_t *)&sv->args[argc+1];

  // calculate flag guest ptr
  char *flag_ptr = stack_start +\
    ((char *)&sv->args[argc+1] +\
     auxv_struct_size +\
     copied_len -\
     host_addr);

  char *flag_copy_dest = (((char *)&sv->args[argc+1]) +\
                          auxv_struct_size +\
                          copied_len);

  memcpy(flag_copy_dest, flag_hash, sizeof(flag_hash));

  auxv[0].a_type = NIVISOR_AT_RANDOM;
  auxv[0].a_value = (uint64_t)flag_ptr;

  auxv[1].a_type = NIVISOR_AT_NULL;
  auxv[1].a_value = 0;

  m_stack_start = (uint64_t)stack_start;

  return NIVISOR_SUCCESS;
}

// some decoupled register setting here to allow state update both from
// syscall context and operational 'main' function context
NIVISOR_STATUS ThreadContext::GetStackStartRegState(struct user_regs_struct *regs)
{
  if (m_stack_start == 0)
  {
    DPRINTF("Attempting to get Stack Start reg state when stack has not been allocated\n");
    return NIVISOR_INVALID_ARG;
  }

  regs->rsp = m_stack_start;
  regs->rbp = m_stack_start;

  return NIVISOR_SUCCESS;
}

NIVISOR_STATUS ThreadContext::SetStackStartState()
{
  struct user_regs_struct regs;
  NIVISOR_STATUS rc = GetRegs(&regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to get regs when attempting to set stack in reg state\n");
    return rc;
  }

  GetStackStartRegState(&regs);

  rc = SetRegs(&regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to set regs when attempting to set stack in reg state\n");
    return rc;
  }

  return rc;
}

#ifndef CFS
// deschedule ourself
void ThreadContext::Deschedule()
{
  m_scheduler->DescheduleThread(m_spid);
}
#endif

void ThreadContext::DumpRegs(struct user_regs_struct *regs)
{
  //DPRINTF("regs->orig_rax %lld\n", regs->orig_rax);
  //DPRINTF("regs->r15 %llx\n", regs->r15);
  //DPRINTF("regs->r14 %llx\n", regs->r14);
  //DPRINTF("regs->r13 %llx\n", regs->r13);
  //DPRINTF("regs->r12 %llx\n", regs->r12);
  //DPRINTF("regs->rbp %llx\n", regs->rbp);
  //DPRINTF("regs->rbx %llx\n", regs->rbx);
  //DPRINTF("regs->r11 %llx\n", regs->r11);
  //DPRINTF("regs->r10 %llx\n", regs->r10);
  //DPRINTF("regs->r8  %llx\n", regs->r8);
  //DPRINTF("regs->r9  %llx\n", regs->r9);
  //DPRINTF("regs->rax %llx\n", regs->rax);
  //DPRINTF("regs->rcx %llx\n", regs->rcx);
  //DPRINTF("regs->rdx %llx\n", regs->rdx);
  //DPRINTF("regs->rsi %llx\n", regs->rsi);
  //DPRINTF("regs->rdi %llx\n", regs->rdi);
  //DPRINTF("regs->rip %llx\n", regs->rip);
  //DPRINTF("regs->rsp %llx\n", regs->rsp);

  // do we care about segment registers? i dont think so
  // if someone finds a leak through that, that would be cool anyways
}

NIVISOR_STATUS ThreadContext::LaunchAndAttach()
{
  NIVISOR_STATUS rc = NIVISOR_SUCCESS;
  struct user_regs_struct regs = {};

  // copy stub into self
  LoadStub();

  // launch new child process with clone
  pid_t pid = (pid_t)syscall(SYS_clone, 0, 0, 0, 0, 0, 0);

  // parent?
  if (pid != 0)
  {

    DPRINTF("Child pid: %d\n", pid);
    // wait and attach
    int wstatus = 0;
    pid_t wpid = waitpid(pid, &wstatus, __WALL|WUNTRACED);
    if (wpid != pid || !WIFSTOPPED(wstatus))
    {
      DPRINTF("Launch binary did not execute correctly\n");
      rc = NIVISOR_BAD_CHILD;
      goto out;
    }

    if (ptrace(PTRACE_ATTACH, pid, 0, 0) != 0)
    {
      DPRINTF("Failed to attach emulator to launch binary %s\n", strerror(errno));
      rc = NIVISOR_BAD_CHILD;
      goto out;
    }

    wpid = waitpid(pid, &wstatus, __WALL|WUNTRACED);
    if (wpid != pid || !WIFSTOPPED(wstatus))
    {
      DPRINTF("Launch binary did not agree with ptrace\n");
      rc = NIVISOR_BAD_CHILD;
      goto out;
    }

    SetChildPid(pid);

    // ensures child dies when our tracer dies
    if (ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD|\
                                          PTRACE_O_TRACEEXIT|\
                                          PTRACE_O_EXITKILL) != 0)
    {
      DPRINTF("Failed to set options on emulator\n");
      rc = NIVISOR_BAD_CHILD;
      goto out;
    }

    // initialize regs to loader status
    rc = GetRegs(&regs);
    if (rc != NIVISOR_SUCCESS)
    {
      DPRINTF("Couldn't get reg state from child\n");
      goto out;
    }


    // stash the initial register state to context for syscall injection
    memcpy(&m_init_regs, &regs, sizeof(struct user_regs_struct));
    // adjust rip to point at the syscall instruction
    m_init_regs.rip -= 2;

  out:
    return rc;
  }

  // new process group for the child
  setsid();

  // call into stub from child
  ((stubCaller*) stubStart)();

  fatal("unreachable");
  return NIVISOR_IMPOSSIBLE;
}

NIVISOR_STATUS ThreadContext::SwitchAndExec(void)
{
  DPRINTF("[!] Switching to app logic for %d\n", m_pid);

  NIVISOR_STATUS rc = NIVISOR_SUCCESS;
  int sys_return = 0;
  struct user_regs_struct regs = {};

  int wstatus = 0;
  while (1)
  {
    if (ptrace(PTRACE_SYSEMU, m_pid, 0, 0) != 0)
    {
      DPRINTF("Failed to launch into ptrace emulator\n");
      return NIVISOR_BAD_CHILD;
    }

    if (waitpid(m_pid, &wstatus, __WALL|WUNTRACED) != m_pid)
    {
      DPRINTF("waitpid failed\n");
      return NIVISOR_BAD_CHILD;
    }

    rc = GetRegs(&regs);
    if (rc != NIVISOR_SUCCESS)
    {
      DPRINTF("[app] failed to get regs in SYSEMU loop\n");
      goto out;
    }

    if (WIFSTOPPED(wstatus))
    {
      if (WSTOPSIG(wstatus) == SIGSTOP)
      {
        // unexpected STOP? log and continue
        DPRINTF("[app] SIGSTOP encountered, ignoring..\n");
        continue;
      }
      else if (WSTOPSIG(wstatus) == (0x80 | SIGTRAP))
      {
        regs.rax = regs.orig_rax;

        DPRINTF("[app] request syscall %lld\n", regs.orig_rax);
        // do syscall emulation!
        // note the syscall instruction itself sets RCX=RIP, and R11=RFLAGS
        DumpRegs(&regs);
        if (regs.orig_rax >= kDefaultSyscallTableLength)
        {
          DPRINTF("Rejecting OOB syscall %llu/%lu\n", regs.orig_rax, kDefaultSyscallTableLength);
          regs.rax = -EPERM;
        }
        else
        {
          DPRINTF("[app] calling into syscall table\n");
          DPRINTF("syscalltable: %p\n", kDefaultSyscallTable);
          SyscallInfo_t *s_info = &(&kDefaultSyscallTable)[regs.orig_rax];    
          DPRINTF("s_info: %p\n", s_info);
          DPRINTF("s_info->fn: %p\n", s_info->fn);
          DPRINTF("s_info->n_args: %d\n", s_info->n_args);
          DPRINTF("s_info->is_terminating: %d\n", s_info->is_terminating);

          if (s_info->fn)
          {
            sys_return = s_info->fn(this, &regs);
          }
          else
          {
            sys_return = -1;
          }

          // some syscalls end the process, no need to set regs in that case
          if (sys_return == 0 && s_info->is_terminating)
          {
            // special return code to signify the scheduler that this thread
            // terminated
            rc = NIVISOR_THREAD_EXIT;
            goto out;
          }

          // all good, reflect the return code back to the caller
          regs.rax = sys_return;
        }
        regs.orig_rax = -1;
      }
      else if (WSTOPSIG(wstatus) == SIGSEGV)
      {
        // siginfo_t siginfo;
        // rc = GetSigInfo(&siginfo);
        // if (rc != NIVISOR_SUCCESS)
        // {
        //   DPRINTF("Failed to get signal info for fault\n");
        //   break;
        // }

        // DPRINTF("Got fault at %p\n", siginfo.si_addr);

        // rc = HandlePageFault(siginfo.si_addr);
        rc = HandlePageFault();
        if (rc != NIVISOR_SUCCESS)
        {
          if (rc == NIVISOR_FAULT)
          {
            DPRINTF("Process crashed, fault was legitimate\n");
          }
          goto out;
        }

      }
      else
      {
        // TODO conisder delivering signal to the app?
        DPRINTF("[app] unhandled status %x\n", WSTOPSIG(wstatus));
        return NIVISOR_SUCCESS;
      }
    }

    rc = SetRegs(&regs);
    if (rc != NIVISOR_SUCCESS)
    {
      DPRINTF("[app] failed to set registers completing syscall\n");
      goto out;
    }

    // exit out after successful emul
    break;
  }

 out:
  return rc;
}

NIVISOR_STATUS ThreadContext::DoSyscall(int sysnum, int64_t *sys_rc, size_t n_args, ...)
{
  int wstatus = 0;
  NIVISOR_STATUS rc = NIVISOR_SUCCESS;
  struct user_regs_struct saved_regs;
  struct user_regs_struct syscall_regs;

  va_list args;
  va_start(args, n_args);

  if (m_pid == 0)
  {
    return NIVISOR_BAD_CHILD;
  }

  if (sys_rc)
  {
    *sys_rc = -ENOSYS;
  }

  // stash off the old regs so we can return here
  rc = GetRegs(&saved_regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to save off register state\n");
    goto out;
  }

  memcpy(&syscall_regs, &m_init_regs, sizeof(struct user_regs_struct));

  syscall_regs.rax = sysnum;

  // begin setting args
  if (n_args >= 1)
  {
    syscall_regs.rdi = va_arg(args, uint64_t);
  }
  if (n_args >= 2)
  {
    syscall_regs.rsi = va_arg(args, uint64_t);
  }
  if (n_args >= 3)
  {
    syscall_regs.rdx = va_arg(args, uint64_t);
  }
  if (n_args >= 4)
  {
    syscall_regs.r10 = va_arg(args, uint64_t);
  }
  if (n_args >= 5)
  {
    syscall_regs.r8 = va_arg(args, uint64_t);
  }
  if (n_args >= 6)
  {
    syscall_regs.r9 = va_arg(args, uint64_t);
  }

  rc = SetRegs(&syscall_regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to reset register state to pre-syscall\n");
    goto out;
  }

  if (ptrace(PTRACE_CONT, m_pid, 0, 0) != 0)
  {
    DPRINTF("Failed to execute syscall via PTRACE_CONT: %s\n", strerror(errno));
    return NIVISOR_BAD_CHILD;
  }

  if (waitpid(m_pid, &wstatus, __WALL|WUNTRACED) != m_pid)
  {
    DPRINTF("Failed to wait for syscall exec: %s\n", strerror(errno));
    return NIVISOR_BAD_CHILD;
  }

  if (WSTOPSIG(wstatus) != SIGTRAP)
  {
    DPRINTF("Received unexpected stop signal during syscall injection\n");
    return NIVISOR_BAD_CHILD;
  }

  // retrieve the return code
  // NB okay to overwrite out syscall_regs here, they were just a launch pad
  rc = GetRegs(&syscall_regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to retrieve post-syscall state for return code\n");
    goto out;
  }

  // set that return code!
  if (sys_rc)
  {
    *sys_rc = syscall_regs.rax;
  }

  // finally restore the old registers
  rc = SetRegs(&saved_regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to restore original state to tracee post-syscall\n");
    goto out;
  }

 out:
  return rc;
}

