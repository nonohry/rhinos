/*
 * RhinOS Main
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include <llist.h>
#include "const.h"
#include "start.h"
#include "klib.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem.h"
#include "context_cpu.h"
#include "thread.h"
#include "sched.h"
#include "irq.h"
#include "pit.h"
#include <ipc.h>

void toto(char c)
{
  char t[2];
  struct ipc_message m;
  u32_t k=1;
  u32_t i;
 
  t[0]=c;
  t[1]=0;

  m.len = 2;

  while(k)
    {
      i=0;
      m.code = k;
      //ipc_send(3,&m);
      klib_bochs_print(t);
      while(i < (1<<9))
	{
	  i++;
	}
      k++;
      //klib_bochs_print(   "Incrementing m.code\n");
    }

  return;
}

void titi(char c)
{
  char t[2];
  struct ipc_message m;

  t[0]=c;
  t[1]=0;

  while(1)
    {
      u32_t i=0;
      klib_bochs_print(t);
      //ipc_receive(IPC_ANY,&m);
      //klib_bochs_print("DATA RECEIVED from %d =>   Message len: %d code :%d\n",m.from,m.len,m.code);

      ipc_notify(m.from);

      while(i < (1<<9))
	{
	  i++;
	}
    }

  return;
}


void tata(char c)
{
  char t[2];
  int j=1048;
  struct ipc_message m;
  u32_t i;

  t[0]=c;
  t[1]=0;

  m.len = 2;
  m.code = 67;

  while(j)
    {
       i=0;
       //ipc_send(3, &m);
      klib_bochs_print(t);
      while(i < (1<<9))
	{
	  i++;
	}
      //j--;
    }
  klib_bochs_print(" [Quit....] ");
  return;
}


/*========================================================================
 * Fonction principale 
 *========================================================================*/


PUBLIC int main()
{
  struct thread* thread_idle;

  /* Initialisation de la memoire physique */
  if ( phys_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_bochs_print("Physical Memory Manager initialized\n");

  /* Initialisation de la pagination */
  if ( paging_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_bochs_print("Paging enabled\n");

  /* Initialisation de la memoire virtuelle */
  if ( virt_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_bochs_print("Virtual Memory Manager initialized\n");

  /* Initialisation des contextes */
  if ( context_cpu_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_bochs_print("Kernel Contexts initialized\n");

  /* Initialisation des thread */
  if ( thread_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_bochs_print("Kernel Threads initialized\n");


  /* Initialisation de l ordonannceur */
  if ( sched_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_bochs_print("Scheduler initialized\n");


  /* Idle Thread */
  thread_idle = thread_create("[Idle]",THREAD_ID_DEFAULT,(virtaddr_t)klib_idle,NULL,THREAD_STACK_SIZE,THREAD_NICE_TOP,THREAD_QUANTUM_DEFAULT);
  if ( thread_idle == NULL )
    {
      goto err00;
    }
  klib_bochs_print("Idle Thread initialized\n");

  /* Tests threads */

  struct thread* to;
  struct thread* ti;
  struct thread* ta;


  to = thread_create("Toto_thread",THREAD_ID_DEFAULT,(virtaddr_t)toto,(void*)'1',THREAD_STACK_SIZE,THREAD_NICE_DEFAULT-5,THREAD_QUANTUM_DEFAULT);
  ti = thread_create("Titi_thread",THREAD_ID_DEFAULT,(virtaddr_t)titi,(void*)'2',THREAD_STACK_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);
  ta = thread_create("Tata_thread",THREAD_ID_DEFAULT,(virtaddr_t)tata,(void*)'3',THREAD_STACK_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);

  /* Simule un ordonnancement */
  sched_dequeue(SCHED_READY_QUEUE,ta);
  sched_enqueue(SCHED_RUNNING_QUEUE,ta);
  

  /* Initialisation du gestionnaire des IRQ */
  if ( irq_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_bochs_print("IRQ System initialized\n");
  
  /* Initialisation Horloge */
  if ( pit_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_bochs_print("Clock (100Hz) initialized\n");

   /* On ne doit plus arriver ici (sauf DEBUG ou erreur) */
 err00:

  while(1)
    {
    }

  return EXIT_SUCCESS;
}

