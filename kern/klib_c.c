/**
   
   klib_c.c
   ========

   Kernel utilities library - C code

**/ 


/**

   Includes
   --------

   - types.h
   - klib.h.h            : self header

**/

#include <types.h>
#include "klib.h"


/**

   Private: Prototypes
   -------------------

**/

PRIVATE void klib_putc(char c);


/**
   
   Function: void klib_serial_init(void)
   -------------------------------------

   Initialize serial port sending parameter to the approriate ports

**/

PUBLIC void klib_serial_init(void)
{

  klib_outb(KLIB_SERIAL_PORT + 1, 0x00);    /* No interrupt   */
  klib_outb(KLIB_SERIAL_PORT + 3, 0x80);    /* DLAB (divisor) */
  klib_outb(KLIB_SERIAL_PORT + 0, 0x03);    /* Divisor (3 == 38400 bauds) - MSB */
  klib_outb(KLIB_SERIAL_PORT + 1, 0x00);    /* Divisor - LSB  */
  klib_outb(KLIB_SERIAL_PORT + 3, 0x03);    /* 8 bits, no parity, 1 stop bit    */

 return;
}


/** 
    Function: void klib_printf(const char* str,...)
    -----------------------------------------------

    kernel printf like. Supported switches:

    - d : print a signed 32b integer
    - u : print an unsigned 32b integer
    - x : print a hexadecimal 32b value
    - s : print a string

**/


PUBLIC void klib_printf(const char* str,...)
{

  char c;
  u8_t neg=0;
  u32_t val_n;
  char* val_s;
  char buf[11]; /* number of characters in a u32_t */
  char* itoa = "0123456789ABCDEF";
  u8_t base=0;
  u32_t* arg = (u32_t*)&str;

  while( (c=*str++) != 0 )
    {
      /* Special character '%' */
      if (c == '%')
	{
	  /* Get the next char */
	  c=*str++;
	  neg=0;
	  val_s=NULL;

	  switch(c)
	    {
	    case 'd':
	      {
		/* Get next arg */
		arg++;
		/* Positive the number if negative in order to print it */
		if ((s32_t)(*arg) < 0)
		  {
		    val_n=-(s32_t)(*arg);
		    neg=1;
		  }
		else
		  {
		    val_n=(s32_t)(*arg);
		  }
	
		base=10;
		break;
	      }
	    case 'u':
	      {
		/* Get next arg */
		arg++;
		val_n = (u32_t)(*arg);
		base=10;
		break;
	      }
	    case 'x':
	      {
		/* Get next arg */
		arg++;
		val_n = (u32_t)(*arg);
		base=16;
		break;
	      }
	    case 's':
	      {
		/* Get next arg */
		arg++;
		val_s = (char*)(*arg);
		base=0;
		break;
	      }
	    case '%':
	      {
		val_s = "%";
		base=0;
		break;
	      }
	    default:
	      {
		val_s = "%?";
		base=0;
		break;
	      }
	    }

	  /* Convert number into string */
	  if (val_s == NULL)
	    {
	      
	      /* Decomposition provides individual numbers in reverse order
		 We use a buffer string to store those numbers from the end to the beginning */

	      /* Go to the buffer string end */
	      val_s = buf + sizeof(buf)-1;
	      /* Set end of string */
	      *val_s = 0;
	      do
		{
		  /* Run throught the characters */
		  --val_s;
		  /* Set character */
		  *val_s = itoa[val_n % base];
		  /* Run through decomposition */
		  val_n /= base;
		}while(val_n>0);

	    }

	  /* Negative number gets a leading '-' */
	  if (neg)
	    {
	      klib_putc('-');
	    }

	  /* Print the string */
	  while(*val_s != 0)
	    {
	      klib_putc(*val_s);
	      val_s++;
	    }

	}
      else
	{
	  /* Unix newline case */
	  if (c == '\n')
	    {
	      klib_putc('\r');
	    }
	  klib_putc(c);
	}
    }
 
  return;
}


/**

   Function: void klib_putc(char c)
   --------------------------------

   Put a character on serial port

**/


PRIVATE void klib_putc(char c)
{
  u32_t i;
  u8_t buf;

  /* Wait for line release */
  for(i=0;i<12345;i++)
    {
      klib_inb(KLIB_SERIAL_PORT+5,&buf);
      if (buf & 0x20)
  	{
  	  break;
  	}
    }

  /* Put character */
  klib_outb(KLIB_SERIAL_PORT,c);
 
  return;
}
