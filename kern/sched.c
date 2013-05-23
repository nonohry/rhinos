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


#include <types.h>
#include <llist.h>
#include "klib.h"
#include "const.h"
#include "thread.h"
#include "sched.h"


/**
   
   Privates
   --------

   Scheduler queues

**/

PRIVATE struct thread* sched_ready[SCHED_QUEUE_SIZE];
PRIVATE struct thread* sched_running;
PRIVATE struct thread* sched_blocked;
PRIVATE struct thread* sched_dead;


/**
   
   Privates
   --------

   Helper functions

**/

PRIVATE void sched_enqueue_roundrobin(struct thread* th);
PRIVATE void sched_enqueue_staircase(struct thread* th);
PRIVATE u8_t sched_get_highest_prio_queue(void);


/**

   Function: u8_t sched_init(void)
   -------------------------------

   Scheduler initialization.
   Just nullify all the queues.
 
**/


PUBLIC u8_t sched_init(void)
{
  u8_t i;

  /* Initialisation des listes */
  for(i=0;i<SCHED_QUEUE_SIZE;i++)
    {
      LLIST_NULLIFY(sched_ready[i]);
    }
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
      /* 2 kinds of Ready Queues, choose one depending on prioriy */
      if (th->sched.static_prio < SCHED_PRIO_RR)
	{
	  sched_enqueue_staircase(th);
	}
      else
	{
	  sched_enqueue_roundrobin(th);
	}
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
      LLIST_REMOVE(sched_ready[th->sched.dynamic_prio], th);
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
   Choose the thread to be executed. This processthread will be the one with
   the highest priority unless this thread has a nil quantum. 

   Current thread will be placed in the correct scheduling queue depending
   on its future state (IPC blocked, Ready, ...)

   `flag` indicates the origin of sched_schedule call. If its the PIT 
   (flag==SCHED_FROM_PIT), then the current thread quantum is decremented.

   The function does not return anything, instead, it defines the elected
   thread as the current thread by calling `thread_switch_to` and returns to caller.

**/


PUBLIC void sched_schedule(u8_t flag)
{
  struct thread* cur_th;
  struct thread* new_th;
  u8_t high_prio;

  /* No thread is running, must be an error */
  if ( LLIST_ISNULL(sched_running) )
    {
      return;
    }

  /* Get current thread */
  cur_th = sched_get_running_thread();
  if (cur_th == NULL)
    {
      return;
    }

  /* If PIT is the caller, decrement thread quantum */
  if (flag == SCHED_FROM_PIT)
    {
      cur_th->sched.dynamic_quantum--;
    }

  /* find the highest priority queue */
  high_prio = sched_get_highest_prio_queue();
     
  /* Scheduling occurs if:
     
     - current thread has consumed his quantum
     - or there is a ready thread with higher priority
     - or the future current thread state is not Ready
     
     Note: Even if the current thread has the highest priority, it will be scheduled if its quantum is nil. */
     
  if ( (cur_th->sched.dynamic_quantum<0) || (high_prio > cur_th->sched.dynamic_prio) || (cur_th->next_state != THREAD_READY) )
    {
      
      /* Re-place the current thread in the right queue */
      sched_dequeue(SCHED_RUNNING_QUEUE,cur_th);
      switch(cur_th->next_state)
	{
	case THREAD_READY:
	  {
 	    sched_enqueue(SCHED_READY_QUEUE,cur_th);
	    break;
	  }

	case THREAD_BLOCKED:
	  {
 	    sched_enqueue(SCHED_BLOCKED_QUEUE,cur_th);
	    break;
	  }

	case THREAD_BLOCKED_SENDING:
	  {
	    /* In this case, current thread is adding to the receiver waiting threads list */
	    LLIST_ADD((cur_th->ipc.send_to)->ipc.receive_waitlist, cur_th);
	    cur_th->state = THREAD_BLOCKED_SENDING;
	    break;
	  }

	default:
 	  sched_enqueue(SCHED_DEAD_QUEUE,cur_th);
	  break;
	}

      
      /* Get the new thread with highest priority */
      new_th = LLIST_GETHEAD(sched_ready[high_prio]);
      if (new_th == NULL)
	{
	  return;
	}

      /* Set the new thread as runnable */
      sched_dequeue(SCHED_READY_QUEUE,new_th);
      sched_enqueue(SCHED_RUNNING_QUEUE,new_th);


      /* Switch to the new thread (set it as current) */
      thread_switch_to(new_th);

    }

  return;
}


/**

   Function: struct thread* sched_get_running_thread(void)
   -------------------------------------------------------

   Get the current thread.
   For the moment, juste return the head of the Running Queue.

**/

PUBLIC struct thread* sched_get_running_thread(void)
{
  return LLIST_GETHEAD(sched_running);
}


/**

   Function: void sched_enqueue_roundrobin(struct thread* th)
   ----------------------------------------------------------

   Helper to enqueue thread `th` in the ready queue. 
   This function is called if `th` has a high priority. In this case,
   thread are juste enqueue at their priority according to a 
   round robin algorithm.

**/


PRIVATE void sched_enqueue_roundrobin(struct thread* th)
{
  if (th->sched.dynamic_quantum < 0)
    {
      /* Re-set quantum */
      th->sched.dynamic_quantum = th->sched.static_quantum;
    }

  /* Enqueue (roundrobin) */
  LLIST_ADD(sched_ready[th->sched.dynamic_prio], th);

  return;
}



/**

   Function: void sched_enqueue_staircase(struct thread* th)
   ---------------------------------------------------------

   Helper to enqueue thread `th` in the ready queue. 
   This function is called if `th` has a low priority. In this case, threads are
   enqueued according to staircase algorithm.

   Staircase algorithm defines dynamic priorities and quanta. At first, the thread
   is executed at its original priority during its quantum. The second execution occurs 
   at (original_priority - 1) during one quantum, then (original_priority - 2) for the third
   execution and so on. 

   When that dynamic priority reaches 0, the thread priority is set to
   (original_priority - 1) with quantum equal to 2*original_quantum. The execution follow the
   same prvious rules and when that second series of execution reaches the priority 0, the thread 
   priority is set to (original_priority - 2) with quantum equal to 3*original_quantum and so on.

   When that dynamic start priority reaches 0, the thread recovers its original priority and
   all the algorithm is replayed. Graphically, we have:

                                         Executions
                                             ^
                                             |
			                  7  |
			               6  1  |
			            5  1  1  |
		                 4  1  1  1  |
		              3  1  1  1  1  |
		           2  1  1  1  1  1  |
	     Quantum :  1  1  1  1  1  1  1  |
 
	     Priority:  6  5  4  3  2  1  0 


   Looks like a staircase no ? (or an Xmas tree)
   
**/

PRIVATE void sched_enqueue_staircase(struct thread* th)
{
  if (th->sched.dynamic_quantum < 0)
    {
      /* Decrement dynamic priority */
      if (th->sched.dynamic_prio > 0)
	{
	  th->sched.dynamic_prio--;
	  /* set a new quantum */
	  th->sched.dynamic_quantum=th->sched.static_quantum;
	}
      else
	{
	  /* Here, dynamic priority reaches 0, so decrements dynamic starting priority */
	  th->sched.head_prio--;

	  /* Starting priority reaches 0 */
	  if (th->sched.head_prio < 0)
	    {
	      /* Back to original state */
	      th->sched.head_prio = th->sched.static_prio;
	    }

	  /* Dynamic priority is now based on strating priority */
	  th->sched.dynamic_prio = th->sched.head_prio;
	  /* Dynamic quantum is set according to dynamic priority */
	  th->sched.dynamic_quantum = (th->sched.static_prio - th->sched.head_prio)*th->sched.static_quantum;

	}

    }

  /* Enqueue in the righ queue */
  LLIST_ADD(sched_ready[th->sched.dynamic_prio], th);  

  return;
}


/**

   Function: u8_t sched_get_highest_prio_queue(void)
   ------------------------------------------------

   Find the non empty queue in Ready Queues with the highest priority

**/


PRIVATE u8_t sched_get_highest_prio_queue(void)
{
  u8_t i;
  
  /* Run through the linked list until we find a non empty one */
  for(i=SCHED_PRIO_MAX;(i!=0)&&(LLIST_ISNULL(sched_ready[i]));i--)
    {}

  return i;
}
