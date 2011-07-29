/*
 * Gestion du timer i82C54
 *
 */

#include <types.h>
#include "klib.h"
#include "tables.h"
#include "irq.h"
#include "pit.h"

/*======================================================================== 
 * Declarations PRIVATE 
 *========================================================================*/

PRIVATE void pit_handler();
PRIVATE struct irq_node pit_irq_node;

/*========================================================================
 * Initialise l'horloge avec une frequence
 *========================================================================*/

PUBLIC void pit_init()
{
  u32_t ticks;

  /* Determine le nombre de pulsations horloge avant interruption */
  ticks = PIT_MAX_FREQ/PIT_FREQ;

  /* Les compteurs sont sur 16bits
   * La valeur max est donc 2^16=65535
   * La valeur 65536 equivaut a 0 
   */
  if (ticks <= 65536)
    {
      /* 65536 = 0 */
      ticks = (ticks==65536?0:ticks);

      /* Envoie le mot de controle */
      outb(PIT_CWREG,PIT_MODE2);

      /* Envoie la frequence */
      outb(PIT_COUNTER0,(u8_t)ticks);        /* LSB d abord */
      outb(PIT_COUNTER0,(u8_t)(ticks>>8));   /* MSB ensuite */
    }
  
  /* Cree le noeud irq */
  pit_irq_node.flih = pit_handler;
  irq_add_flih(0,&pit_irq_node);

  return;
}


/*========================================================================
 * Handler (flih) 
 *========================================================================*/

PRIVATE void pit_handler()
{
  
   bochs_print("tick ! ");

  return;
}

/*========================================================================
 * Lit l horloge
 *========================================================================*/

PUBLIC u16_t pit_read()
{
  u8_t r1,r2;

  outb(PIT_CWREG,PIT_LATCH); /* Active le counter latch */
  inb(PIT_COUNTER0,&r1);      /* Lit le LSB */
  inb(PIT_COUNTER0,&r2);      /* Lit le MSB */

  return ( r1 | (r2<<8) );
}
