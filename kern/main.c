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

**/


#include <define.h>
#include <types.h>
#include <arch_io>


/**

   Function: int main(void)
   ------------------------

   Kernel main function

   It is responsible for initializing kernel services and launch servers

**/


PUBLIC int main(void)
{

  arch_printf("Hello World !\n");

  while(1)
    {
    }

  return EXIT_SUCCESS;
}

