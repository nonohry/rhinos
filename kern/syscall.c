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
 * Point d entree
 *========================================================================*/


void syscall_handle()
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
      /* TODO Recherche le thread via son threadID */
      target_th = thread_id2thread(arg_id);
      ASSERT_RETURN_VOID( target_th != NULL );
    }

  /* Redirection vers les fonction effectives */
  switch(syscall_num)
    {
    case SYSCALL_SEND:
      {
	klib_bochs_print(cur_th->name);
	klib_bochs_print(" sending to ");
	(target_th==NULL)?klib_bochs_print("ANY"):klib_bochs_print(target_th->name);
	klib_bochs_print(" ");
	break;
      }

    case SYSCALL_RECEIVE:
      {
	klib_bochs_print(cur_th->name);
	klib_bochs_print(" receiving from ");
	(target_th==NULL)?klib_bochs_print("ANY"):klib_bochs_print(target_th->name);
	klib_bochs_print(" ");
	break;
      }

    case SYSCALL_NOTIFY:
      {
	klib_bochs_print(cur_th->name);
	klib_bochs_print(" notifying to ");
	(target_th==NULL)?klib_bochs_print("ANY"):klib_bochs_print(target_th->name);
	klib_bochs_print(" ");
	break;
      }
      
    default:
      break;  
    }

  return;
}
