/**

   setup.c
   =======

   Set up a clean x86 environnement for kernel

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - serial.h       : output on serial port
   - setup.h        : self header

**/

#include <define.h>
#include <types.h>
#include "serial.h"
#include "setup.h"



/**

   Function: void setup_x86(u32_t magic, physaddr_t mbi_addr)
   -----------------------------------------------------------

   Entry point.
   Initialize serial port for external communication
   Retrieve memory information from bootloader and correct them
   Check boot modules (user progs)
   Create GDT & IDT

**/


PUBLIC void setup_x86(u32_t magic, physaddr_t mbi_addr)
{

  /* Initialize serial port */
  serial_init();

  /* Hello world */
  serial_printf("Hello World !\n");

  /* Debug loop */
  while(1)
    {}

  return;
}
