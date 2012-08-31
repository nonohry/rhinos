/*
 * klib_c.c
 * Librairie noyau en C
 *
 */



/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "klib.h"


/*========================================================================
 * Declaration Private
 *========================================================================*/

PRIVATE void klib_putc(char c);



PUBLIC void klib_serial_init(void)
{
  /* Initialise le port serie */

  klib_outb(KLIB_SERIAL_PORT + 1, 0x00);    // Disable all interrupts
  klib_outb(KLIB_SERIAL_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
  klib_outb(KLIB_SERIAL_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
  klib_outb(KLIB_SERIAL_PORT + 1, 0x00);    //                  (hi byte)
  klib_outb(KLIB_SERIAL_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit

 return;
}

/*========================================================================
 * Printf Noyau
 *========================================================================*/

PUBLIC void klib_printf(const char* str,...)
{

  char c;
  
  while( (c=*str++) != 0 )
    {
      /* Cas du retour a la ligne Unix */
      if (c == '\n')
	{
	  klib_putc('\r');
	}

      /* Traitement si caractere special % */
      if (c == '%')
	{
	  c=*str++;
	  switch(c)
	    {
	    case 'd':
	      {
		break;
	      }
	    case 'u':
	      {
		break;
	      }
	    case 'x':
	      {
		break;
	      }
	    case 's':
	      {
		break;
	      }
	    case '%':
	      {
		break;
	      }
	    default:
	      {
		break;
	      }
	    }
	}
      else
	{
	  klib_putc(c);
	}
    }
 
  return;
}


/*========================================================================
 * Affiche un caractere sur le port serie
 *========================================================================*/


PRIVATE void klib_putc(char c)
{
  u32_t i;
  u8_t buf;

  /* Attend que la ligne soit libre */
  for(i=0;i<123456;i++)
    {
      klib_inb(KLIB_SERIAL_PORT+5,&buf);
      if (buf & 0x20)
	{
	  break;
	}
    }

  /* Emet le caractere */
  klib_outb(KLIB_SERIAL_PORT,c);

  return;
}
