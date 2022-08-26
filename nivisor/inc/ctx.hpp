#pragma once

#include <memory>

#include <signal.h>
#include <sys/user.h>
#include <queue>

#include "fm.hpp"
#include "mm.hpp"
#include "syscall.h"
#include "loader.hpp"
#include "bundlefs.hpp"
#include "scheduler.hpp"

#define NIVISOR_STUB_ADDR 0x7fffffff0000

#ifdef CFS
class ThreadContext;
// this is how tasks are grouped
// It's the granularity of scheduling
// (can also be a single task however)
class SchedEntity {
  public:
    uint64_t se_slice;
    uint64_t num_tasks;
    uint64_t id;
    //ThreadContext **m_parent_rq;
    //ThreadContext **m_rq;
    std::queue<ThreadContext *> my_rq;

    // amount of faults + syscalls a task has had (how we're calc'ing runtime)
    uint64_t vruntime;
};
#endif

class ThreadContext {
 public:
  ThreadContext();
  ~ThreadContext();
  ThreadContext(const ThreadContext  & rctx);

  void SetMm(std::shared_ptr<MemoryManager> mm) { m_mm = mm; };
#ifdef CFS
  void SetSchedEntity(SchedEntity *sse) {m_se = sse;};
  void SetScheduler(CFSScheduler *scheduler) { m_scheduler = scheduler; };
  void Kms() { DPRINTF("Kill %d\n", m_pid); kill(m_pid, 9); }
#else
  void SetScheduler(Scheduler *scheduler) { m_scheduler = scheduler; };
#endif
  void SetFs(BundleFs *fs) { m_fs = fs; }

  void SetSpid(pid_t spid) { m_spid = spid; };

  std::shared_ptr<MemoryManager> GetMm() { return m_mm; };
#ifdef CFS
  SchedEntity *GetSchedEntity() { return m_se; };
  CFSScheduler *GetScheduler() { return m_scheduler; };
#else
  Scheduler *GetScheduler() { return m_scheduler; }
#endif
  BundleFs *GetFs() { return m_fs; }
  std::shared_ptr<FileManager> GetFm() { return m_fm; }

  pid_t GetPid() { return m_pid; };
  pid_t GetSPid() { return m_spid; }
  int64_t GetStackStart() { return m_stack_start; }

  NIVISOR_STATUS LaunchAndAttach();
  NIVISOR_STATUS SwitchAndExec(void);
  NIVISOR_STATUS DoSyscall(int sysnum, int64_t *sys_rc, size_t n_args, ...);

  NIVISOR_STATUS GetRegs(struct user_regs_struct *regs);
  NIVISOR_STATUS SetRegs(struct user_regs_struct *regs);
  NIVISOR_STATUS GetSigInfo(siginfo_t *siginfo);
  void DumpRegs(struct user_regs_struct *regs);

  NIVISOR_STATUS HandleFaultForPage(void *fault_addr);
  NIVISOR_STATUS HandlePageFault();
  size_t ReadFromUser(char *, char *, size_t);
  NIVISOR_STATUS WriteToUser(char *, char *, size_t);

  ThreadContext *Clone(int flags);
  NIVISOR_STATUS AllocateStack(size_t stack_size, char **args);
  NIVISOR_STATUS GetStackStartRegState(struct user_regs_struct *regs);
  NIVISOR_STATUS SetStackStartState();
  void Deschedule(void);

#ifdef CFS
  // scheduling entity task is associated with
  SchedEntity *m_se;
#endif

 private:
  // tracee real pid
  pid_t m_pid;
  // sandboxed pid
  pid_t m_spid;
  // stack starting position
  int64_t m_stack_start;
  // last fault address, used for COW management
  uint64_t m_last_fault_addr;

  std::shared_ptr<MemoryManager> m_mm;
  // scheduler owns all ThreadContexts, we dont want to imply we own
  // the scheduler with a smart pointer?
#ifdef CFS
  CFSScheduler *m_scheduler;
#else
  Scheduler *m_scheduler;
#endif
  // this is assumed to exist through
  BundleFs *m_fs;

  std::shared_ptr<FileManager> m_fm;

  // syscall injection
  struct user_regs_struct m_init_regs;

  void LoadStub(void);
  void SetChildPid(pid_t pid) { m_pid = pid; };
};

