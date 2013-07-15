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
  serial_printf("Got %d boot modules, mmap at 0x%x and first available byte at 0x%x\n",
		boot.mods_count,
		boot.mmap_addr,
		boot.start);

  while(1)
    {
    }

  return EXIT_SUCCESS;
}

