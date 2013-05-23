/**
   
   main.c
   ======

   Kernel main function. All initializations occur here
   
**/


/**
   
   Includes
   --------
   
   - types.h
   - llist.h
   - const.h
   - start.h
   - klib.h
   - physmem.h   : Physical memory manger initialization 
   - paging.h    : Paging subsystem initialization
   - virtmem.h   : Virtual memory manager initialization
   - thread.h    : Thread subsytem initialization
   - sched.h     : Scheduler initialization
   - proc.h      : Process management initialization
   - irq.h       : IRQ subsystem initialization
   - pit.h       : Clock initialization
   - ipc.h       : Inter process communication system initialization

**/

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



/**

   Proof of concept stuff
   ----------------------

   Here is the proof of IPC mechanism. 2 threads require a third to compute value (addition and multuiplication)
   They exchange command and data through the structure `struct calc_msg`.

**/

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
  u32_t k=1;

  cm.op_code = 1;

  while(k<max)
    {
      cm.op_1 = k;
      cm.op_2 = k;
      klib_mem_copy((addr_t)&cm,(addr_t)m.data,sizeof(struct calc_msg));
      klib_printf("Add - Sending %d + %d\n",cm.op_1,cm.op_2);
      if (ipc_sendrec(3,&m)!=IPC_SUCCESS)
	{
	   klib_printf("Add: error on IPC\n");
	  break;
	}
      klib_mem_copy((addr_t)m.data,(addr_t)&cm,sizeof(struct calc_msg));
      klib_printf("Add - Receiving : %d\n",cm.op_res);
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
      klib_mem_copy((addr_t)m.data,(addr_t)&cm,sizeof(struct calc_msg)); 
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
      klib_mem_copy((addr_t)&cm,(addr_t)m.data,sizeof(struct calc_msg));
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

  cm.op_code = 2;
  j=max;

 
  while(j)
    {
      cm.op_1 = j;
      cm.op_2 = j;
      klib_mem_copy((addr_t)&cm,(addr_t)m.data,sizeof(struct calc_msg));
      klib_printf("Mult - Sending %d * %d\n",cm.op_1,cm.op_2);
      if (ipc_sendrec(3,&m)!=IPC_SUCCESS)
	{
	  klib_printf("Mult: error on IPC\n");
	  break;
	}
      klib_mem_copy((addr_t)m.data,(addr_t)&cm,sizeof(struct calc_msg));
      klib_printf("Mult - Receiving : %d\n",cm.op_res);
      j--;
    }
  klib_printf(" [Quit Mult....]\n");
  return;
}



/**
 
   Function: int main(void)
   ------------------------

   Kernel main function. Initialize all subsystems then wait for a thread to spawn
   (at least, the idle thread will spawn)

**/


PUBLIC int main()
{
  u32_t i;
  struct thread* thread_idle;
  struct multiboot_mod_entry* mod;
  physaddr_t paddr;

  struct thread* th_add;
  struct thread* th_calc;
  struct thread* th_mult;
  struct thread* th_user;
  virtaddr_t v_entry, v_stack;

  struct proc* kern_proc;
  struct proc* user_proc;


  char* proc_names[CONST_BOOT_MODULES] = {"user_proc1","user_proc2","user_proc3"};
  char* th_names[CONST_BOOT_MODULES] = {"user_thread1","user_thread2","user_thread3"};


  /* Physical memory manager */
  if ( phys_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Physical Memory Manager initialized\n");


  /* Relocate boot modules with new (and controlled) physical pages */
  for (mod = (struct multiboot_mod_entry *) start_mbi->mods_addr, i=0;
       i < start_mbi->mods_count;
       mod++, i++)
    {
      /* Allocate a physical area */
      paddr = (physaddr_t)phys_alloc(mod->end-mod->start);
      if (!paddr)
	{
	  goto err00;
	}
      /* Copy module */
      klib_mem_copy(mod->start,paddr,mod->end-mod->start);
      /* Update end & start */
      mod->end = paddr + (mod->end-mod->start);
      mod->start = paddr;

    }

  /* Pagination */
  if ( paging_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Paging enabled\n");

  /* Virtual memory manager */
  if ( virt_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Virtual Memory Manager initialized\n");

  /* Scheduler */
  if ( sched_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Scheduler initialized\n");

  /* Threads */
  if ( thread_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Threads initialized\n");


  /* Processes */
  if ( proc_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Processes initialized\n");


  /* Kernel process creation */
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

 
  /* Idle thread */
  thread_idle = thread_create_kern("[Idle]",THREAD_ID_DEFAULT,(virtaddr_t)klib_idle,NULL,THREAD_NICE_TOP,THREAD_QUANTUM_DEFAULT);
  if ( thread_idle == NULL )
    {
      goto err00;
    }
  klib_printf("Idle Thread initialized\n");

  /* Testing kernel threads */
  th_add = thread_create_kern("Add_thread",THREAD_ID_DEFAULT,(virtaddr_t)Add,(void*)12,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);
  th_calc = thread_create_kern("Calc_thread",THREAD_ID_DEFAULT,(virtaddr_t)Calc,(void*)IPC_ANY,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);
  th_mult = thread_create_kern("Mult_thread",THREAD_ID_DEFAULT,(virtaddr_t)Mult,(void*)15,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);
  klib_printf("Test Kernel Threads initialized\n");


  /* Gather testing threads and idle thread in kernel process */
  proc_add_thread(kern_proc,kern_th);
  proc_add_thread(kern_proc,thread_idle);
  proc_add_thread(kern_proc,th_add);
  proc_add_thread(kern_proc,th_mult);
  proc_add_thread(kern_proc,th_calc);



  /* Arbitrary address for user stack (testing purpose) */
  v_stack = 0x90000000;


  /* Boot modules are user processes (testing purpose) */
  for(mod = (struct multiboot_mod_entry *) start_mbi->mods_addr, i=0;
      i < start_mbi->mods_count;
      mod++,i++)
    {
        /* Create an user process */
      user_proc = proc_create(proc_names[i]);

      /* Get on user page directory to create virtual/physical mappings
	 (the kernel part is already sync by process creation, so it is seamless) */
      klib_load_CR3(user_proc->p_pd);

      /* Map code/data */
      paddr=mod->start;
      for(v_entry=0x80000000, paddr=mod->start;
	  paddr < mod->end ;
	  v_entry += CONST_PAGE_SIZE, paddr+= CONST_PAGE_SIZE)
	{
	  
	  if (paging_map(v_entry,paddr,PAGING_USER) == EXIT_FAILURE) 
	    {
	      goto err00;
	    }
	}
  
      /* Map stack */
      paddr = (physaddr_t)phys_alloc(CONST_PAGE_SIZE); 
      if (!paddr)
	{ 
	  goto err00; 
	} 
      if (paging_map(v_stack,paddr,PAGING_USER) == EXIT_FAILURE) 
	{ 
	  goto err00; 
	} 

      /*  Create an user thread with stack and entry point in this address space */
      th_user = thread_create_user(th_names[i],THREAD_ID_DEFAULT,0x80000000,v_stack,CONST_PAGE_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT); 
      if (th_user == NULL) 
	{ 
	  goto err00; 
	} 
  
      /* Back to kernel address space */
      klib_load_CR3((physaddr_t)kern_PD); 
  
      /* Add the new thread to the user process */
      proc_add_thread(user_proc,th_user);
    }
  
  /* Set the current process */
  cur_proc = kern_proc;
  
    /* IRQ can be enabled now */
  if ( irq_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("IRQ System initialized\n");

  /* Clock */
  if ( pit_init() != EXIT_SUCCESS )
    {
      goto err00;
    }
  klib_printf("Clock (100Hz) initialized\n");

  /* Restore interrupts */
  klib_sti();

  /* Non ending loop (we should not stay here after first clock tick) */
 err00:
  
  while(1)
    {
    }

  return EXIT_SUCCESS;
}

