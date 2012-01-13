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
  int num;
  int arg_1, arg_2;

  /* Le thread courant */
  cur_th = sched_get_running_thread();
  ASSERT_RETURN_VOID( cur_th != NULL );

  /* Le numero d appel dans les registres */
  num = (int)(cur_th->ctx->edx);

  /* Les arguments dans les registres */
  arg_1 = (int)(cur_th->ctx->ebx);
  arg_2 = (int)(cur_th->ctx->ecx);

  /* Affiche pour le moment */
  //klib_bochs_print("[(%d)%d-%d]",num, arg_1, arg_2);
  klib_bochs_print("1");

  return;
}
