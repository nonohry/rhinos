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
   - arch_io   : architecture dependent io library
   - boot.h    : structure boot_info

**/


#include <define.h>
#include <types.h>
#include <arch_io.h>
#include "mem.h"
#include "thread.h"
#include "boot.h"


/**

   Function: int main(void)
   ------------------------

   Kernel main function

   It is responsible for initializing kernel services and launching servers

**/


PUBLIC int main(void)
{

  arch_printf("Hello World (from main) !\n");
  if (mem_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to intialize kernel memory manager\n");
      goto err;
    }

  
  virtaddr_t s = (virtaddr_t)mem_alloc(64);
  struct thread* th = thread_create("toto",
				    0xBADBEEF,
				    s,
				    64);

  if (th != NULL)
    {
      arch_printf("Hello my name is %s\n",th->name);
      arch_printf("My entry point is at 0x%x\n",th->ctx.eip);
      arch_printf("My stack is at 0x%x and is 0x%x bytes long\n",
		  th->stack_base,
		  th->stack_size);
    }

  err:
 
  while(1)
    {
    }

  return EXIT_SUCCESS;
}

