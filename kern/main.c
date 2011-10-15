/*
 * RhinOS Main
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "const.h"
#include <llist.h>
#include "start.h"
#include "klib.h"
#include "assert.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem.h"
#include "context_cpu.h"
#include "thread.h"
#include "sched.h"
#include "irq.h"
#include "pit.h"




void toto(char c)
{
  char t[2];

  t[0]=c;
  t[1]=0;

  while(1)
    {
      klib_bochs_print(t);
      thread_switch(cur_thread,THREAD_READY);
    }

  return;
}

void titi(char c)
{
  char t[2];

  t[0]=c;
  t[1]=0;

  while(1)
    {
      klib_bochs_print(t);
      thread_switch(cur_thread,THREAD_READY);
    }

  return;
}

/*========================================================================
 * Fonction principale 
 *========================================================================*/


PUBLIC int main()
{

  /* Initialisation de la memoire physique */
  phys_init();
  klib_bochs_print("Physical Memory Manager initialized\n");

  /* Initialisation de la pagination */
  paging_init();
  klib_bochs_print("Paging enabled\n");

  /* Initialisation de la memoire virtuelle */
  virt_init();
  klib_bochs_print("Virtual Memory Manager initialized\n");

  /* Initialisation des contextes */
  context_cpu_init();
  klib_bochs_print("Kernel Contexts initialized\n");

  /* Initialisation des thread */
  thread_init();
  klib_bochs_print("Kernel Threads initialized\n");

  /* Initialisation de l ordonannceur */
  sched_init();
  klib_bochs_print("Scheduler initialized\n");

  struct thread* to;
  struct thread* ti;

  to = thread_create("Toto_thread",(virtaddr_t)toto,(void*)'#',(virtaddr_t)toto,(void*)'#',THREAD_STACK_SIZE);
  ti = thread_create("Titi_thread",(virtaddr_t)titi,(void*)'-',(virtaddr_t)titi,(void*)'-',THREAD_STACK_SIZE);

  // thread_destroy(to);
  // thread_destroy(ti);

  /* Initialisation du gestionnaire des IRQ */
  irq_init();

  /* Initialisation Horloge */
  pit_init();
  klib_bochs_print("Clock initialized (100Hz)\n");

  cur_thread = to;
  LLIST_REMOVE(sched_ready,cur_thread);
  LLIST_ADD(sched_running,cur_thread);
  context_cpu_switch_to(cur_thread->ctx);
 
  /* On ne doit plus arriver ici (sauf DEBUG) */
  while(1)
    {
    }

  return EXIT_SUCCESS;
}
