#include "ctx.hpp"
#include "syscall.h"
#include "stdnivisor.h"

#include <sched.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#ifdef POOL
#include "pool.h"
#endif

// all syscall implementations live here for now
// to add a new syscall, define it here and add it to the
// syscall table in syscalltable.cpp

#ifdef CFS
int sys_setsid(ThreadContext *ctx, struct user_regs_struct *regs) {
  // null out our se field and schedule ourselves
  // scheduler will make a new sched entity for us
  // TODO: How to handle a now potentially empty se?
  uint64_t id = regs->rdi;

  DPRINTF("in setssid, num nodes is currently: %d\n",ctx->GetScheduler()->m_rbtree->m_size);
  //ctx->GetScheduler()->m_rbtree->m_mpool->Print();
  if (ctx->GetScheduler()->m_rbtree->m_size >= NIVISOR_MAX_PROCESSES-1) {
    return NIVISOR_NO_SPACE;
  }
  SchedEntity *next_se = new SchedEntity;
  next_se->id = id;
  next_se->vruntime = ctx->m_se->vruntime;
  ctx->GetScheduler()->SetNextSe(next_se);
  return 0;
}
#endif

int sys_exit(ThreadContext *ctx, struct user_regs_struct *regs)
{
  // good enough?
  int code = (int)regs->rdi;
  DPRINTF("Thread %d requested exit with code %d\n", ctx->GetSPid(), code);

  // we inject a syscall here so only the calling thread departs
  int _notused = 0;
  if (ctx->DoSyscall(SYS_exit, NULL, 0) != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to request thread exit via syscall injection\n");
    return -1;
  }

  // 0 conveys the process exited successfully to the caller
  // caller will be responsible for descheduling the thread
  return 0;
}

// Fork the process, we call it 'split'ing
int sys_split_proc(ThreadContext *ctx, struct user_regs_struct *regs)
{
  DPRINTF("Splitting process\n");

  int flags = (int)regs->rdi;

  if (!ctx->GetScheduler()->CanScheduleMore())
  {
    DPRINTF("Reached process limit, refusing to create new proc\n");
    return -1;
  }

  // perform the syscall injection of clone(2) and wraps it in a context
  ThreadContext *lctx = ctx->Clone(flags);
  if (lctx == NULL)
  {
    DPRINTF("Failed to clone ThreadContext\n");
    return -1;
  }

  DPRINTF("Create new child with tid %d\n", lctx->GetPid());

  struct user_regs_struct new_regs = {};

  memcpy(&new_regs, regs, sizeof(struct user_regs_struct));

  // update the child's registers to reflect that it was just created
  new_regs.rax = 0;
  if (lctx->SetRegs(&new_regs) != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to set regs for child\n");
    return -1;
  }

  // schedule the thread
#ifdef CFS
  pid_t spid = ctx->GetScheduler()->ScheduleThread(lctx, true);
#else
  pid_t spid = ctx->GetScheduler()->ScheduleThread(lctx);
#endif
  //printf("Returning: %d to parent\n", spid);
  return spid;
}

// Get 'sandboxed' pid
int sys_get_spid(ThreadContext *ctx, struct user_regs_struct *regs)
{
  return ctx->GetSPid();
}

#define CHAR_CACHE_SIZE 512
#define STDOUT_FD 1
int sys_write_console(ThreadContext *ctx, struct user_regs_struct *regs)
{

  char *buf = (char *)regs->rdi;
  size_t sz = (size_t)regs->rsi;

  DPRINTF("Writing %p of %ld bytes to the console\n", buf, sz);

  size_t chunk_sz = 0;
  char cache[CHAR_CACHE_SIZE] = {0};
  while (sz > 0)
  {
    if (sz > CHAR_CACHE_SIZE)
    {
      chunk_sz = CHAR_CACHE_SIZE;
    }
    else
    {
      chunk_sz = sz;
    }

    size_t amt_left = ctx->ReadFromUser(cache, buf, chunk_sz);
    if (amt_left == chunk_sz)
    {
      DPRINTF("Failed to read %ld bytes from user memory at %p\n", chunk_sz, buf);
      return -NIVISOR_FAULT;
    }

    if (write(STDOUT_FD, cache, chunk_sz - amt_left) < CHAR_CACHE_SIZE)
    {
      return sz;
    }

    buf += chunk_sz;
    sz -= chunk_sz;
  }

  return 0;
}

#define STDIN_FD 0
int sys_read_console(ThreadContext *ctx, struct user_regs_struct *regs)
{
  char *buf = (char *)regs->rdi;
  size_t sz = (size_t)regs->rsi;

  DPRINTF("Read %p of %ld bytes from console\n", buf, sz);

  size_t amt_read = 0;
  size_t chunk_sz = 0;
  char cache[CHAR_CACHE_SIZE] = {0};
  while (sz > 0)
  {
    if (sz > CHAR_CACHE_SIZE)
    {
      chunk_sz = CHAR_CACHE_SIZE;
    }
    else
    {
      chunk_sz = sz;
    }

    amt_read = read(STDIN_FD, cache, chunk_sz);
    if (amt_read < 0)
    {
      return sz;
    }

    if (ctx->WriteToUser(buf, cache, amt_read) != NIVISOR_SUCCESS)
    {
      DPRINTF("Failed to write %ld bytes to user memory %p\n", chunk_sz, buf);
      return -NIVISOR_FAULT;
    }

    buf += chunk_sz;
    sz -= chunk_sz;
  }

  return 0;
}

int sys_new_anon_region(ThreadContext *ctx, struct user_regs_struct *regs)
{
  int64_t rc = 0;

  uint64_t addr = regs->rdi;
  size_t length = regs->rsi;
  int prot      = regs->rdx;

  DPRINTF("New anonymous region requested at %lx, %lx, %x\n", addr, length, prot);

  if (length > NIVISOR_MAX_MAP_LENGTH)
  {
    DPRINTF("Invalid length passed in\n");
    return -NIVISOR_INVALID_ARG;
  }

  if (prot & ~(PROT_READ|PROT_WRITE|PROT_EXEC))
  {
    DPRINTF("Invalid protection flags specified\n");
    return -NIVISOR_INVALID_ARG;
  }

  rc = ctx->GetMm()->NewVma(addr, length, prot, MAP_PRIVATE|MAP_ANONYMOUS, -1);
  DPRINTF("New VMA at %ld\n", rc);
  if (rc < 0)
  {
    DPRINTF("Failed to create new VMA for request\n");
    return rc;
  }

  return rc;
}

int sys_exec_proc(ThreadContext *ctx, struct user_regs_struct *regs)
{
  int i = 0;
  size_t argc = 0;
  NIVISOR_STATUS rc = NIVISOR_SUCCESS;
  char **copied_args[NIVISOR_MAX_ARGS+1] = {0};
  char *copied_in_args[NIVISOR_MAX_ARGS+1] = {0};
  std::unique_ptr<Loader> loader = NULL;

  char *path = (char *)regs->rdi;
  char **args = (char **)regs->rsi;

  DPRINTF("sys_exec_proc(%p, %p)\n", path, args);

  size_t amt_left = 0;
  char bin_path[256] = {0};
  amt_left = ctx->ReadFromUser(bin_path, path, sizeof(bin_path));
  if (amt_left == sizeof(bin_path))
  {
    DPRINTF("Failed to read out path string\n");
    return -NIVISOR_FAULT;
  }
  bin_path[sizeof(bin_path)-1] = '\0';

  // prepare to copy in args
  amt_left = ctx->ReadFromUser((char *)copied_args, (char *)args, sizeof(copied_args));
  if (amt_left == sizeof(copied_args))
  {
    DPRINTF("Failed to copy out args array\n");
    return -NIVISOR_FAULT;
  }

  for (argc = 0; copied_args[argc]; argc++)
  {
    copied_in_args[argc] = (char *)malloc(NIVISOR_MAX_ARG_LEN);
    if (copied_in_args[argc] == NULL)
    {
      rc = -NIVISOR_MEMORY_ERR;
      goto out;
    }

    amt_left = ctx->ReadFromUser((char *)copied_in_args[argc], (char *)copied_args[argc], NIVISOR_MAX_ARG_LEN);
    if (amt_left == NIVISOR_MAX_ARG_LEN)
    {
      rc = -NIVISOR_FAULT;
      goto out;
    }

    copied_in_args[argc][NIVISOR_MAX_ARG_LEN-1] = '\0';
  }
  copied_in_args[argc+1] = NULL;

  // TODO clean up child threads which are staying the MM?
  // Destroy all VMA regions
  ctx->GetMm()->Clear(ctx);

  // Load binary
  loader = LoadBinary(*ctx, bin_path);
  if (loader == NULL || loader->GetLoadError() != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to load binary image\n");
    return NIVISOR_INVALID_ARG;
  }

  // Set initial register state
  if (loader->GetEntrypointRegState(regs) != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to set entry state for image\n");
    return NIVISOR_IO_ERROR;
  }

  // setup stack with args
  if (ctx->AllocateStack(NIVISOR_STACK_SIZE, copied_in_args) != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to set stack in image load\n");
    return NIVISOR_IO_ERROR;
  }

  if (ctx->GetStackStartRegState(regs) != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to reg state for new stack\n");
    return NIVISOR_IO_ERROR;
  }

 out:
  for(i = 0; i < argc; i++)
  {
    free(copied_in_args[argc]);
  }

  return rc;
}

int sys_open_file(ThreadContext *ctx, struct user_regs_struct *regs)
{
  char *path = (char *)regs->rdi;
  int flags = regs->rsi;

  DPRINTF("open(%p, %d)\n", path, flags);

  char file_path[256] = {0};
  size_t amt_left = ctx->ReadFromUser(file_path, path, sizeof(file_path));
  if (amt_left == sizeof(file_path))
  {
    DPRINTF("Failed to read out path string\n");
    return -NIVISOR_FAULT;
  }
  file_path[sizeof(file_path)-1] = '\0';

  int nix_flags = 0;

  if (flags == 0)
  {
    DPRINTF("Invalid flags argument of 0\n");
    return -NIVISOR_INVALID_ARG;
  }

  if (flags & NIVISOR_READ)
  {
    nix_flags |= O_RDONLY;
  }

  if (flags & NIVISOR_WRITE)
  {
    if (nix_flags)
    {
      nix_flags |= O_RDWR;
    }
    else
    {
      nix_flags |= O_WRONLY;
    }
  }

  int fd = ctx->GetFs()->Open(file_path, nix_flags);
  if (fd < 0)
  {
    DPRINTF("Failed to open file %s\n", file_path);
    return -NIVISOR_IO_ERROR;
  }


  // TODO: do more here? have some kind of fake file struct
  int sfd = ctx->GetFm()->PutFile(fd);
  if (sfd < 0)
  {
    DPRINTF("Failed to insert file into file table\n");
    close(fd);
    return -NIVISOR_IO_ERROR;
  }

  DPRINTF("Opened new file at %d -> %d\n", sfd, fd);

  return sfd;
}

int sys_read_file(ThreadContext *ctx, struct user_regs_struct *regs)
{
  int sfd = regs->rdi;
  char *buf = (char *)regs->rsi;
  size_t n = regs->rdx;

  DPRINTF("read(%d, %p, %lx)\n", sfd, buf, n);

  int fd = ctx->GetFm()->GetFile(sfd);
  if (fd < 0)
  {
    DPRINTF("Failed to resolve fd %d\n", sfd);
    return fd;
  }

  size_t sz = n;
  size_t amt_read = 0;
  size_t chunk_sz = 0;
  size_t total_read = 0;
  char cache[CHAR_CACHE_SIZE] = {0};
  while (n > 0)
  {
    if (n > CHAR_CACHE_SIZE)
    {
      chunk_sz = CHAR_CACHE_SIZE;
    }
    else
    {
      chunk_sz = sz;
    }

    amt_read = read(fd, cache, chunk_sz);
    if (amt_read < 0)
    {
      return sz - n;
    }

    // total up all the bytes read
    total_read += amt_read;

    if (ctx->WriteToUser(buf, cache, amt_read) != NIVISOR_SUCCESS)
    {
      DPRINTF("Failed to write %ld bytes to user memory %p\n", chunk_sz, buf);
      return -NIVISOR_FAULT;
    }

    // after we've written the content into the guest, break if
    // there was a short read
    if (amt_read < chunk_sz)
    {
      break;
    }

    buf += chunk_sz;
    n -= chunk_sz;
  }

  return total_read;
}

int sys_write_file(ThreadContext *ctx, struct user_regs_struct *regs)
{
  int sfd = regs->rdi;
  char *buf = (char *)regs->rsi;
  size_t n = regs->rdx;

  DPRINTF("write(%d, %p, %lx)\n", sfd, buf, n);

  int fd = ctx->GetFm()->GetFile(sfd);
  if (fd < 0)
  {
    DPRINTF("Failed to resolve fd %d\n", sfd);
    return fd;
  }

  size_t sz = n;
  size_t chunk_sz = 0;
  char cache[CHAR_CACHE_SIZE] = {0};
  while (n > 0)
  {
    if (n > CHAR_CACHE_SIZE)
    {
      chunk_sz = CHAR_CACHE_SIZE;
    }
    else
    {
      chunk_sz = n;
    }

    size_t amt_left = ctx->ReadFromUser(cache, buf, chunk_sz);
    if (amt_left == chunk_sz)
    {
      DPRINTF("Failed to read %ld bytes from user memory at %p\n", chunk_sz, buf);
      return -NIVISOR_FAULT;
    }

    size_t wrote = write(fd, cache, chunk_sz - amt_left);
    if (wrote < 0)
    {
      return -NIVISOR_IO_ERROR;
    }

    if (wrote < chunk_sz - amt_left)
    {
      return (sz - n) + wrote;
    }

    buf += chunk_sz;
    n -= chunk_sz;
  }

  return sz;
}

int sys_close_file(ThreadContext *ctx, struct user_regs_struct *regs)
{
  int sfd = regs->rdi;

  DPRINTF("close(%d)\n", sfd);

  // sanity check fd
  int fd = ctx->GetFm()->GetFile(sfd);
  if (fd < 0)
  {
    DPRINTF("Failed to resolved fd %d\n", sfd);
    return fd;
  }

  return ctx->GetFm()->RemoveFile(sfd);
}

#ifndef NIVISOR_REMOVE_MPROTECT
int sys_change_region_prot(ThreadContext *ctx, struct user_regs_struct *regs)
{
  uint64_t addr = regs->rdi;
  int flags     = regs->rsi;
  size_t length = regs->rdx;

  DPRINTF("change_region_prot(%lx, %x, %lx)\n", addr, flags, length);

  // Buggy code below
  Vma *vma = ctx->GetMm()->VmaForAddr((void *)addr);
  if (vma == NULL)
  {
    DPRINTF("Could not find VMA for addr %lx\n", addr);
    return -NIVISOR_NO_ELEM;
  }

  // TODO could also wrap patch around length check
  // some concern that teams may worry about checking Addr being a
  // a functionality break?
#ifdef MPROTECT_BUG_PATCHED
  if (addr != vma->GetAddr())
  {
    DPRINTF("Addr argument specified did not match VMA start %lx\n", addr);
    return -NIVISOR_INVALID_ARG;
  }
#endif

  if (length != vma->GetLength())
  {
    DPRINTF("Length argument specified did not match VMA length %lx\n", length);
    return -NIVISOR_INVALID_ARG;
  }

  if (flags & (~(PROT_READ|PROT_WRITE|PROT_EXEC)))
  {
    DPRINTF("Invalid flag value %x specified\n", flags);
    return -NIVISOR_INVALID_ARG;
  }

  int64_t _rc = 0;
  if (ctx->DoSyscall(SYS_mprotect,
                     &_rc,
                     3,
                     addr,
                     length,
                     flags) != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to inject syscall for mprotect\n");
    return -NIVISOR_IO_ERROR;
  }

  if (_rc != 0)
  {
    DPRINTF("Mprotect did not return success %s\n", strerror(abs(_rc)));
    return -NIVISOR_IO_ERROR;
  }

  vma->SetProt(flags);

  return 0;
}
#endif
