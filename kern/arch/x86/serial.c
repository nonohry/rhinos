/**

   serial.c
   ========

   Basic serial port management

**/



/**

   Includes
   --------

   - define.h
   - types.h
   - x86_lib.h       : assembly utilities
   - serial.h        : self header

**/

#include <define.h>
#include <types.h>
#include "x86_lib.h"
#include "serial.h"


/**
   
   Function: void serial_init(void)
   -------------------------------------

   Initialize serial port sending parameter to the approriate ports

**/

PUBLIC void serial_init(void)
{

  x86_outb(SERIAL_PORT + 1, 0x00);    /* No interrupt   */
  x86_outb(SERIAL_PORT + 3, 0x80);    /* DLAB (divisor) */
  x86_outb(SERIAL_PORT + 0, 0x03);    /* Divisor (3 == 38400 bauds) - MSB */
  x86_outb(SERIAL_PORT + 1, 0x00);    /* Divisor - LSB  */
  x86_outb(SERIAL_PORT + 3, 0x03);    /* 8 bits, no parity, 1 stop bit */

 return;
}
