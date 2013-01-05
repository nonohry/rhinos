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
#include "proc.h"
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
      //klib_printf("Add - Sending %d + %d\n",cm.op_1,cm.op_2);
      if (ipc_sendrec(3,&m)!=IPC_SUCCESS)
	{
	   klib_printf("Add: error on IPC\n");
	  break;
	}
      klib_mem_copy((addr_t)m.data,(addr_t)&cm,m.len);
      //klib_printf("Add - Receiving : %d\n",cm.op_res);
      k++;
    }
  klib_printf(" [Quit Add....]\n");
  return;
}

void Calc(u8_t who)
{
  struct ipc_message m;
  struct calc_msg cm;

  while(ipc_receive(who,&m)==IPC_SUCCESS)
    {
      klib_mem_copy((addr_t)m.data,(addr_t)&cm,m.len);
      klib_printf("Receive op %u %u from %s\n",cm.op_1, cm.op_2,(thread_id2thread(m.from))->name);
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

  klib_printf(" [Quit Calc....]\n");
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
      //klib_printf("Mult - Sending %d * %d\n",cm.op_1,cm.op_2);
      if (ipc_sendrec(3,&m)!=IPC_SUCCESS)
	{
	  klib_printf("Mult: error on IPC\n");
	  break;
	}
      klib_mem_copy((addr_t)m.data,(addr_t)&cm,m.len);
      //klib_printf("Mult - Receiving : %d\n",cm.op_res);
      j--;
    }
  klib_printf(" [Quit Mult....]\n");
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
  klib_printf("Physical Memory Manager initialized\n");

  /* Initialisation de la pagination */
  if ( paging_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Paging enabled\n");

  /* Initialisation de la memoire virtuelle */
  if ( virt_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Virtual Memory Manager initialized\n");

  /* Initialisation de l ordonannceur */
  if ( sched_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Scheduler initialized\n");

  /* Initialisation des thread */
  if ( thread_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Threads initialized\n");


  /* Initialisation des processes */
  if ( proc_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Processes initialized\n");


  /* Test Proc */

  struct proc* kern_proc;
  struct proc* user_proc;

  kern_proc = (struct proc*)virt_alloc(sizeof(struct proc));
  kern_proc->name[0] = 'k';
  kern_proc->name[1] = 'e';
  kern_proc->name[2] = 'r';
  kern_proc->name[3] = 'n';
  kern_proc->name[4] = '_';
  kern_proc->name[5] = 'p';
  kern_proc->name[6] = 'r';
  kern_proc->name[7] = 'o';
  kern_proc->name[8] = 'c';
  kern_proc->name[9] = 0;
  kern_proc->v_pd = (struct pde*)PAGING_GET_PD();
  kern_proc->p_pd = paging_virt2phys((virtaddr_t)kern_proc->v_pd);
  LLIST_NULLIFY(kern_proc->thread_list);
 
  user_proc = proc_create("user_proc");
 
  /* Idle Thread */
  thread_idle = thread_create_kern("[Idle]",THREAD_ID_DEFAULT,(virtaddr_t)klib_idle,NULL,THREAD_NICE_TOP,THREAD_QUANTUM_DEFAULT);
  if ( thread_idle == NULL )
    {
      goto err00;
    }
  klib_printf("Idle Thread initialized\n");

  /* Tests threads */

  struct thread* th_add;
  struct thread* th_calc;
  struct thread* th_mult;
  struct thread* th_user;
  virtaddr_t v_entry, v_stack;
  physaddr_t paddr;
   
  th_add = thread_create_kern("Add_thread",THREAD_ID_DEFAULT,(virtaddr_t)Add,(void*)12,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);
  th_calc = thread_create_kern("Calc_thread",THREAD_ID_DEFAULT,(virtaddr_t)Calc,(void*)IPC_ANY,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);
  th_mult = thread_create_kern("Mult_thread",THREAD_ID_DEFAULT,(virtaddr_t)Mult,(void*)15,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);


  /* Destinations arbitraire pour le code et la pile utilisateur */
  v_entry = 0x80000000; /* == (1<31) */
  v_stack = 0x80001000;

  /* Se met sur le page directory du processus utilisateur (qui est synchro avec le noyau donc pas de soucis) pour le mapping */
  klib_load_CR3(user_proc->p_pd);

  /* Effectue les mappages dans ce nouvel espace d'adressage */

  /* paddr = (physaddr_t)phys_alloc(CONST_PAGE_SIZE); */
  /* if (!paddr) */
  /*   { */
  /*     goto err00; */
  /*   } */

  
  if (paging_map(v_entry,0x1835000,PAGING_USER) == EXIT_FAILURE) 
    {
      goto err00;
    }
  
  paddr = (physaddr_t)phys_alloc(CONST_PAGE_SIZE); 
  if (!paddr)
    { 
      goto err00; 
    } 
  
  if (paging_map(v_stack,paddr,PAGING_USER) == EXIT_FAILURE) 
    { 
      goto err00; 
    } 
  

  /*  Cree le thread dans l'espace utilisateur (sinon, pas acces au point d entree ni a la pile) */
  th_user = thread_create_user("User_thread",THREAD_ID_DEFAULT,v_entry,v_stack,CONST_PAGE_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT); 
  if (th_user == NULL) 
    { 
      goto err00; 
    } 
  
  /* Retourne dans l'espace d'adressage noyau */
  klib_load_CR3((physaddr_t)kern_PD); 

  /* Rentre tous les threads noyau dans le processus noyau */
  proc_add_thread(kern_proc,kern_th);
  proc_add_thread(kern_proc,thread_idle);
  proc_add_thread(kern_proc,th_add);
  proc_add_thread(kern_proc,th_mult);
  proc_add_thread(kern_proc,th_calc);
  
  
  /* Ajoute le thread user a proc user */
  proc_add_thread(user_proc,th_user);
  
  /* Affecte le processus courant et cree un ordonnancement initial */
  cur_proc = kern_proc;
  
  /* Initialisation du gestionnaire des IRQ */
  if ( irq_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("IRQ System initialized\n");
  
  /* Initialisation Horloge */
  if ( pit_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Clock (100Hz) initialized\n");

  /* Restaure les interruptions */
  klib_sti();

  /* On ne doit plus arriver ici (sauf DEBUG ou erreur) */
 err00:
  
  while(1)
    {
    }

  return EXIT_SUCCESS;
}

