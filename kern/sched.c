/**
 
   sched.c
   =======

   Threads scheduler

**/


/**

  Includes
  --------

  - types.h
  - llist.h
  - klib.h
  - const.h
  - thread.h : struct thread needed
  - sched.h  : self header

**/


#include <define.h>
#include <types.h>
#include <llist.h>
#include "thread.h"
#include "sched.h"


/**
   
   Privates
   --------

   Scheduler queues

**/

PRIVATE struct thread* sched_ready;
PRIVATE struct thread* sched_running;
PRIVATE struct thread* sched_blocked;
PRIVATE struct thread* sched_dead;


/**

   Function: u8_t sched_setup(void)
   -------------------------------

   Scheduler initialization.
   Just nullify all the queues.
 
**/


PUBLIC u8_t sched_setup(void)
{
  LLIST_NULLIFY(sched_ready);
  LLIST_NULLIFY(sched_running);
  LLIST_NULLIFY(sched_blocked);
  LLIST_NULLIFY(sched_dead);

  return EXIT_SUCCESS;
}


/**

   Function: u8_t sched_enqueue(u8_t queue, struct thread* th)
   -----------------------------------------------------------

   Add a thread to a scheduler queue. There are 4 queues:

   - SCHED_RUNNING_QUEUE : Thread is currently being executed by processor
   - SCHED_READY_QUEUE   : Thread is ready to be executed. The queue is in fact a queues array
   - SCHED_BLOCKED_QUEUE : Thread is blocked in an IPC operation
   - SCHED_DEAD_QUEUE    : Thread is dead, waiting to be deleted

   Queues manipulation is done thanks to linked list primitives.

**/

PUBLIC u8_t sched_enqueue(u8_t queue, struct thread* th)
{
 
  if ( th == NULL)
    {
      return EXIT_FAILURE;
    }

  /* Queue to insert into */
  switch(queue)
    {
    
    case SCHED_RUNNING_QUEUE:
      
      LLIST_ADD(sched_running, th);
      th->state = THREAD_RUNNING;
      break;

    case SCHED_READY_QUEUE:
      LLIST_ADD(sched_ready, th);
      th->state = THREAD_READY;
      break;

    case SCHED_BLOCKED_QUEUE:
      LLIST_ADD(sched_blocked, th);
      th->state = THREAD_BLOCKED;
      break;
      
    case SCHED_DEAD_QUEUE:
      LLIST_ADD(sched_dead, th);
      th->state = THREAD_DEAD;
      break;

    default:
       return EXIT_FAILURE;
      
    }

    return EXIT_SUCCESS;

}


/**

   Function: u8_t sched_dequeue(u8_t queue, struct thread* th)
   -----------------------------------------------------------

   Remove a thread from a queue.
   
**/


PUBLIC u8_t sched_dequeue(u8_t queue, struct thread* th)
{
  if ( th == NULL)
    {
      return EXIT_FAILURE;
    }

  switch(queue)
    {
    
    case SCHED_RUNNING_QUEUE:
       LLIST_REMOVE(sched_running, th);
      break;

    case SCHED_READY_QUEUE:
      LLIST_REMOVE(sched_ready, th);
      break;
      
    case SCHED_BLOCKED_QUEUE:
       LLIST_REMOVE(sched_blocked, th);
      break;

    case SCHED_DEAD_QUEUE:
       LLIST_REMOVE(sched_dead, th);
      break;

    default:
       return EXIT_FAILURE;
      
    }

  return EXIT_SUCCESS;

}


/**

   Function: void sched_schedule(u8_t flag)
   ----------------------------------------

   Main scheduler fonction. 

**/


PUBLIC void sched_schedule()
{
  struct thread* th;

  th = LLIST_GETHEAD(sched_ready);
  sched_dequeue(SCHED_READY_QUEUE,th);
  sched_enqueue(SCHED_READY_QUEUE,th);
  cur_th = th;

  return;
}

