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
#include "thread.h"
#include "sched.h"
#include "irq.h"
#include "pit.h"
#include <ipc.h>

void Add(u16_t max)
{
  struct ipc_message m;
  u32_t k=0;

  m.len = 2;
  m.code = 1;

  while(k<max)
    {
      m.arg1 = k;
      m.arg2 = k;
      klib_bochs_print("Add - Sending %d + %d\n",m.arg1,m.arg2);
      ipc_send(3,&m);
      ipc_receive(3,&m);
      klib_bochs_print("Add - Receiving : %d\n",m.res);
      k++;
    }
  klib_bochs_print(" [Quit Add....]\n");
  return;
}

void Calc(u8_t who)
{
  struct ipc_message m;

  while(1)
    {
      ipc_receive(who,&m);

      switch(m.code)
	{     
	case 1:
	  m.res = m.arg1 + m.arg2;
	  break;
	case 2:
	  m.res = m.arg1 * m.arg2;
	  break;
	default:
	  m.res = 0;
	}
      
      ipc_notify(m.from);
      ipc_send(m.from,&m);

    }

  return;
}


void Mult(u8_t max)
{
  int j;
  struct ipc_message m;

  m.len = 2;
  m.code = 2;
  j=max;

  while(j)
    {
      m.arg1 = j;
      m.arg2 = j;
      klib_bochs_print("Mult - Sending %d * %d\n",m.arg1,m.arg2);
      ipc_send(3,&m);
      ipc_receive(3,&m);
      klib_bochs_print("Mult - Receiving : %d\n",m.res);
      j--;
    }
  klib_bochs_print(" [Quit Mult....]\n");
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


  to = thread_create("Add_thread",THREAD_ID_DEFAULT,(virtaddr_t)Add,(void*)12,THREAD_STACK_SIZE,THREAD_NICE_DEFAULT-5,THREAD_QUANTUM_DEFAULT);
  ti = thread_create("Calc_thread",THREAD_ID_DEFAULT,(virtaddr_t)Calc,(void*)IPC_ANY,THREAD_STACK_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);
  ta = thread_create("Mult_thread",THREAD_ID_DEFAULT,(virtaddr_t)Mult,(void*)15,THREAD_STACK_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);

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

