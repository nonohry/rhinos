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


struct calc_msg
{
  u8_t op_code;
  u16_t op_1;
  u16_t op_2;
  u32_t op_res;
} __attribute__((packed));


void Add(u16_t max)
{
  struct ipc_message m;
  struct calc_msg cm;
  u32_t k=0;

  m.len = sizeof(struct calc_msg);
  cm.op_code = 1;

  while(k<max)
    {
      cm.op_1 = k;
      cm.op_2 = k;
      klib_mem_copy((addr_t)&cm,(addr_t)m.data,m.len);
      klib_bochs_print("Add - Sending %d + %d\n",cm.op_1,cm.op_2);
      if (ipc_sendrec(3,&m)!=IPC_SUCCESS)
	{
	  break;
	}
      klib_mem_copy((addr_t)m.data,(addr_t)&cm,m.len);
      klib_bochs_print("Add - Receiving : %d\n",cm.op_res);
      k++;
    }
  klib_bochs_print(" [Quit Add....]\n");
  return;
}

void Calc(u8_t who)
{
  struct ipc_message m;
  struct calc_msg cm;

  while(ipc_receive(who,&m)==IPC_SUCCESS)
    {
      klib_mem_copy((addr_t)m.data,(addr_t)&cm,m.len);
      switch(cm.op_code)
	{     
	case 1:
	  cm.op_res = cm.op_1 + cm.op_2;
	  break;
	case 2:
	  cm.op_res = cm.op_1 * cm.op_2;
	  break;
	default:
	  cm.op_res = 0;
	}
      klib_mem_copy((addr_t)&cm,(addr_t)m.data,m.len);      
      ipc_notify(m.from);
      ipc_send(m.from,&m);

    }

  klib_bochs_print(" [Quit Calc....]\n");
  return;
}


void Mult(u8_t max)
{
  int j;
  struct ipc_message m;
  struct calc_msg cm;

  m.len = sizeof(struct calc_msg);
  cm.op_code = 2;
  j=max;

  while(j)
    {
      cm.op_1 = j;
      cm.op_2 = j;
      klib_mem_copy((addr_t)&cm,(addr_t)m.data,m.len);
      klib_bochs_print("Mult - Sending %d * %d\n",cm.op_1,cm.op_2);
      if (ipc_sendrec(3,&m)!=IPC_SUCCESS)
	{
	  break;
	}
      klib_mem_copy((addr_t)m.data,(addr_t)&cm,m.len);
      klib_bochs_print("Mult - Receiving : %d\n",cm.op_res);
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

  klib_printf("coucou%d\n",4);
  klib_printf("coucou%d\n",-7);
  klib_printf("coucou%d\n",2);
  while(1){}

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

  struct thread* th_add;
  struct thread* th_calc;
  struct thread* th_mult;


  th_add = thread_create("Add_thread",THREAD_ID_DEFAULT,(virtaddr_t)Add,(void*)12,THREAD_STACK_SIZE,THREAD_NICE_DEFAULT-5,THREAD_QUANTUM_DEFAULT);
  th_calc = thread_create("Calc_thread",THREAD_ID_DEFAULT,(virtaddr_t)Calc,(void*)IPC_ANY,THREAD_STACK_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);
  th_mult = thread_create("Mult_thread",THREAD_ID_DEFAULT,(virtaddr_t)Mult,(void*)15,THREAD_STACK_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);

  /* Simule un ordonnancement */
  sched_dequeue(SCHED_READY_QUEUE,th_calc);
  sched_enqueue(SCHED_RUNNING_QUEUE,th_calc);
  

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

