#pragma once

#include <vector>
#include <memory>

#include <time.h>
#include <unistd.h>
#include <signal.h>

class ThreadContext;

// Basic thread scheduler
class Scheduler {
 public:
  Scheduler()
    : m_last_spid(0)
  {}

  pid_t ScheduleThread(ThreadContext *ctx);
  void DescheduleThread(pid_t spid);
  pid_t GetNextSpid() { return ++m_last_spid; }
  bool CanScheduleMore() { return m_threads.size() < NIVISOR_MAX_PROCESSES; }

  void Run();

 private:
  int m_last_spid;
  std::vector<pid_t> m_spids;
  std::map<pid_t, ThreadContext *> m_threads;
};

#ifdef CFS

#include "myrb.h"
class SchedEntity;

/* ######################### */
/* #  CFS SCHEDULER BEGIN  # */
/* ######################### */

// CFS scheduler
class CFSScheduler {
  public:
#ifdef POOL
    CFSScheduler(uint64_t sched_period, void *heap_mem);
#else
    CFSScheduler(uint64_t sched_period);
#endif
    pid_t ScheduleThread(ThreadContext *ctx, bool is_cloned=false);
    //void DescheduleThread(pid_t spid);
    
    //bool CanScheduleMore() { return m_rbtree->m_size < NIVISOR_MAX_PROCESSES; }
    //bool CanScheduleMore() { return m_num_tasks < NIVISOR_MAX_PROCESSES; }
    bool CanScheduleMore() { 
      bool more_pls = m_num_tasks < NIVISOR_MAX_PROCESSES;
      if (more_pls)
        DPRINTF("Allowing more tasks\n");
      else {
        DPRINTF("Not Allowing more tasks. Already have %d\n", m_num_tasks);
      }
      return more_pls;
    }
    void SetNextSe(SchedEntity *se) { m_next_se = se; }
    SchedEntity *GetNextSe(void) { return m_next_se; }
    void KillTasks();

    void Run();
    void RunEntity();

    std::unique_ptr<RBTree<uint64_t, SchedEntity *>> m_rbtree;
    uint64_t m_num_tasks;
    uint64_t high_mark_task;
  private:
    // period during which all tasks should run at least once
    // for us it will be measured in # of syscalls + faults
    uint64_t m_sched_period;
    uint64_t m_slice;
    SchedEntity *m_next_se;
};
#endif
