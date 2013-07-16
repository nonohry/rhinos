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

  char* a;
  while( (a=(char*)mem_alloc(99)) != NULL)
    {
      arch_printf("Allocation success: 0x%x\n",(u32_t)a);
    }

 err:
 
  while(1)
    {
    }

  return EXIT_SUCCESS;
}

