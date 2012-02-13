/*
 * Syscall
 * Traitement des appels systemes
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include <llist.h>
#include <ipc.h>
#include "const.h"
#include "klib.h"
#include "assert.h"
#include "thread.h"
#include "sched.h"
#include "syscall.h"


/*========================================================================
 * Declaration PRIVATE
 *========================================================================*/


PRIVATE u8_t syscall_send(struct thread* th_sender, struct thread* th_receiver, struct ipc_message* message);
PRIVATE u8_t syscall_receive(struct thread* th_receiver, struct thread* th_sender, struct ipc_message* message);
PRIVATE u8_t syscall_notify(struct thread* th_from, struct thread* th_to);


/*========================================================================
 * Point d entree
 *========================================================================*/


PUBLIC u8_t syscall_handle()
{
  struct thread* cur_th;
  struct thread* target_th;
  u32_t syscall_num;
  s32_t arg_id;
  struct ipc_message* arg_msg;
  u8_t res;

  /* Le thread courant */
  cur_th = sched_get_running_thread();
  ASSERT_RETURN( cur_th != NULL , IPC_FAILURE);

  /* Le numero d appel dans les registres */
  syscall_num = (u32_t)(cur_th->ctx->edx);

  /* Les arguments dans les registres */
  arg_id = (s32_t)(cur_th->ctx->ebx);
  if (syscall_num != SYSCALL_NOTIFY)
    {
      arg_msg = (struct ipc_message*)(cur_th->ctx->ecx);
    }

  /* Le thread cible */
  if (arg_id == IPC_ANY)
    {
      target_th = NULL;
    }
  else
    {
      /* Recherche le thread via son threadID */
      target_th = thread_id2thread(arg_id);
      ASSERT_RETURN( target_th != NULL , IPC_FAILURE);
    }

  /* Redirection vers les fonction effectives */
  switch(syscall_num)
    {
    case SYSCALL_SEND:
      {
	res = syscall_send(cur_th, target_th, arg_msg);
	break;
      }

    case SYSCALL_RECEIVE:
      {
	res = syscall_receive(cur_th, target_th, arg_msg);
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

  return res;
}


/*========================================================================
 * Send
 *========================================================================*/

PRIVATE u8_t syscall_send(struct thread* th_sender, struct thread* th_receiver, struct ipc_message* message)
{
  struct thread* th_tmp;

  /* Pas de broadcast */
  ASSERT_RETURN( th_receiver!=NULL , IPC_FAILURE);

  /* DEBUG */
  klib_bochs_print(th_sender->name);
  klib_bochs_print(" sending to ");
  klib_bochs_print(th_receiver->name);
  klib_bochs_print(" ");

  /* Indique a qui envoyer */
  th_sender->ipc.send_to = th_receiver;

  /* Precise l envoi */
  th_sender->ipc.state &= SYSCALL_IPC_SENDING;

  /* Si la reception envoie, alors on s'assure qu on ne boucle pas */
  if (th_receiver->ipc.state & SYSCALL_IPC_SENDING)
    {
      th_tmp = th_receiver->ipc.send_to;
      while( th_tmp->ipc.state & SYSCALL_IPC_SENDING )
	{
	  /* Deadlock */
	  if (th_tmp == th_sender)
	    {
	      return IPC_DEADLOCK;
	    }
	  th_tmp = th_tmp->ipc.send_to;

	}
    }

  /* Pas de deadlock, le message est recupere (dans l espace d adressage de l emetteur)  */
  th_sender->ipc.send_message = message;

  /* Regarde si la reception receptionne uniquement et de la part de l emetteur */
  if ( ( (th_receiver->ipc.state & (SYSCALL_IPC_SENDING|SYSCALL_IPC_RECEIVING)) == SYSCALL_IPC_RECEIVING)
       && ( (th_receiver->ipc.receive_from == th_sender)||(th_receiver->ipc.receive_from == NULL) ) )
    {

      /* Differencie les cas d'espace d'adressage noyau ou non */
      if ( (th_sender->ctx->cs == th_receiver->ctx->cs)
	   && (th_sender->ctx->cs == CONST_CS_SELECTOR) )
	{
	  /* Espace noyau commun, simple reference memoire */
	  th_receiver->ipc.receive_message = message;
	}
      else
	{
	  /* TODO : Espace different non noyau */
	}

      /* Debloque la reception au besoin */
      if (th_receiver->state == THREAD_BLOCKED)
	{
	  th_receiver->next_state = THREAD_READY;
	  ASSERT_RETURN( sched_dequeue(SCHED_BLOCKED_QUEUE, th_receiver)==EXIT_SUCCESS, IPC_FAILURE);
	  ASSERT_RETURN( sched_enqueue(SCHED_READY_QUEUE, th_receiver)==EXIT_SUCCESS, IPC_FAILURE);
	}

      /* Bloque l emetteur naturellement */
      th_sender->next_state = THREAD_BLOCKED;
    }
  else
    {
      /* Bloque l emetteur dans la wait list de la reception */
      th_sender->next_state = THREAD_BLOCKED_SENDING;
    }

  /* Ordonnance */
  sched_schedule(SCHED_FROM_SEND);

  return IPC_SUCCESS;
}


/*========================================================================
 * Receive
 *========================================================================*/

PRIVATE u8_t syscall_receive(struct thread* th_receiver, struct thread* th_sender, struct ipc_message* message)
{
  klib_bochs_print(th_receiver->name);
  klib_bochs_print(" receiving from ");
  (th_sender==NULL)?klib_bochs_print("ANY"):klib_bochs_print(th_sender->name);
  klib_bochs_print(" ");

  if (!LLIST_ISNULL(th_receiver->ipc.receive_waitlist))
    {
      u8_t n=0;
      struct thread* th_tmp = LLIST_GETHEAD(th_receiver->ipc.receive_waitlist);
      do
	{
	  n++;
	  th_tmp = LLIST_NEXT(th_receiver->ipc.receive_waitlist, th_tmp);
	}while(!LLIST_ISHEAD(th_receiver->ipc.receive_waitlist, th_tmp));
      klib_bochs_print(" (Got %d messages !) ",n);      
    }

  return IPC_SUCCESS;
}

/*========================================================================
 * Notify
 *========================================================================*/

PRIVATE u8_t syscall_notify(struct thread* th_from, struct thread* th_to)
{
  klib_bochs_print(th_from->name);
  klib_bochs_print(" notifying to ");
  (th_to==NULL)?klib_bochs_print("ANY"):klib_bochs_print(th_to->name);
  klib_bochs_print(" ");

  return IPC_SUCCESS;
}
