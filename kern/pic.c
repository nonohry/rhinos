/*
 * Gestion du PIC i8259A
 *
 */


/*************
 * Includes 
 *************/

#include <types.h>
#include "klib.h"
#include "pic.h"


/*******************************
 * Initialisation du PIC i8259
 *******************************/

PUBLIC void pic_init()
{

  /* Envoie ICW1 sue les ports maitre et esclave */
  outb(IRQ_MASTER_PORT,IRQ_ICW1);
  outb(IRQ_SLAVE_PORT,IRQ_ICW1);

  /* Puis ICW2 */
  outb(IRQ_MASTER_PORT+1,IRQ_ICW2_MASTER);
  outb(IRQ_SLAVE_PORT+1,IRQ_ICW2_SLAVE);

  /* ICW3 a la suite */
  outb(IRQ_MASTER_PORT+1,IRQ_ICW3_MASTER);
  outb(IRQ_SLAVE_PORT+1,IRQ_ICW3_SLAVE);

  /* Enfin, ICW4 */
  outb(IRQ_MASTER_PORT+1,IRQ_ICW4);
  outb(IRQ_SLAVE_PORT+1,IRQ_ICW4);

  /* Tant qu'on n'a pas de gestionnaire
   * d'interruptions, on les desactive 
   * excepte la ligne 2 (cascade)
   */

  outb(IRQ_MASTER_PORT+1,0xFB);
  outb(IRQ_SLAVE_PORT+1,0xFF);

  return;
}


/*******************************
 * Activation d'une ligne IRQ
 *******************************/

PUBLIC void pic_enable_irq(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < PIC_VECTORS)
    {
      u8_t read;
      u16_t port;

      /* Determine le PIC concerne */
      port = (n<8)?IRQ_MASTER_PORT+1:IRQ_SLAVE_PORT+1;

      /* Active la ligne */
      inb(port,&read);
      read &= ~(1<<n);
      outb(port,read);
    }

  return;
}

/*********************************
 * Desactivation d'une ligne IRQ
 *********************************/

PUBLIC void pic_disable_irq(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < PIC_VECTORS)
    {
      u8_t read;
      u16_t port;

      /* Determine le PIC concerne */
      port = (n<8)?IRQ_MASTER_PORT+1:IRQ_SLAVE_PORT+1;

      /* Desactive la ligne */
      inb(port,&read);
      read |= (1<<n);
      outb(port,read);
    }

  return;
}
