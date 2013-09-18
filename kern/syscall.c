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
PRIVATE struct thread* syscall_find_sender(struct proc* ptarget, struct proc* pfrom);
PRIVATE u8_t syscall_copymsg( struct thread* src, struct thread* dest);


/**

   Function: void syscall_handle(void)
   -----------------------------------

   Syscall main handler
   Redispatch calls to the correct IPC primitive according to syscall number 
   retrieved in the caller CPU context.

   Result is returned to caller through its EAX register

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
  arch_ctx_set((arch_ctx_t*)cur_th, X86_CONST_RETURN,res);

  return;
}



/**

   Function: u8_t syscall_send(struct thread* th_sender, struct proc* proc_receiver)
   ---------------------------------------------------------------------------------


   Pass ̀message` from `th_sender` to `proc_receiver`.
   First it checks for deadlock situation (`proc_receiver` sending to `th_sender` proc)

   Then, if `th_receiver` is waiting for the message, ̀message` is copied from `th_sender` to `th_receiver`,
   `th_receiver` is set as ready for scheduling and `th_sender` is set as blocked (waiting for a notify call).

   If `th_receiver` is not waiting for any message, `th_sender` is set as blocked in `th_receiver` waiting list.

   At last, scheduler is call because sender will be blocked in any cases.


**/

PRIVATE u8_t syscall_send(struct thread* th_sender, struct proc* proc_receiver)
{
  struct thread* th_receiver;
  struct thread* th_tmp;
  
  /* There must be a receiver */
  if ( proc_receiver == NULL )
    {
      return IPC_FAILURE;
    }

  /* Set receiver in sender structure */
  th_sender->ipc.send_to = proc_receiver;

  /* Set state */
  th_sender->ipc.state |= SYSCALL_IPC_SENDING;

  /* Check for a deadlock: run through sending chain from receiver */
  if (th_receiver->ipc.state & SYSCALL_IPC_SENDING)
    {

      th_tmp = th_receiver->ipc.send_to;
      while( th_tmp->ipc.state & SYSCALL_IPC_SENDING )
	{
	  /* One thread in the sending chain is the sender: deadlock */
	  if (th_tmp == th_sender)
	    {
	      return IPC_DEADLOCK;
	    }
	  th_tmp = th_tmp->ipc.send_to;

	}
    }

  /* No deadlock here, check if receiver is only receiving, and is receiving from the sender or any thread */
  if ( (th_receiver->ipc.state == SYSCALL_IPC_RECEIVING)
       && ( (th_receiver->ipc.receive_from == th_sender)||(th_receiver->ipc.receive_from == NULL) ) )
    {
      /* Receiver is willing to receive: copy message from sender to receiver */
      th_receiver->cpu.ebx = th_sender->cpu.ebx;
      th_receiver->cpu.ecx = th_sender->cpu.ecx;
      th_receiver->cpu.edx = th_sender->cpu.edx;   
      th_receiver->cpu.esi = th_sender->cpu.esi;
  
      /* Set receiver ready for scheduling */ 
      if (th_receiver->state == THREAD_BLOCKED)
	{

	  th_receiver->next_state = THREAD_READY;
	  /* Set end of reception */
	  th_receiver->ipc.state &= ~SYSCALL_IPC_RECEIVING;

	  /* Scheduler queues manipulations */
	  if ( sched_dequeue(SCHED_BLOCKED_QUEUE, th_receiver) != EXIT_SUCCESS )
	    {
	      return IPC_FAILURE;
	    }

	  if ( sched_enqueue(SCHED_READY_QUEUE, th_receiver) != EXIT_SUCCESS )
	    {
	      return IPC_FAILURE;
	    }

	}

      /* Message is delivered to receiver, set end of sending */
      th_sender->ipc.state &= ~SYSCALL_IPC_SENDING;
      /* Sender is blocked, waiting for message processing (must be unblocked via notify) */
      th_sender->next_state = THREAD_BLOCKED;

    }
  else
    {
      /* Receiver is not waiting for sender at the moment: block sender in receiver's waiting list */
      th_sender->next_state = THREAD_BLOCKED_SENDING;
    }

  /* In any cases, current thread (sender) is blocked, so scheduling is needing */
  sched_schedule(SCHED_FROM_SEND);

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

  /* Set thread to receive from */
  th_receiver->ipc.receive_from = th_sender;

  /* Set receive state */
  th_receiver->ipc.state |= SYSCALL_IPC_RECEIVING;

  /* Look for a matching thread in receive list */
  if (!LLIST_ISNULL(th_receiver->ipc.receive_waitlist))
    {
      /* Receive from any thread, get the first available */
      if (th_sender == NULL)
	{
	  th_available = LLIST_GETHEAD(th_receiver->ipc.receive_waitlist);
	}
      else
	{
	  /* Look for `th_sender` */
	  struct thread* th_tmp = LLIST_GETHEAD(th_receiver->ipc.receive_waitlist);
	  do
	    {
	      /* Found ! */
	      if (th_tmp == th_sender)
		{
		  th_available = th_tmp;
		  break;
		}
	      th_tmp = LLIST_NEXT(th_receiver->ipc.receive_waitlist, th_tmp);
	    }while(!LLIST_ISHEAD(th_receiver->ipc.receive_waitlist, th_tmp));
	}
    }


  /* A matching sender found ? */
  if ( th_available != NULL )
    {
      /* Copy message from sender to receiver */ 
      th_receiver->cpu.ebx = th_available->cpu.ebx;
      th_receiver->cpu.ecx = th_available->cpu.ecx;
      th_receiver->cpu.edx = th_available->cpu.edx;
      th_receiver->cpu.esi = th_available->cpu.esi;

      /* Unblock sender if needed */
      if (th_available->state == THREAD_BLOCKED_SENDING)
	{
           
	  th_available->next_state = THREAD_READY;
	  /* Set end of sending */
	  th_available->ipc.state &= ~SYSCALL_IPC_SENDING;

	  /* Remove sender from receiver waiting list et set it as ready for scheduling */
	  LLIST_REMOVE(th_receiver->ipc.receive_waitlist, th_available);
	  if ( sched_enqueue(SCHED_READY_QUEUE, th_available) != EXIT_SUCCESS )
	    {
	      return IPC_FAILURE;
	    }

	}

      /* End of reception */
      th_receiver->ipc.state &= ~SYSCALL_IPC_RECEIVING;
      
    }
  else
    {
      /* No matching sender found: blocked waiting for a sender */
      th_receiver->next_state = THREAD_BLOCKED;
      /* Current thread (receiver) is blocked, need scheduling */
      sched_schedule(SCHED_FROM_RECEIVE);
    }

  return IPC_SUCCESS;
}


/**

   u8_t syscall_notify(struct thread* th_from, struct thread* th_to)
   -----------------------------------------------------------------

   Send a notification from `th_from` to `th_to`
   This syscall is used by receiver to notify blocked sender that message processing is finished.
   Simply change `th_to` state et set is ready for scheduling

**/

PRIVATE u8_t syscall_notify(struct thread* th_from, struct proc* proc_to)
{
  if ( (th_to->state == THREAD_BLOCKED) && (th_to->ipc.state != SYSCALL_IPC_RECEIVING) )
	{
	  /* Set recipient ready for scheduling */
	  th_to->next_state = THREAD_READY;
	  /* End of sending */
	  th_to->ipc.state &= ~SYSCALL_IPC_SENDING;

	  /* Scheduler queues manipulation */
	  sched_dequeue(SCHED_BLOCKED_QUEUE,th_to);
	  sched_enqueue(SCHED_READY_QUEUE,th_to);
	}

  return IPC_SUCCESS;
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

   Function: struct thread* syscall_find_sender(struct proc* ptarget, struct proc* pfrom)
   --------------------------------------------------------------------------------------

   Return a thread belonging to `pfrom` in `ptarget` wait list.
   Return NULL if such a thread does not exist;

**/
   

PRIVATE struct thread* syscall_find_sender(struct proc* ptarget, struct proc* pfrom)
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
