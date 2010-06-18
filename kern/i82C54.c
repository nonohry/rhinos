/*
 * Gestion du timer i82C54
 *
 */

#include <types.h>
#include "klib.h"
#include "prot.h"
#include "proc.h"
#include "i8259.h"
#include "i82C54.h"

/************************ 
 * Declarations PRIVATE 
 ************************/

PRIVATE u8_t clock_handler();

/*******************************************
 * Initialise l'horloge avec une frequence
 *******************************************/

PUBLIC void clock_init()
{
  u32_t ticks;

  /* Determine le nombre de pulsations horloge avant interruption */
  ticks = CLOCK_MAX_FREQ/CLOCK_FREQ;

  /* Les compteurs sont sur 16bits
   * La valeur max est donc 2^16=65535
   * La valeur 65536 equivaut a 0 
   */
  if (ticks <= 65536)
    {
      /* 65536 = 0 */
      ticks = (ticks==65536?0:ticks);

      /* Envoie le mot de controle */
      outb(CLOCK_CWREG,CLOCK_MODE2);

      /* Envoie la frequence */
      outb(CLOCK_COUNTER0,(u8_t)ticks);        /* LSB d abord */
      outb(CLOCK_COUNTER0,(u8_t)(ticks>>8));   /* MSB ensuite */
    }

  irq_add_handler(0,&clock_handler,&clock_chaine);

  return;
}

/*************
 * Handler 
 *************/

PRIVATE u8_t clock_handler()
{
  
  task_schedule();
  
  return TRUE;
}

/*****************
 * Lit l horloge
 *****************/

PUBLIC u16_t clock_read()
{
  u8_t r1,r2;

  outb(CLOCK_CWREG,CLOCK_LATCH); /* Active le counter latch */
  inb(CLOCK_COUNTER0,&r1);      /* Lit le LSB */
  inb(CLOCK_COUNTER0,&r2);      /* Lit le MSB */

  return ( r1 | (r2<<8) );
}
