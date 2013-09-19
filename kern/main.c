/**

   main.c
   ======

    Kernel main function. All initializations occur here

**/



/**
   
   Includes
   --------
   
   - define.h
   - types.h
   - arch_io.h   : architecture dependent io library
   - arch_hw.h   : architecture dependent sti
   - boot.h      : structure boot_info

**/


#include <define.h>
#include <types.h>
#include <arch_io.h>
#include <arch_hw.h>
#include "thread.h"
#include "irq.h"
#include "boot.h"
#include "pager0.h"
#include "vm_pool.h"
#include "vm_slab.h"
#include "thread.h"
#include "proc.h"
#include "sched.h"
#include "clock.h"


/**

   Function: int main(void)
   ------------------------

   Kernel main function

   It is responsible for initializing kernel services and launching servers

**/


PUBLIC int main(void)
{

  u16_t i;

  if (pager0_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to setup Pager0\n");
      goto err;
    }
  
  if (vm_pool_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to setup virtual pages pool\n");
      goto err;
    } 


  if (vm_cache_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to setup slab allocator\n");
      goto err;
    }

  /* Restore interrupts to handle page faults (needed for thread cache) */
  arch_sti();
  
  if (thread_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to setup threads subsystem\n");
      goto err;
    }

  if (proc_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to setup process subsystem\n");
      goto err;
    }


  /* Boot modules */
  struct boot_mod_entry* mods = (struct boot_mod_entry*)(boot.mods_addr);
  for(i=0;i<boot.mods_count;i++)
    {
      arch_printf("0x%x - 0x%x (%s)\n",
		  mods[i].start,
		  mods[i].end,
		  mods[i].cmdline);
    }

  struct proc* ptest1;
  struct thread* thtest1;

  ptest1 = proc_create("ptest1");
  if (ptest1 == NULL)
    {
      arch_printf("Unable to create ptest1\n");
      goto err;
    }

  if (proc_memcopy(ptest1,mods[0].start,0x80000000,mods[0].end-mods[0].start) != EXIT_SUCCESS)
    {
      arch_printf("Unable to copy in ptest1\n");
      goto err;
    }


  thtest1 = thread_create("thtest1",0x80000000,0x90000000,0x200);
  if (thtest1 == NULL)
    {
      arch_printf("Unable to create in thtest1\n");
      goto err;
    }

  if (proc_add_thread(ptest1,thtest1) != EXIT_SUCCESS)
    {
      arch_printf("Unable to add thtest1 to ptest1\n");
      goto err;
    }

 
  struct proc* ptest2;
  struct thread* thtest2;

  ptest2 = proc_create("ptest2");
  if (ptest2 == NULL)
    {
      arch_printf("Unable to create ptest2\n");
      goto err;
    }

  if (proc_memcopy(ptest2,mods[1].start,0x80000000,mods[1].end-mods[1].start) != EXIT_SUCCESS)
    {
      arch_printf("Unable to copy in ptest2\n");
      goto err;
    }


  thtest2 = thread_create("thtest2",0x80000000,0x90000000,0x200);
  if (thtest2 == NULL)
    {
      arch_printf("Unable to create in thtest2\n");
      goto err;
    }

  if (proc_add_thread(ptest2,thtest2) != EXIT_SUCCESS)
    {
      arch_printf("Unable to add thtest2 to ptest2\n");
      goto err;
    }


  struct proc* ptest3;
  struct thread* thtest3;

  ptest3 = proc_create("ptest3");
  if (ptest3 == NULL)
    {
      arch_printf("Unable to create ptest3\n");
      goto err;
    }

  if (proc_memcopy(ptest3,mods[1].start,0x80000000,mods[1].end-mods[1].start) != EXIT_SUCCESS)
    {
      arch_printf("Unable to copy in ptest3\n");
      goto err;
    }


  thtest3 = thread_create("thtest3",0x80000000,0x90000000,0x200);
  if (thtest3 == NULL)
    {
      arch_printf("Unable to create in thtest3\n");
      goto err;
    }

  if (proc_add_thread(ptest3,thtest3) != EXIT_SUCCESS)
    {
      arch_printf("Unable to add thtest3 to ptest3\n");
      goto err;
    }


  if (irq_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to intialize IRQ subsystem\n");
      goto err;
    }

  if (clock_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to intialize clock\n");
      goto err;
    }

 err:
  
  while(1)
    {
      for(i=0;i<4321;i++)
	{}

    }

  return EXIT_SUCCESS;
}

