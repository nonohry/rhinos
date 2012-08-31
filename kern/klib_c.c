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
  u8_t neg=0;
  u32_t val_n;
  char* val_s;
  u8_t base=0;
  u32_t* arg = (u32_t*)&str;
  
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
	  neg=0;
	  val_s=NULL;

	  switch(c)
	    {
	    case 'd':
	      {
		/* Recupere l'argument suivant */
		arg++;
		/* Positive le nombre signe */
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
		/* Recupere l'argument suivant */
		arg++;
		val_n = (u32_t)(*arg);
		base=10;
		break;
	      }
	    case 'x':
	      {
		/* Recupere l'argument suivant */
		arg++;
		val_n = (u32_t)(*arg);
		base=16;
		break;
	      }
	    case 's':
	      {
		/* Recupere l'argument suivant */
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
		val_s = "?";
		base=0;
		break;
	      }
	    }

	  if (val_s == NULL)
	    {
	      if (neg)
		{
		  klib_bochs_print("-");
		}
	      klib_bochs_print("%d",val_n);
	    }
	  else
	    {
	      klib_bochs_print(val_s);
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
