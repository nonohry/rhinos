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


  struct thread* t;

  t = thread_create("Test_thread",(virtaddr_t)toto,(void*)'#',(virtaddr_t)toto,(void*)'#',THREAD_STACK_SIZE);

  klib_bochs_print(t->name);
  klib_bochs_print("\nStack: 0x%x\n",t->stack_base);

  thread_destroy(t);


  /* Initialisation du gestionnaire des IRQ */
  irq_init();

  /* Initialisation Horloge */
  pit_init();
  klib_bochs_print("Clock initialized (100Hz)\n");

 
  /* On ne doit plus arriver ici (sauf DEBUG) */
  while(1)
    {
    }

  return EXIT_SUCCESS;
}
