/*
 * Gestion logicielle des IRQ
 *
 */

#include <types.h>
#include "const.h"
#include <llist.h>
#include "klib.h"
#include "interrupt.h"
#include "pic.h"
#include "thread.h"
#include "irq.h"


/*======================================================================== 
 * Private
 *========================================================================*/

PRIVATE struct int_node* irq_flih[IRQ_VECTORS];


/*========================================================================
 * Initialisation
 *========================================================================*/

PUBLIC u8_t irq_init(void)
{
  u8_t i;

  /* Initialise les listes */

  for(i=0;i<IRQ_VECTORS;i++)
    {
      LLIST_NULLIFY(irq_flih[i]);
    }

  return EXIT_SUCCESS;
}


/*========================================================================
 * Activation/Desactivation d une irq
 *========================================================================*/

PUBLIC void irq_enable(u8_t irq)
{
  pic_enable_irq(irq);
  return;
}

PUBLIC void irq_disable(u8_t irq)
{
  pic_disable_irq(irq);
  return;
}


/*========================================================================
 * Ajout d un handler - fonction pour le boot
 *========================================================================*/

PUBLIC void irq_add_flih(u8_t irq, struct int_node* node)
{
  if (irq < IRQ_VECTORS)
    {
      /* Ajout du noeud au tableau des flih */
      LLIST_ADD(irq_flih[irq],node);
      
      /* Active au besoins l irq */
      irq_enable(irq);
    }

  return;
}


/*========================================================================
 * Retrait d un handler - fonction pour le boot
 *========================================================================*/

PUBLIC void irq_remove_flih(u8_t irq, struct int_node* node)
{
  if (irq < IRQ_VECTORS)
    {
      
      if (!LLIST_ISNULL(irq_flih[irq]))
	{
	  /* Enleve l element de la liste */
	  LLIST_REMOVE(irq_flih[irq],node);
	  
	  /* Retourne */
	  return;  
	}
    }
  return;
}



/*========================================================================
 * Execution des flih
 *========================================================================*/

PUBLIC void irq_handle_flih(u8_t irq, struct thread* th)
{
  if (irq < IRQ_VECTORS)
    {
      if (!LLIST_ISNULL(irq_flih[irq]))
	{
	  /* Noeud de parcours */
	   struct int_node* node;

	   node = irq_flih[irq];
	   do
	     {
	       /* Execute le flih */
	       node->flih(th);
	       /* Suite du parcours */
	       node = LLIST_NEXT(irq_flih[irq],node);

	     }while(!LLIST_ISHEAD(irq_flih[irq],node));

	}
    }

  return;
}
