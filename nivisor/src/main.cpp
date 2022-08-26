#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>

#include "ctx.hpp"
#include "syscall.h"
#include "loader.hpp"
#include "bundlefs.hpp"
#include "scheduler.hpp"
#include "stdnivisor.h"

extern SyscallInfo_t kDefaultSyscallTable;
extern size_t kDefaultSyscallTableLength; 
#ifdef POOL                               
#include "pool.h"                         
char heap[HEAP_SIZE] = {-1};              
#ifdef FART                               
char bs[8*64 - 8 - 8] = {1};              
#else                                     
char bs[1024] = {1};                                                                                  
#endif                                    
#endif                                    

#ifdef CFS
CFSScheduler *g_Scheduler = NULL;
#endif

void fatal(const char *s)
{
  fprintf(stderr, "%s\n", s);
  exit(1);
}

int readline(int fd, char *buf, size_t n)
{
  size_t i = 0;
  for (i = 0; i < n; i++)
  {
    if (read(fd, &buf[i], 1) < 0)
    {
      DPRINTF("Failed to read\n");
      return -1;
    }

    if (buf[i] == '\n')
    {
      buf[i] = '\0';
      return 0;
    }
  }

  return -1;
}

void killKids(int signum)
{
#ifdef CFS
  DPRINTF("Killing kids %p\n", g_Scheduler);
  g_Scheduler->KillTasks();
#endif
}

int readArgs(int pipefd, char **rootfs, char **executable, char ***args)
{
  unsigned handshake = 0;
  if (read(pipefd, &handshake, sizeof(handshake)) != sizeof(handshake))
  {
    DPRINTF("Failed to read handshake %s\n", strerror(errno));
    return -1;
  }

  if (handshake != 0x4e415554)
  {
    DPRINTF("Got wrong hanshake value\n");
    return -1;
  }

  char local_rootfs[PATH_MAX] = {0};
  if (readline(pipefd, local_rootfs, sizeof(local_rootfs)) < 0)
  {
    DPRINTF("Failed to read rootfs\n");
    return -1;
  }

  *rootfs = strdup(local_rootfs);
  if (*rootfs == NULL)
  {
    DPRINTF("Failed to strdup rootfs\n");
    return -1;
  }

  char local_executable[PATH_MAX] = {0};
  if (readline(pipefd, local_executable, sizeof(local_executable)) < 0)
  {
    DPRINTF("Failed to read executable\n");
    return -1;
  }

  *executable = strdup(local_executable);
  if (*executable == NULL)
  {
    DPRINTF("Failed to strdup executable\n");
    return -1;
  }

  char arg_count[8] = {0};
  if (readline(pipefd, arg_count, sizeof(arg_count)) < 0)
  {
    DPRINTF("Failed to read n args\n");
    return -1;
  }

  size_t n_args = atoi(arg_count);
  if (n_args > 30)
  {
    DPRINTF("Too many args specified\n");
    return -1;
  }

  n_args += 1;

  // +1 for final NULL
  char **new_args = (char **)malloc((n_args + 1) * sizeof(char *));
  if (new_args == NULL)
  {
    DPRINTF("Failed to allocate space for arg vector\n");
    return -1;
  }

  new_args[0] = *executable;

  size_t i = 0;
  for (i = 1; i < n_args; i++)
  {
    char local_arg[1024] = {0};
    if (readline(pipefd, local_arg, sizeof(local_arg)) < 0)
    {
      DPRINTF("Failed to read arg\n");
      return -1;
    }

    new_args[i] = strdup(local_arg);
    if (new_args[i] == NULL)
    {
      DPRINTF("Failed to strdup\n");
      return -1;
    }
  }
  new_args[i] = NULL;

  *args = new_args;

  return 0;
}

int main(int argc, char **argv)
{
  ThreadContext *ctx;

  if (argc < 3)
  {
    fprintf(stderr, "%s [--read-args <pipefd>|<rootfs> <executable>]\n", argv[0]);
    return 1;
  }

  char *rootfs = NULL;
  char *executable = NULL;
  char **args = NULL;
  if (!strcmp(argv[1], "--read-args"))
  {
    int pipefd = atoi(argv[2]);

    DPRINTF("Accepting args over pipefd %d\n", pipefd);
    if (readArgs(pipefd, &rootfs, &executable, &args) < 0)
    {
      DPRINTF("Failed to read args over stdin\n");
      return 1;
    }

    close(pipefd);
  }
  else
  {
    rootfs = argv[1];
    executable = argv[2];

    args = &argv[2];
  }

  // file system and scheduler's lifetime is tied to main
  BundleFs bfs(rootfs);
#ifdef CFS
#ifdef POOL
  CFSScheduler scheduler(NIVISOR_MAX_PROCESSES, heap);
#else
  CFSScheduler scheduler(NIVISOR_MAX_PROCESSES);
#endif
#else
   Scheduler scheduler;
#endif


  // this is Pid 1
  // allocate the first thread on the heap to prevent double free it requests its own deletion
  ctx = new ThreadContext;
  // we only account for this in the copy constructor
#ifdef CFS
  scheduler.high_mark_task++;
#endif
  ctx->SetFs(&bfs);

  std::unique_ptr<Loader> loader = LoadBinary(*ctx, executable);
  if (loader == NULL || loader->GetLoadError() != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to load process binary\n");
    return 1;
  }

  if (ctx->LaunchAndAttach() != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to place process in ptrace harness\n");
    return 1;
  }

  if (loader->SetEntrypointState(ctx) != NIVISOR_SUCCESS) {
    DPRINTF("Failed to set entry state for process\n");
    return 1;
  }

  if (ctx->AllocateStack(NIVISOR_STACK_SIZE, args) != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to set stack for process\n");
    return 1;
  }

  if (ctx->SetStackStartState() != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to set stack start state for process\n");
    return 1;
  }

  // push our initial process, scheduler takes ownership of ThreadContext
  scheduler.ScheduleThread(ctx);
#ifdef CFS
  scheduler.m_num_tasks++;
#endif

  g_Scheduler = GetScheduler();
  DPRINTF("Scheduler: %p\n", g_Scheduler);

  if (signal(SIGSEGV, killKids) == SIG_ERR)
  {
    DPRINTF("Failed to set signal handler for SIGSEGV\n");
    return NIVISOR_BAD_CHILD;
  }

  scheduler.Run();

  DPRINTF("All threads exited\n");

  return 0;
}
