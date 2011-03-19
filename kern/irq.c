/*
 * Gestion logicielle des IRQ
 *
 */

#include <types.h>
#include <llist.h>
#include "bootmem.h"
#include "pic.h"
#include "irq.h"


/*********** 
 * Private
 ***********/

PRIVATE struct irq_node* irq_flih[IRQ_VECTORS];


/*****************
 * Initialisation
 *****************/

PUBLIC void irq_init(void)
{
  u8_t i;

  /* Initialise les listes */

  for(i=0;i<IRQ_VECTORS;i++)
    {
      LLIST_NULLIFY(irq_flih[i]);
    }

  return;
}


/**************************************
 * Activation/Desactivation d une irq
 **************************************/

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


/*********************************************
 * Ajout d un handler - fonction pour le boot
 *********************************************/

PUBLIC void irq_boot_add_flih(u8_t irq, irq_flih_t func)
{
  if (irq < IRQ_VECTORS)
    {
      struct irq_node* node;
      
      /* Creation et allocation de l irq_node */
      node = (struct irq_node*)bootmem_alloc(sizeof(struct irq_node));
      node->flih = func;

      /* Ajout du noeud au tableau des flih */
      if (LLIST_ISNULL(irq_flih[irq]))
	{
	  irq_flih[irq]=node;
	  LLIST_HEAD(irq_flih[irq]);
	}
      else
	{
	  LLIST_ADD(irq_flih[irq],node);
	}     
      
      irq_enable(irq);
    }

  return;
}


/***********************************************
 * Retrait d un handler - fonction pour le boot
 ***********************************************/

PUBLIC void irq_boot_remove_flih(u8_t irq, irq_flih_t func)
{
  if (irq < IRQ_VECTORS)
    {
      
      if (!LLIST_ISNULL(irq_flih[irq]))
	{
	  /* Noeud de parcours */
	  struct irq_node* node;
	  node=irq_flih[irq];

	  do
	    {
	      /* Cherche le flih correspondant */
	      if (node->flih == func)
		{
		  /* Enleve l element de la liste */
		  LLIST_REMOVE(irq_flih[irq],node);
		  
		  /* Libere la zone memoire */
		  bootmem_free(node,sizeof(struct irq_node));
		  
		  /* Retourne */
		  return;  
		}

	      /* Suite du parcours */
	      node=LLIST_NEXT(irq_flih[irq],node);

	    }while(!LLIST_ISHEAD(irq_flih[irq],node));
	}
    }

  return;
}
