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


/**

   Function: void klib_putc(char c)
   --------------------------------

   Put a character on serial port

**/


PUBLIC void serial_putc(char c)
{
  u32_t i;
  u8_t buf;

  /* Wait for line release */
  for(i=0;i<12345;i++)
    {
      x86_inb(SERIAL_PORT+5,&buf);
      if (buf & 0x20)
  	{
  	  break;
  	}
    }

  /* Put character */
  x86_outb(SERIAL_PORT,c);
 
  return;
}
