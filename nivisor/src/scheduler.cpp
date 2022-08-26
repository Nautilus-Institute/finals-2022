#include "ctx.hpp"
#include "scheduler.hpp"
#include <cassert>

#ifndef CFS

pid_t Scheduler::ScheduleThread(ThreadContext *ctx)
{
  pid_t spid = GetNextSpid();

  ctx->SetSpid(spid);
  ctx->SetScheduler(this);

  m_threads[spid] = ctx;
  m_spids.push_back(spid);

  return spid;
}

void Scheduler::DescheduleThread(pid_t spid)
{
  m_threads.erase(spid);

  auto it = m_spids.begin();
  for (it; it != m_spids.end(); it++)
  {
    if (*it == spid)
    {
      m_spids.erase(it);
      return;
    }
  }
}

void Scheduler::Run()
{
  NIVISOR_STATUS rc = NIVISOR_SUCCESS;

  uint32_t cycle = 0;
  for(cycle = 0; m_threads.size() > 0; cycle++)
  {
    // close enough to round robin for my tastes
    size_t next_thread = cycle % m_spids.size();
    pid_t next_spid = m_spids[next_thread];
    rc = m_threads[next_spid]->SwitchAndExec();

    if (rc == NIVISOR_THREAD_EXIT)
    {
      DPRINTF("Scheduler observed thread exit, descheduling...\n");
      DescheduleThread(next_spid);
      continue;
    }
    if (rc == NIVISOR_FAULT)
    {
      DPRINTF("Scheduler observed thread crash, descheduling...\n");
      DescheduleThread(next_spid);
      continue;
    }
    if (rc != NIVISOR_SUCCESS)
    {
      DPRINTF("Scheduler encountered a failed context switch\n");
      return;
    }
  }
}

#else

/* ######################### */
/* #  CFS SCHEDULER BEGIN  # */
/* ######################### */

#ifdef POOL
CFSScheduler::CFSScheduler(uint64_t sched_period, void *heap) {
#else
CFSScheduler::CFSScheduler(uint64_t sched_period) {
#endif
  m_sched_period = sched_period;
  // when we start, out slice is equal to the sched_period since we have no tasks
  m_slice = sched_period;
  // key will be vruntime
#ifdef POOL
  m_rbtree = std::make_unique<RBTree<uint64_t, SchedEntity *>>(heap);
#else
  m_rbtree = std::make_unique<RBTree<uint64_t, SchedEntity *>>();
#endif
  DPRINTF("m_rbtree: %p\n", m_rbtree.get());
  m_next_se = NULL;
  m_num_tasks = 0;
  high_mark_task = 0;
}

/*
ThreadContext *CFSScheduler::DescheduleThread(RBNode<uint64_t, SchedEntity *> node) {

}
*/

pid_t CFSScheduler::ScheduleThread(ThreadContext *ctx, bool is_cloned) {
  pid_t spid = m_num_tasks + 1;
  ctx->SetSpid(spid);
  ctx->SetScheduler(this);
  SchedEntity *se = ctx->GetSchedEntity();
  if (se == NULL) {
    DPRINTF("Creating a new sched entity\n");
    //std::unique_ptr<SchedEntity> se = std::make_unique<SchedEntity>();
    se = new SchedEntity;
    ctx->SetSchedEntity(se);
  }

  // We could be pushing duplicate SEs onto the tree if we don't check this
  // only called from (A) initial task and (B) setsid 
  if (!is_cloned) {
#ifdef FART
    m_rbtree->add(se->vruntime, se, se->id);
#else
    m_rbtree->add(se->vruntime, se);
#endif
  }
  else {
    m_num_tasks++;
  }
  // we don't account for this in clone constructor
  se->my_rq.push(ctx);
  DPRINTF("[add] m_num_tasks is now: %d\n", m_num_tasks);
  if (m_num_tasks > 30) {
    abort();
  }

  return spid;
}

void CFSScheduler::KillTasks()
{
  for (uint64_t cycles=0; m_rbtree->m_size > 0; cycles++)
  {
    DPRINTF("Attempting to kill children\n");
    RBNode<uint64_t, SchedEntity *> *node = m_rbtree->removeLeftmostNode();
    uint32_t num_tasks = node->m_val->my_rq.size();
    int64_t iteration = 0;
    for (iteration = 0; iteration < num_tasks; iteration++)
      {
       ThreadContext *to_run = node->m_val->my_rq.front();
       node->m_val->my_rq.pop();
       to_run->Kms();
     }
   }
}

void CFSScheduler::Run()
{
  NIVISOR_STATUS rc = NIVISOR_SUCCESS;
  uint32_t num_entities = m_rbtree->m_size;
  uint64_t slice_per_entity = m_sched_period / num_entities;
  DPRINTF("[+] num_entities: %d\n", num_entities);
  // loop as long as our rbtree has entities
  for (uint64_t cycles=0; m_rbtree->m_size > 0; cycles++) {
    //printf("m_rbtree->m_size: %d\n", m_rbtree->m_size);
    DPRINTF("[+] num_entities: %d\n", m_rbtree->m_size);
    RBNode<uint64_t, SchedEntity *> *node = m_rbtree->removeLeftmostNode();
    uint32_t num_tasks = node->m_val->my_rq.size();
    // for each entity, we run a task for a slice of our entities slice
    for (int y=0; y < num_tasks; y++) {
      ThreadContext *to_run = node->m_val->my_rq.front();
      node->m_val->my_rq.pop();
      // divide our entities slice into a slice per task we have
      uint64_t slice_per_task = slice_per_entity / num_tasks;
      DPRINTF("num_tasks: %d\n", num_tasks);
      DPRINTF("slice_per_task is: %ld\n", slice_per_task);
      DPRINTF("Running node: %p with se %p and task: %p | vruntime: %ld\n", node, node->m_val, to_run, node->m_val->vruntime);
      //node->print();
      int j;
      for (j=0; j < slice_per_task; j++) {
        rc = to_run->SwitchAndExec();
        if (rc == NIVISOR_THREAD_EXIT)
        {
          DPRINTF("Scheduler observed thread exit, descheduling...\n");
          // TODO: update the time slices for the remaining tasks here
          break;
        }
        if (rc == NIVISOR_FAULT)
        {
          DPRINTF("Scheduler observed thread fault, descheduling...\n");
          // TODO: update the time slices for the remaining tasks here
          break;
        }

      } // end slice_per_task
      // if our thread exited, don't add it back to our task queue
      if (rc == NIVISOR_SUCCESS) {
        // check if we have a new se
        // TODO: do we want to adjust the slice_per_task if our task exists?
        DPRINTF("Adding thread back to our rq\n");
        node->m_val->my_rq.push(to_run);
      }

      else {
        if (GetNextSe() != NULL) {
          assert(rc == NIVISOR_THREAD_EXIT);
          DPRINTF("got a new se %p\n", GetNextSe());
          to_run->m_se = GetNextSe();
          // what if we wait to set this to NULL until after thread schedule
          SetNextSe(NULL);
          //printf("\n\n BIG TIME \n\n");
          ScheduleThread(to_run, false);
        }
        else {
          DPRINTF("Deleting task: %p\n", to_run);
          delete(to_run);
          m_num_tasks--;
          DPRINTF("[SUBTRACT] m_num_tasks is now: %d\n", m_num_tasks);
        }
      }

      node->m_val->vruntime += j;

    } // end num_tasks in the rq (finish a single entity)

    // check if we want to insert the sched_entity back on the tree
    // If there are no tasks left in the rq because they all exited, then no need
    if (node->m_val->my_rq.size() != 0) {
      DPRINTF("adding entity back to our tree\n");
      m_rbtree->deleteNode(node);
#ifdef FART
      m_rbtree->add(node->m_val->vruntime, node->m_val, node->m_val->id);
#else
      m_rbtree->add(node->m_val->vruntime, node->m_val);
#endif
      continue;
    }
    else {
      DPRINTF("Deleting SchedEntity: %p\n", node->m_val);
      DPRINTF("It has remaining tasks: %d\n", node->m_val->my_rq.size());
      delete(node->m_val);
    }
    // free the node.... is that okay?
#ifdef POOL
    m_rbtree->deleteNode(node);
#else
    delete(node);
#endif

  } // end iterating through the entities in the rb tree
  DPRINTF("Returning because empty tree\n");
  DPRINTF("High mark nodes was %d\n", m_rbtree->high_mark);
  DPRINTF("High mark tasks was %ld\n", high_mark_task);
#ifdef POOL
  DPRINTF("High mark block was %ld\n", m_rbtree->GetBlockHighMark());
#endif
  return;
}

#endif
