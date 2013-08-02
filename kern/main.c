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

  arch_printf("Hello World (from main) !\n");

  if (pager0_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to setup Pager0\n");
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



  struct thread kern_th;
  kern_th.ctx.ss = 16;
  kern_th.name[0] = '[';
  kern_th.name[1] = 'K';
  kern_th.name[2] = 'e';
  kern_th.name[3] = 'r';
  kern_th.name[4] = 'n';
  kern_th.name[5] = ']';
  kern_th.name[6] = 0;
  
  kern_th.state = THREAD_READY;
  sched_enqueue(SCHED_READY_QUEUE,&kern_th);
  cur_th = &kern_th;
  
  arch_sti();
 
  int* p = 0x80000000;
 
  //int pp = *p;
  *p = 9;
  
 err:
  
  while(1)
    {
      //arch_printf("_");
      for(i=0;i<4321;i++)
	{}

    }

  return EXIT_SUCCESS;
}

