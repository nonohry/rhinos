/**
   
   syscall.c
   =========

   Kernel syscalls.
   Provide classical microkernel IPC API (send & receive) 

**/


/**

   Includes 
   --------


   - define.h
   - types.h
   - llist.h
   - ipc.h           : IPC constants needed
   - arch_const.h    : architecture dependant constants
   - arch_ctx.h      : cpu context
   - proc.h          : proc needed
   - thread.h        : struct thread needed
   - sched.h         : scheduler queue manipulation
   - syscall.h       : self header


**/

#include <define.h>
#include <types.h>
#include <llist.h>
#include <ipc.h>
#include <arch_const.h>
#include <arch_ctx.h>
#include "proc.h"
#include "thread.h"
#include "sched.h"
#include "syscall.h"



/**
  
   Constants: Syscall numbers
   --------------------------
   
**/


#define SYSCALL_SEND        1
#define SYSCALL_RECEIVE     2
#define SYSCALL_NOTIFY      3


/**
  
   Constants: Syscall IPC states
   -----------------------------
   
**/


#define SYSCALL_IPC_SENDING    1
#define SYSCALL_IPC_RECEIVING  2
#define SYSCALL_IPC_NOTIFYING  3



/**

   Privates
   --------

   Real IPC primitives

**/


PRIVATE u8_t syscall_send(struct thread* th_sender, struct proc* proc_receiver);
PRIVATE u8_t syscall_receive(struct thread* th_receiver, struct proc* proc_sender);
PRIVATE u8_t syscall_notify(struct thread* th_from, struct proc* proc_to);


/**

   Privates
   --------

   Helpers

**/


PRIVATE u8_t syscall_deadlock(struct proc* psender, struct proc* ptarget);
PRIVATE struct thread* syscall_find_receiver(struct proc* ptarget, struct proc* pfrom);
PRIVATE struct thread* syscall_find_waiting_sender(struct proc* ptarget, struct proc* pfrom);
PRIVATE struct thread* syscall_find_blocked_sender(struct proc* ptarget, struct proc* pfrom);
PRIVATE u8_t syscall_copymsg( struct thread* src, struct thread* dest);


/**

   Function: void syscall_handle(void)
   -----------------------------------

   Syscall main handler
   Redispatch calls to the correct IPC primitive according to syscall number 
   retrieved in the caller CPU context.

   Result is returned to caller through its return register

**/


PUBLIC void syscall_handle(void)
{
  struct thread* cur_th;
  struct proc* target_proc;
  pid_t pid;
  u32_t syscall_num;
  u8_t res;

  /* Get current thread */
  if (cur_th == NULL)
    {
      res = IPC_FAILURE;
      goto end;
    }

  /* Get syscall number from source register */
  syscall_num = arch_ctx_get((arch_ctx_t*)cur_th, ARCH_CONST_SOURCE);

  /* Put originator proc into source register instead */
  arch_ctx_set((arch_ctx_t*)cur_th, ARCH_CONST_SOURCE,cur_th->proc->pid);

  /* Destination proc, stored in EDI */
  pid = (pid_t)arch_ctx_get((arch_ctx_t*)cur_th, ARCH_CONST_DEST);
  if ( pid == IPC_ANY)
    {
      target_proc = NULL;
    }
  else
    {
      /* Get proc structure from given id */
      target_proc = proc_pid(pid);
      if ( target_proc == NULL )
	{
	  res = IPC_FAILURE;
	  goto end;
	}
    }
  
  /* Dispatch call to effective primitives */
  switch(syscall_num)
    {
    case SYSCALL_SEND:
      {
	res = syscall_send(cur_th, target_proc);
	break;
      }

    case SYSCALL_RECEIVE:
      {
	res = syscall_receive(cur_th, target_proc);
	break;
      }

    case SYSCALL_NOTIFY:
      {
	res = syscall_notify(cur_th, target_proc);
	break;
      }
    default:
      {
	res = IPC_FAILURE;
	break;
      }  
    }

 end:
  /* Set result in caller's return register */
  arch_ctx_set((arch_ctx_t*)cur_th, ARCH_CONST_RETURN,res);

  return;
}



/**

   Function: u8_t syscall_send(struct thread* th_sender, struct proc* proc_receiver)
   ---------------------------------------------------------------------------------


   Pass message from `th_sender` to `proc_receiver`.
   First it checks for deadlock situation (`proc_receiver` sending to `th_sender` proc)

   Then, if a thread in `proc_receiver` is waiting for the message, it is copied from `th_sender` to that thread,
   The receiving thread is set ready for scheduling and `th_sender` is set blocked (waiting for a notify call).

   If `proc_receiver` is not waiting for any message, `th_sender` is set as blocked in `proc_receiver` waiting list.

   At last, scheduler is call because sender will be blocked in any cases.


**/

PRIVATE u8_t syscall_send(struct thread* th_sender, struct proc* proc_receiver)
{
  struct thread* th_receiver;
  struct thread* th_tmp;
  
  /* There must be a receiver - No broadcast allow */
  if ( proc_receiver == NULL )
    {
      return IPC_FAILURE;
    }

  /* Check for deadlock */
  if (syscall_deadlock(th_sender->proc,proc_receiver) == IPC_FAILURE)
    {
      return IPC_FAILURE;
    }

  /* No deadlock here, set state */
  th_sender->ipc.state |= SYSCALL_IPC_SENDING;

  /* Set destination */
  th_sender->ipc.send_to = proc_receiver;

  /* Get a thread willing to receive the message */
  th_receiver = syscall_find_receiver(proc_receiver,th_sender->proc);

  if (th_receiver != NULL)
    {
      /* Found a thread ! copy message from sender to receiver */
      syscall_copymsg(th_sender,th_receiver);
  
      /* Set end of reception */
      th_receiver->ipc.state &= ~SYSCALL_IPC_RECEIVING;

      /* Ready for scheduling */
      th_receiver->state = THREAD_READY;

      /* Scheduler queues manipulations */
      sched_dequeue(SCHED_BLOCKED_QUEUE, th_receiver);
      sched_enqueue(SCHED_READY_QUEUE, th_receiver);

      /* Message is delivered to receiver, set end of sending */
      th_sender->ipc.state &= ~SYSCALL_IPC_SENDING;    

       /* Scheduler queues manipulations (blocks sender) */
      sched_dequeue(SCHED_READY_QUEUE, th_sender);
      sched_enqueue(SCHED_BLOCKED_QUEUE, th_sender);
    }
  else
    {
      /* No receiving thread, enqueue in wait list */
      sched_dequeue(SCHED_READY_QUEUE, th_sender);
      LLIST_ADD(proc_receiver->wait_list,th_sender);
    }
  
  /* Sender is blocked, waiting for message processing (must be unblocked via notify) */
  th_sender->state = THREAD_BLOCKED;

  /* In any cases, current thread (sender) is blocked, so scheduling is needing */
  sched_elect();

  return IPC_SUCCESS;
}



/**

   Function: u8_t syscall_receive(struct thread* th_receiver, struct thread* th_sender)
   ------------------------------------------------------------------------------------


   Set up the receiving state for `th_receiver`.
   If `th_sender` is in its waiting list, retrieve sender's message and unblock sender.
   Otherwise, `th_receiver` will blocked, waiting for `th_sender`.


**/

PRIVATE u8_t syscall_receive(struct thread* th_receiver, struct proc* proc_sender)
{
  struct thread* th_available = NULL;

  /* Set receive state */
  th_receiver->ipc.state |= SYSCALL_IPC_RECEIVING;

  /* Set thread to receive from (can be NULL) */
  th_receiver->ipc.receive_from = proc_sender;

  /* Find a thread sending to me */
  th_available = syscall_find_waiting_sender(th_receiver->proc, proc_sender)
 
  /* A matching sender found ? */
  if ( th_available != NULL )
    {
      /* Copy message from sender to receiver */ 
      syscall_copymsg(th_available,th_receiver);
 
      /* Unblock sender */
      th_available->state = THREAD_READY;

      /* Set end of sending */
      th_available->ipc.state &= ~SYSCALL_IPC_SENDING;

      /* Remove sender from receiver waiting list et set it as ready for scheduling */
      LLIST_REMOVE(th_receiver->ipc.receive_waitlist, th_available);
      sched_enqueue(SCHED_READY_QUEUE, th_available);

      /* End of reception */
      th_receiver->ipc.state &= ~SYSCALL_IPC_RECEIVING;
      
    }
  else
    {
      /* No matching sender found: blocked waiting for a sender */
      th_receiver->state = THREAD_BLOCKED;

      /* Sched queues manipulation */
      sched_dequeue(SCHED_READY_QUEUE, th_receiver);
      sched_enqueue(SCHED_BLOCKED_QUEUE, th_receiver);

      /* Current thread (receiver) is blocked, need scheduling */
      sched_elect();
    }

  return IPC_SUCCESS;
}


/**

   u8_t syscall_notify(struct thread* th_from, struct thread* th_to)
   -----------------------------------------------------------------

   Send a notification from `th_from` to `th_to`
   This syscall is used by receiver to notify blocked sender that message processing is finished.
   Simply change `th_to` state to ready for scheduling

**/

PRIVATE u8_t syscall_notify(struct thread* th_from, struct proc* proc_to)
{
  struct thread* th;

  th = syscall_find_blocked_sender(th_from->proc,proc_to);
  if (th != NULL)
    {
      /* Set recipient ready for scheduling */
      th->state = THREAD_READY;
      /* End of sending */
      th->ipc.state &= ~SYSCALL_IPC_SENDING;
      
      /* Scheduler queues manipulation */
      sched_dequeue(SCHED_BLOCKED_QUEUE,th);
      sched_enqueue(SCHED_READY_QUEUE,th);

      return IPC_SUCCESS;
    }

  return IPC_FAILURE;
}



/**

   Function: u8_t syscall_deadlock(struct proc* psender, struct proc* ptarget)
   ---------------------------------------------------------------------------

   Check for deadlock during send

   Simply follow the "send chain" to check if a loop occurs.
   This function is a DFS-like recursive function.

**/

PRIVATE u8_t syscall_deadlock(struct proc* psender, struct proc* ptarget)
{

  struct thread_wrapper* wrapper;

  /* Check all threads in `ptarget`*/
  if (!LLIST_ISNULL(ptarget->thread_list))
    {
      wrapper=LLIST_GETHEAD(ptarget->thread_list);
      do
	{
	  /* A thread is sending */
	  if (wrapper->thread->ipc.state == SYSCALL_IPC_SENDING)
	    {
	      /* Does it send to sender ? */
	      if (wrapper->thread->ipc.send_to == psender)
		{
		  /* If yes, return Failure */
		  return IPC_FAILURE;
		}
	      else
		{
		  /* Otherwise, check that new "send chain" */
		  if (syscall_deadlock(psender,wrapper->thread->ipc.send_to) == IPC_FAILURE)
		    {
		      return IPC_FAILURE;
		    }
		}
	    }
	  
	  wrapper = LLIST_NEXT(ptarget->thread_list,wrapper);
	  
	}while(!LLIST_ISHEAD(ptarget->thread_list,wrapper));
    }
  
  return IPC_SUCCESS;
}


/**

   Function: struct thread* syscall_find_receiver(struct proc* ptarget, struct proc* pfrom)
   --------------------------------------------------------------------------------------

   Return a thread in `ptarget` which is receiving from `pfrom` or from ANY if `pfrom` is NULL
   Return NULL if such a thread does not exist;

**/
   

PRIVATE struct thread* syscall_find_receiver(struct proc* ptarget, struct proc* pfrom)
{

  struct thread_wrapper* wrapper;

   /* Check all threads in `ptarget`*/
  if (!LLIST_ISNULL(ptarget->thread_list))
    {
      wrapper=LLIST_GETHEAD(ptarget->thread_list);
      do
	{
	  /* A thread is receiving from `pfrom` - works even if `pfrom` is NULL */
	  if ( (wrapper->thread->ipc.state == SYSCALL_IPC_RECEIVING)
	       && ((wrapper->thread->ipc.recv_from == pfrom) || (wrapper->thread->ipc.recv_from == NULL)) )
	    {
	      /* Return thread */
	      return wrapper->thread;
	    }
	  
	  wrapper = LLIST_NEXT(ptarget->thread_list,wrapper);
	  
	}while(!LLIST_ISHEAD(ptarget->thread_list,wrapper));
    }

  return NULL;
}


/**

   Function: struct thread* syscall_find_waiting_sender(struct proc* ptarget, struct proc* pfrom)
   ----------------------------------------------------------------------------------------------

   Return a thread belonging to `pfrom` in `ptarget` wait list.
   Return NULL if such a thread does not exist;

**/
   

PRIVATE struct thread* syscall_find_waitig_sender(struct proc* ptarget, struct proc* pfrom)
{

  struct thread* th;

   /* Check all threads in `ptarget` wait list*/
  if (!LLIST_ISNULL(ptarget->wait_list))
    {
      th=LLIST_GETHEAD(ptarget->wait_list);

      if (pfrom == NULL)
	{
	  /* Receive from ANY, return wait list head */
	  return th;
	}
      else
	{
	  /* look from a thread belonging to `pfrom` */
	  do
	    {
	      if (th->proc == pfrom)
		{
		  return th;
		}
	      
	      th = LLIST_NEXT(ptarget->wait_list,th);
	  
	    }while(!LLIST_ISHEAD(ptarget->wait_list,th));
	}
    }

  return NULL;
}


/**

   Function: struct thread* syscall_find_blocked_sender(struct proc* ptarget, struct proc* pfrom)
   ----------------------------------------------------------------------------------------------

   Return a thread belonging to `pfrom` blocked sending to `ptarget`.
   Return NULL if such a thread does not exist;

**/
   

PRIVATE struct thread* syscall_find_blocked_sender(struct proc* ptarget, struct proc* pfrom)
{

  struct thread* th;

   /* Check all threads in `ptarget` wait list*/
  if (!LLIST_ISNULL(pfrom->thread_list))
    {
      th=LLIST_GETHEAD(pfrom->thread_list);

      /* look from a thread sending to `ptarget` */
      do
	{
	  if ( (th->ipc.send_to == ptarget)
	       && (th->state = THREAD_BLOCKED)
	       && (th->ipc.state != SYSCALL_IPC_RECEIVING) )
	    {
	      return th;
	    }
	  
	  th = LLIST_NEXT(pfrom->thread_list,th);
	  
	}while(!LLIST_ISHEAD(pfrom->thread_list,th));
    }

  return NULL;
}



/**

   Function: u8_t syscall_copymsg( struct thread* src, struct thread* dest)
   ------------------------------------------------------------------------
   
   Copy message stored in registers from `src` to `dest`.
   
**/


PRIVATE u8_t syscall_copymsg( struct thread* src, struct thread* dest)
{

  arch_ctx_set((arch_ctx_t*)dest,ARCH_CONST_SOURCE,arch_ctx_get((arch_ctx_t*)src,ARCH_CONST_SOURCE));
  arch_ctx_set((arch_ctx_t*)dest,ARCH_CONST_MSG1,arch_ctx_get((arch_ctx_t*)src,ARCH_CONST_MSG1));
  arch_ctx_set((arch_ctx_t*)dest,ARCH_CONST_MSG2,arch_ctx_get((arch_ctx_t*)src,ARCH_CONST_MSG2));
  arch_ctx_set((arch_ctx_t*)dest,ARCH_CONST_MSG3,arch_ctx_get((arch_ctx_t*)src,ARCH_CONST_MSG3));

  return EXIT_SUCCESS;
}
