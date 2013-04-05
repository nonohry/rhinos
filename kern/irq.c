/**

   irq.c
   =====

   Hardware interrupt request handling

**/


/**

   Includes
   --------

   - types.h
   - const.h
   - llist.h
   - klib.h
   - interrupt.h      : struct int_node needed
   - pic.h            : physically enable/disable IRQ line
   - thread.h         : struct thread needed
   - irq.h            : self header

**/

#include <types.h>
#include "const.h"
#include <llist.h>
#include "klib.h"
#include "interrupt.h"
#include "pic.h"
#include "thread.h"
#include "irq.h"


/**

   PRIVATE: irq_flih[IRQ_VECTORS]
   ------------------------------

   Array of IRQ handlers linked lists indexed by IRQ vectors

**/

PRIVATE struct int_node* irq_flih[IRQ_VECTORS];


/**

   Function: u8_t irq_init(void)
   -----------------------------

   Simply initialize linked lists in irq_flih

**/

PUBLIC u8_t irq_init(void)
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
  pic_enable_irq(irq);
  return;
}


/**

   Function: void irq_disable(u8_t irq)
   ------------------------------------

   Disable an IRQ line deactivating the line in the PIC

**/

PUBLIC void irq_disable(u8_t irq)
{
  pic_disable_irq(irq);
  return;
}


/**

   Function: void irq_add_flih(u8_t irq, struct int_node* node)
   ------------------------------------------------------------

   Add a flih to a IRQ line by simply adding a struct int_node in the correct linked list

**/

PUBLIC void irq_add_flih(u8_t irq, struct int_node* node)
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

PUBLIC void irq_remove_flih(u8_t irq, struct int_node* node)
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

   Function: void irq_handle_flih(u8_t irq, struct thread* th)
   -----------------------------------------------------------

   Flih handler. Run through the correct linked list executing the flih.
   All the flih for a given IRQ vector are executed. The flih must return immediately
   if they are not concerned by the IRQ event.

**/

PUBLIC void irq_handle_flih(u8_t irq, struct thread* th)
{
  if (irq < IRQ_VECTORS)
    {
      if (!LLIST_ISNULL(irq_flih[irq]))
	{
	  struct int_node* node;

	   node = irq_flih[irq];
	   do
	     {
	       /* Execute the flih */
	       node->flih(th);
	       /* Run through the list */
	       node = LLIST_NEXT(irq_flih[irq],node);

	     }while(!LLIST_ISHEAD(irq_flih[irq],node));

	}
    }

  return;
}
