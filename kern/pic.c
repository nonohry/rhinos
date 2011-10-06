/*
 * Gestion du PIC i8259A
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "const.h"
#include "klib.h"
#include "pic.h"


/*========================================================================
 * Initialisation du PIC i8259
 *========================================================================*/

PUBLIC void pic_init()
{

  /* Envoie ICW1 sue les ports maitre et esclave */
  klib_outb(PIC_MASTER_PORT,PIC_ICW1);
  klib_outb(PIC_SLAVE_PORT,PIC_ICW1);

  /* Puis ICW2 */
  klib_outb(PIC_MASTER_PORT+1,PIC_ICW2_MASTER);
  klib_outb(PIC_SLAVE_PORT+1,PIC_ICW2_SLAVE);

  /* ICW3 a la suite */
  klib_outb(PIC_MASTER_PORT+1,PIC_ICW3_MASTER);
  klib_outb(PIC_SLAVE_PORT+1,PIC_ICW3_SLAVE);

  /* Enfin, ICW4 */
  klib_outb(PIC_MASTER_PORT+1,PIC_ICW4);
  klib_outb(PIC_SLAVE_PORT+1,PIC_ICW4);

  /* Tant qu'on n'a pas de gestionnaire
   * d'interruptions, on les desactive 
   * excepte la ligne 2 (cascade)
   */

  klib_outb(PIC_MASTER_PORT+1,0xFB);
  klib_outb(PIC_SLAVE_PORT+1,0xFF);

  return;
}


/*========================================================================
 * Activation d'une ligne IRQ
 *========================================================================*/

PUBLIC void pic_enable_irq(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < PIC_VECTORS)
    {
      u8_t read;
      u16_t port;

      /* Determine le PIC concerne */
      port = (n<8)?PIC_MASTER_PORT+1:PIC_SLAVE_PORT+1;

      /* Active la ligne */
      klib_inb(port,&read);
      read &= ~(1<<n);
      klib_outb(port,read);
    }

  return;
}

/*========================================================================
 * Desactivation d'une ligne IRQ
 *========================================================================*/

PUBLIC void pic_disable_irq(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < PIC_VECTORS)
    {
      u8_t read;
      u16_t port;

      /* Determine le PIC concerne */
      port = (n<8)?PIC_MASTER_PORT+1:PIC_SLAVE_PORT+1;

      /* Desactive la ligne */
      klib_inb(port,&read);
      read |= (1<<n);
      klib_outb(port,read);
    }

  return;
}
