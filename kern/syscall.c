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


PRIVATE void syscall_send(struct thread* th_sender, struct thread* th_receiver, struct ipc_message* message);
PRIVATE void syscall_receive(struct thread* th_receiver, struct thread* th_sender, struct ipc_message* message);
PRIVATE void syscall_notify(struct thread* th_from, struct thread* th_to);


/*========================================================================
 * Point d entree
 *========================================================================*/


PUBLIC void syscall_handle()
{
  struct thread* cur_th;
  struct thread* target_th;
  u32_t syscall_num;
  s32_t arg_id;
  struct ipc_message* arg_msg;

  /* Le thread courant */
  cur_th = sched_get_running_thread();
  ASSERT_RETURN_VOID( cur_th != NULL );

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
      ASSERT_RETURN_VOID( target_th != NULL );
    }

  /* Redirection vers les fonction effectives */
  switch(syscall_num)
    {
    case SYSCALL_SEND:
      {
	syscall_send(cur_th, target_th, arg_msg);
	break;
      }

    case SYSCALL_RECEIVE:
      {
	syscall_receive(cur_th, target_th, arg_msg);
	break;
      }

    case SYSCALL_NOTIFY:
      {
	syscall_notify(cur_th, target_th);
	break;
      }
      
    default:
      break;  
    }

  return;
}


/*========================================================================
 * Send
 *========================================================================*/

PRIVATE void syscall_send(struct thread* th_sender, struct thread* th_receiver, struct ipc_message* message)
{

  /* Pas de broadcast */
  ASSERT_RETURN_VOID( th_receiver!=NULL );

  /* DEBUG */
  klib_bochs_print(th_sender->name);
  klib_bochs_print(" sending to ");
  klib_bochs_print(th_receiver->name);
  klib_bochs_print(" ");

  /* Indique a qui envoyer */
  th_sender->ipc.send_to = th_receiver;

  /* Bloque l envoyeur et ordonnance */
  th_sender->next_state = THREAD_BLOCKED_SENDING;
  sched_schedule(SCHED_FROM_SEND);

  return;
}


/*========================================================================
 * Receive
 *========================================================================*/

PRIVATE void syscall_receive(struct thread* th_receiver, struct thread* th_sender, struct ipc_message* message)
{
  klib_bochs_print(th_receiver->name);
  klib_bochs_print(" receiving from ");
  (th_sender==NULL)?klib_bochs_print("ANY"):klib_bochs_print(th_sender->name);
  klib_bochs_print(" ");

  return;
}

/*========================================================================
 * Notify
 *========================================================================*/

PRIVATE void syscall_notify(struct thread* th_from, struct thread* th_to)
{
  klib_bochs_print(th_from->name);
  klib_bochs_print(" notifying to ");
  (th_to==NULL)?klib_bochs_print("ANY"):klib_bochs_print(th_to->name);
  klib_bochs_print(" ");

  return;
}
