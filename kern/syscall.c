/**
   
   syscall.c
   =========

   Kernel syscalls.
   Provide classical microkernel IPC API (send & receive) 

**/


/**

   Includes 
   ========


   - types.h
   - llist.h
   - ipc.h           : IPC constnats needed
   - const.h
   - klib.h
   - thread.h        : struct thread needed
   - sched.h         : scheduler queue manipulation
   - physmem.h       : phys_alloc needed
   - virtmem_buddy.h : virtual buddy allocation/release 
   - paging.h        : mapping physical/virtual
   - syscall.h       : self header


**/

#include <types.h>
#include <llist.h>
#include <ipc.h>
#include "const.h"
#include "klib.h"
#include "thread.h"
#include "sched.h"
#include "physmem.h"
#include "virtmem.h"
#include "paging.h"
#include "syscall.h"


/**

   Privates
   --------

   Effective IPC primitives

**/


PRIVATE u8_t syscall_send(struct thread* th_sender, struct thread* th_receiver, struct ipc_message* message);
PRIVATE u8_t syscall_receive(struct thread* th_receiver, struct thread* th_sender, struct ipc_message* message);
PRIVATE u8_t syscall_notify(struct thread* th_from, struct thread* th_to);
PRIVATE u8_t syscall_copymsg(struct ipc_message* message, physaddr_t phys_msg, u8_t op);


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
  struct thread* target_th;
  struct ipc_message* arg_message;
  u32_t syscall_num;
  s32_t arg_id;
  u8_t res;

  /* Get current thread */
  cur_th = sched_get_running_thread();
  if (cur_th == NULL)
    {
      res = IPC_FAILURE;
      goto end;
    }

  /* Get syscall number in EDX */
  syscall_num = (u32_t)(cur_th->cpu.edx);

  /* Get destination thread (to send to or to receive from) in EBX */
  arg_id = (s32_t)(cur_th->cpu.ebx);

  /* Send & receive syscall need a message. Get it in ECX */
  if (syscall_num != SYSCALL_NOTIFY)
    {
      arg_message = (struct ipc_message*)(cur_th->cpu.ecx);
      /* Set message originator */
      arg_message->from = cur_th->id->id;
    }

  /* Destination thread */
  if (arg_id == IPC_ANY)
    {
      target_th = NULL;
    }
  else
    {
      /* Get thread structure from given id */
      target_th = thread_id2thread(arg_id);
      if ( target_th == NULL )
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
	res = syscall_send(cur_th, target_th, arg_message);
	break;
      }

    case SYSCALL_RECEIVE:
      {
	res = syscall_receive(cur_th, target_th, arg_message);
	break;
      }

    case SYSCALL_NOTIFY:
      {
	res = syscall_notify(cur_th, target_th);
	break;
      }
    default:
      {
	res = IPC_FAILURE;
	break;
      }  
    }

 end:
  /* Set result in caller's EAX register */
  cur_th->cpu.eax=res;

  return;
}



/**

   Function: u8_t syscall_send(struct thread* th_sender, struct thread* th_receiver, struct ipc_message* message)
   --------------------------------------------------------------------------------------------------------------


   Pass ̀message` from `th_sender` to `th_receiver`.
   First it checks for a deadlock situation (`th_receiver` sending to `th_sender`)

   Then, if `th_receiver` is waiting for the message, ̀message` is copied from `th_sender` to `th_receiver`,
   `th_receiver` is set as ready for scheduling and `th_sender` is set as blocked (waiting for a notify call).

   If `th_receiver` is not waiting for any message, `th_sender` is set as blocked in `th_receiver` waiting list.

   At last, scheduler is call because sender will be blocked in any cases.


**/

PRIVATE u8_t syscall_send(struct thread* th_sender, struct thread* th_receiver, struct ipc_message* message)
{
  struct thread* th_tmp;

  /* There must be a receiver */
  if ( th_receiver == NULL )
    {
      return IPC_FAILURE;
    }

  /* Set receiver in sender structure */
  th_sender->ipc.send_to = th_receiver;

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

  /* No deadlock here, get message from sender address space */ 
  th_sender->ipc.send_message = message;
  th_sender->ipc.send_phys_message = paging_virt2phys((virtaddr_t)message);

  /* No physical mapping for message: error */
  if ( !th_sender->ipc.send_phys_message )
    {
      return IPC_FAILURE;
    }
 
  /* Check if receiver is only receiving, and is receiving from the sender or any thread */
  if ( (th_receiver->ipc.state == SYSCALL_IPC_RECEIVING)
       && ( (th_receiver->ipc.receive_from == th_sender)||(th_receiver->ipc.receive_from == NULL) ) )
    {
      /* Receiver is willing to receive: copy message from sender to receiver */
      if (syscall_copymsg(message,th_receiver->ipc.receive_phys_message,SYSCALL_SEND) != EXIT_SUCCESS)
	{
	  return IPC_FAILURE;
	}
      
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

   Function: u8_t syscall_receive(struct thread* th_receiver, struct thread* th_sender, struct ipc_message* message)
   -----------------------------------------------------------------------------------------------------------------


   Set up the receiving state for `th_receiver`.
   If `th_sender` is in its waiting list, retrieve sender's message and unblock sender.
   Otherwise, `th_receiver` will blocked, waiting for `th_sender`.


**/

PRIVATE u8_t syscall_receive(struct thread* th_receiver, struct thread* th_sender, struct ipc_message* message)
{
  struct thread* th_available = NULL;

  /* Set thread to receive from */
  th_receiver->ipc.receive_from = th_sender;

  /* Set receive state */
  th_receiver->ipc.state |= SYSCALL_IPC_RECEIVING;

  /* Set message for reception */
  th_receiver->ipc.receive_message = message;
  th_receiver->ipc.receive_phys_message = paging_virt2phys((virtaddr_t)message);  

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
      if (syscall_copymsg(message,th_available->ipc.send_phys_message,SYSCALL_RECEIVE) != EXIT_SUCCESS)
	{
	  return IPC_FAILURE;
	}

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

PRIVATE u8_t syscall_notify(struct thread* th_from, struct thread* th_to)
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

   Function: u8_t syscall_copymsg(struct ipc_message* message, physaddr_t phys_msg, u8_t op)
   -----------------------------------------------------------------------------------------

   Helper to copy messages between 2 adress spaces. Source and destination for copy depend 
   on operation `op`.
   phys_msg will be mapped in current adress space. If operation is sending, `message` is copied
   to the brand new mapped `phys_msg`. Otherwise, `phys_msg` is copied to `message`.


**/

PRIVATE u8_t syscall_copymsg(struct ipc_message* message, physaddr_t phys_msg, u8_t op)
{
  physaddr_t phys_page;
  virtaddr_t virt_page;
  virtaddr_t virt_message;
  
  /* Allocate an unmapped virtual page */
  virt_page = (virtaddr_t)virtmem_buddy_alloc(CONST_PAGE_SIZE,VIRT_BUDDY_NOMAP);
  if ((void*)virt_page == NULL)
    {
      return EXIT_FAILURE;
    }
  
  /* Get physical page containing `phys_msg` */
  phys_page = PHYS_ALIGN_INF(phys_msg);
  
  /* Map that physical page with the previous allocated virtual page */
  if (paging_map(virt_page, phys_page, PAGING_SUPER) == EXIT_FAILURE)
    {
      virtmem_buddy_free((void*)virt_page);
      return EXIT_FAILURE;
    }
      
  /* Clean cache for the virtual page */
  klib_invlpg(virt_page);

  /* Compute `phys_msg` virtual address for the copy */
  virt_message = virt_page + (phys_msg - phys_page);
      
  /* Message copy, depending on operation */
  switch(op)
    {
    case SYSCALL_RECEIVE:
      klib_mem_copy(virt_message, (addr_t)message, sizeof(struct ipc_message));
      break;

    case SYSCALL_SEND:
      klib_mem_copy((addr_t)message, virt_message, sizeof(struct ipc_message));
      break;

    default:
      return EXIT_FAILURE;
      break;
    }
      
  /* Unmap allocated page */
  paging_unmap(virt_page);
      
  /* Clean cache */
  klib_invlpg(virt_page);

  /* Free allocated page */
  virtmem_buddy_free((void*)virt_page);

  return EXIT_SUCCESS;
}
