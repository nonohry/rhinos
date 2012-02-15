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
#include "physmem.h"
#include "virtmem.h"
#include "paging.h"
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
  u8_t res;

  /* Le thread courant */
  cur_th = sched_get_running_thread();
  ASSERT_RETURN( cur_th != NULL , IPC_FAILURE);

  /* Le numero d appel dans les registres */
  syscall_num = (u32_t)(cur_th->ctx->edx);

  /* Les arguments dans les registres */
  arg_id = (s32_t)(cur_th->ctx->ebx);

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
	res = syscall_send(cur_th, target_th, (struct ipc_message*)(cur_th->ctx->ecx));
	break;
      }

    case SYSCALL_RECEIVE:
      {
	res = syscall_receive(cur_th, target_th, (struct ipc_message*)(cur_th->ctx->ecx));
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

  /* Indique a qui envoyer */
  th_sender->ipc.send_to = th_receiver;

  /* Precise l envoi */
  th_sender->ipc.state |= SYSCALL_IPC_SENDING;

  /* Si le destinataire envoie, alors on s'assure qu on ne boucle pas */
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
  th_sender->ipc.send_phys_message = paging_virt2phys((virtaddr_t)message);
  ASSERT_RETURN( th_sender->ipc.send_phys_message , IPC_FAILURE);

  /* Regarde si le destinataire receptionne uniquement et de la part de l emetteur */
  if ( ( (th_receiver->ipc.state & (SYSCALL_IPC_SENDING|SYSCALL_IPC_RECEIVING)) == SYSCALL_IPC_RECEIVING)
       && ( (th_receiver->ipc.receive_from == th_sender)||(th_receiver->ipc.receive_from == NULL) ) )
    {

      /* Differencie les cas d'espace d'adressage noyau ou non */
      if ( (th_sender->ctx->cs == th_receiver->ctx->cs)
	   && (th_sender->ctx->cs == CONST_CS_SELECTOR) )
	{
	  /* Espace noyau commun  */
	  physaddr_t phys_page;
	  virtaddr_t virt_page;
	  virtaddr_t virt_message;

	  /* Alloue une page virtuelle */
	  virt_page = (virtaddr_t)virt_alloc(CONST_PAGE_SIZE);
	  ASSERT_RETURN( (void*)virt_page != NULL , IPC_FAILURE);

	  /* Demap la page */
	  paging_unmap(virt_page);

	  /* Determine la page physique du receive_message */
	  phys_page = PHYS_ALIGN_INF(th_receiver->ipc.receive_phys_message);

	  /* Map la page physique du message avec la page virtuelle */
	  ASSERT_RETURN( paging_map(virt_page, phys_page, TRUE) != EXIT_FAILURE , IPC_FAILURE );

	  /* Determine l adresse virtuelle du message */
	  virt_message = virt_page + (th_receiver->ipc.receive_phys_message - phys_page);

	  /* Copie le message */
	  klib_mem_copy((virtaddr_t)message, virt_message, sizeof(struct ipc_message));

	  /* Liberation */
	  paging_unmap(virt_page);
	  virt_free((void*)virt_page);

	}
      else
	{
	  /* TODO : Espace different non noyau */
	}

      /* Debloque le destinataire au besoin */
      if (th_receiver->state == THREAD_BLOCKED)
	{
	  th_receiver->next_state = THREAD_READY;
	  /* Fin de reception */
	  th_receiver->ipc.state &= ~SYSCALL_IPC_RECEIVING;
	  ASSERT_RETURN( sched_dequeue(SCHED_BLOCKED_QUEUE, th_receiver)==EXIT_SUCCESS, IPC_FAILURE);
	  ASSERT_RETURN( sched_enqueue(SCHED_READY_QUEUE, th_receiver)==EXIT_SUCCESS, IPC_FAILURE);
	}

      /* Fin d envoi */
      th_sender->ipc.state &= ~SYSCALL_IPC_SENDING;
     
    }
  else
    {
      /* Bloque l emetteur dans la wait list du destinataire */
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
  struct thread* th_available = NULL;

  /* Indique de qui recevoir */
  th_receiver->ipc.receive_from = th_sender;

  /* Precise la reception */
  th_receiver->ipc.state |= SYSCALL_IPC_RECEIVING;

  /* Le message de reception */
  th_receiver->ipc.receive_message = message;
  th_receiver->ipc.receive_phys_message = paging_virt2phys((virtaddr_t)message);  

  /* Cherche un thread disponible dans la waitlist */
  if (!LLIST_ISNULL(th_receiver->ipc.receive_waitlist))
    {
      if (th_sender == NULL)
	{
	  th_available = LLIST_GETHEAD(th_receiver->ipc.receive_waitlist);
	}
      else
	{
	  struct thread* th_tmp = LLIST_GETHEAD(th_receiver->ipc.receive_waitlist);
	  do
	    {
	      if (th_tmp == th_sender)
		{
		  th_available = th_tmp;
		}
	      th_tmp = LLIST_NEXT(th_receiver->ipc.receive_waitlist, th_tmp);
	    }while(!LLIST_ISHEAD(th_receiver->ipc.receive_waitlist, th_tmp));
	}
    }


  /* Un thread disponible */
  if ( th_available != NULL )
    {
       /* Differencie les cas d'espace d'adressage noyau ou non */
      if ( (th_receiver->ctx->cs == th_available->ctx->cs)
	   && (th_receiver->ctx->cs == CONST_CS_SELECTOR) )
	{
	  /* Espace noyau commun  */
	  physaddr_t phys_page;
	  virtaddr_t virt_page;
	  virtaddr_t virt_message;

	  /* Alloue une page virtuelle */
	  virt_page = (virtaddr_t)virt_alloc(CONST_PAGE_SIZE);
	  ASSERT_RETURN( (void*)virt_page != NULL , IPC_FAILURE);

	  /* Demap la page */
	  paging_unmap(virt_page);

	  /* Determine la page physique du receive_message */
	  phys_page = PHYS_ALIGN_INF(th_available->ipc.send_phys_message);

	  /* Map la page physique du message avec la page virtuelle */
	  ASSERT_RETURN( paging_map(virt_page, phys_page, TRUE) != EXIT_FAILURE , IPC_FAILURE );

	  /* Determine l adresse virtuelle du message */
	  virt_message = virt_page + (th_available->ipc.send_phys_message - phys_page);

	  /* Copie le message */
	  klib_mem_copy(virt_message, (virtaddr_t)message, sizeof(struct ipc_message));

	  /* Liberation */
	  paging_unmap(virt_page);
	  virt_free((void*)virt_page);

	}
      else
	{
	  /* TODO : Espace different non noyau */
	}


      /* Debloque le sender au besoin */
      if (th_available->state == THREAD_BLOCKED_SENDING)
	{
	  th_available->next_state = THREAD_READY;
	  /* Fin d'envoi */
	  th_available->ipc.state &= ~SYSCALL_IPC_SENDING;
	  LLIST_REMOVE(th_receiver->ipc.receive_waitlist, th_available);
	  ASSERT_RETURN( sched_enqueue(SCHED_READY_QUEUE, th_available)==EXIT_SUCCESS, IPC_FAILURE);
	}

      /* Fin de reception */
      th_receiver->ipc.state &= ~SYSCALL_IPC_RECEIVING;

    }
  else
    {
      /* Pas de message ni de thread disponible */
      th_receiver->next_state = THREAD_BLOCKED;
      /* Ordonnance  */
      sched_schedule(SCHED_FROM_RECEIVE);
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
