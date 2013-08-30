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
      //arch_printf("_");
      for(i=0;i<4321;i++)
	{}

    }

  return EXIT_SUCCESS;
}

