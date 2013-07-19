/**

   irq.c
   =====

   Hardware interrupt request handling

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - llist.h
   - arch_hw.h        : PIT manipulation
   - irq.h            : self header

**/

#include <define.h>
#include <types.h>
#include <llist.h>
#include <arch_hw.h>
#include "irq.h"


/**

   Constant: IRQ_VECTORS
   ---------------------

   Number of vectors

**/

#define IRQ_VECTORS   16



/**

   PRIVATE: irq_flih[IRQ_VECTORS]
   ------------------------------

   Array of IRQ handlers linked lists indexed by IRQ vectors

**/

PRIVATE struct irq_node* irq_flih[IRQ_VECTORS];


/**

   Function: u8_t irq_setup(void)
   -----------------------------

   Simply initialize linked lists in irq_flih

**/

PUBLIC u8_t irq_setup(void)
{
  u8_t i;

  for(i=0;i<IRQ_VECTORS;i++)
    {
      LLIST_NULLIFY(irq_flih[i]);
    }

  return EXIT_SUCCESS;
}


/**

   Function: void irq_enable(u8_t irq)
   -----------------------------------

   Enable an IRQ line activating the line in the PIC

**/
   

PUBLIC void irq_enable(u8_t irq)
{
  arch_enable_irq(irq);
  return;
}


/**

   Function: void irq_disable(u8_t irq)
   ------------------------------------

   Disable an IRQ line deactivating the line in the PIC

**/

PUBLIC void irq_disable(u8_t irq)
{
  arch_disable_irq(irq);
  return;
}


/**

   Function: void irq_add_flih(u8_t irq, struct int_node* node)
   ------------------------------------------------------------

   Add a flih to a IRQ line by simply adding a struct int_node in the correct linked list

**/

PUBLIC void irq_add_flih(u8_t irq, struct irq_node* node)
{
  if (irq < IRQ_VECTORS)
    {
      LLIST_ADD(irq_flih[irq],node);
      irq_enable(irq);
    }

  return;
}


/**

   Function: void irq_remove_flih(u8_t irq, struct int_node* node)
   ---------------------------------------------------------------

   Remove a flih from a IRQ line by simply removing its struct int_node from the correct linked list

**/

PUBLIC void irq_remove_flih(u8_t irq, struct irq_node* node)
{
  if (irq < IRQ_VECTORS)
    {
      
      if (!LLIST_ISNULL(irq_flih[irq]))
	{
	  LLIST_REMOVE(irq_flih[irq],node);
	}
    }
  return;
}



/**

   Function: void irq_handle_flih(u8_t irq)
   ----------------------------------------

   Flih handler. Run through the correct linked list executing the flih.
   All the flih for a given IRQ vector are executed. The flih must return immediately
   if they are not concerned by the IRQ event.

**/

PUBLIC void irq_handle_flih(u8_t irq)
{
  if (irq < IRQ_VECTORS)
    {
      if (!LLIST_ISNULL(irq_flih[irq]))
	{
	  struct irq_node* node;

	   node = irq_flih[irq];
	   do
	     {
	       /* Execute the flih */
	       node->flih();
	       /* Run through the list */
	       node = LLIST_NEXT(irq_flih[irq],node);

	     }while(!LLIST_ISHEAD(irq_flih[irq],node));

	}
    }

  return;
}
