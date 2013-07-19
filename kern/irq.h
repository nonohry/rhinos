/**

   irq.h
   =====

   IRQ header file

**/

#ifndef IRQ_H
#define IRQ_H


/**

   Includes
   --------

   - define.h
   - types.h
   - interrupt.h      : struct int_node needed
   
**/

#include <define.h>
#include <types.h>
#include "interrupt.h"



/**

   Structure: struct int_node
   --------------------------

   Link all the first level interrupt handlers  sharing the same IRQ.
   Members are:

   - flih     : first level interrupt handler 
   - prev     : previous item in the linked list 
   - next     : next item in the linked list

**/
   

PUBLIC struct irq_node
{
  void (*flih)();
  struct irq_node* prev;
  struct irq_node* next;
};


/**

   Prototypes
   ----------

   Declare 6 functions in order to:
   
   - initialize IRQ subsystem
   - enable or disable an IRQ line
   - add or remove an handler for an IRQ vector
   - handle an IRQ vector

**/


PUBLIC u8_t irq_setup(void);
PUBLIC void irq_enable(u8_t irq);
PUBLIC void irq_disable(u8_t irq);
PUBLIC void irq_add_flih(u8_t irq, struct irq_node* node);
PUBLIC void irq_remove_flih(u8_t irq, struct irq_node* node);
PUBLIC void irq_handle_flih(u8_t irq);

#endif
