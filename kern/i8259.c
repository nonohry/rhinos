/*
 * Gestion du PIC i8259A
 *
 */


/*************
 * Includes 
 *************/

#include <types.h>
#include "klib.h"
#include "prot.h"
#include "i8259.h"


/*******************************
 * Initialisation du PIC i8259
 *******************************/

PUBLIC void i8259_init()
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

PUBLIC void irq_enable(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < IRQ_VECTORS)
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

PUBLIC void irq_disable(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < IRQ_VECTORS)
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

/*******************************
 * Execution des ISR d'une IRQ
 *******************************/

PUBLIC void irq_handle(u8_t n)
{
  if (n < IRQ_VECTORS)
    {
      /* Debut de la chaine de handlers */
      struct irq_chaine* p;

      p = irq_handlers[n];
      
      /* Execution des handlers */
      while (p != NULL)
	{
	  /* Indique que l ISR est active */
	  irq_active[n] |= p->id;

	  /* ISR fini ? */
	  if (p->handler())
	  {
	    /* Indique que l ISR est inactive */
	    irq_active[n] &= ~p->id;
	  }

	  /* Suit la liste chainee */
	  p=p->next;
	}

    }

  return;
}

/********************************
 * Ajout d'une ISR pour une IRQ
 ********************************/

PUBLIC void irq_add_handler(u8_t n, irq_handler_t handler, struct irq_chaine* chaine)
{

  if (n < IRQ_VECTORS)
    {
      u32_t id;  /* ID de la nouvelle ISR */
      struct irq_chaine** p;  /* Pointeur pour lier la nouvelle chaine */

      p = &irq_handlers[n];
      chaine->next = NULL;
      chaine->handler = handler;
      chaine->irq = n;
   
      /* Determine l id */
      id=1;
      while ( *p != NULL )
	{
	  id = id << 1;
	  p = &(*p)->next;
	}

      /* Si id=0, on a trop shifte => plus de 32 ISR */
      if (id == 0)
	{
	  bochs_print("Too much handlers for IRQ\n");
	  return ;
	}

      /* Desormais on peut affecter l'id */
      chaine->id = id;

      /* On lie la nouvelle chaine */
      *p = chaine;
  
      /* Active l'IRQ au niveau des PICs */
      irq_enable(n);

    }
 
  return;
}

/********************************
 * Retrait d'une ISR pour une IRQ
 ********************************/

PUBLIC void irq_rm_handler(struct irq_chaine* chaine)
{
  u8_t irq;  /* IRQ */

  irq = chaine->irq;

  if (irq < IRQ_VECTORS)
    {
      u32_t id;  /* ID de la chaine */
      struct irq_chaine** p;  /* Pointeur pour le parcours de la chaine */

      id = chaine->id;
      p = &irq_handlers[irq];

      /* Cherche la chaine voulue */
      while ( *p != NULL )
	{
	  /* Si l id correspond */
	  if ((*p)->id == id)
	    {
	      *p = (*p)->next;  /* place l element suivant a sa place */
	      break; 
	    }

	  /* Poursuit la recherche */
	  p = &(*p)->next;
	}
      
      /* Mise a jour du champ id */
      p = &irq_handlers[irq];
      id=1;
      while ( *p != NULL )
	{
	  (*p)->id = id;   /* Reaffecte le champs id */
	  id = id << 1;    /* Shift */

	  /* Poursuit le parcours */
	  p = &(*p)->next;
	}
      

    }

  return;
}
