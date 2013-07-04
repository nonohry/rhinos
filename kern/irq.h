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
   - const.h
   - interrupt.h      : struct int_node needed
   
**/

#include <define.h>
#include <types.h>
#include "const.h"
#include "interrupt.h"


/**

   Constant: IRQ_VECTORS
   ---------------------

   Number of vectors

**/

#define IRQ_VECTORS   16


/**

   Prototypes
   ----------

   Declare 6 functions in order to:
   
   - initialize IRQ subsystem
   - enable or disable an IRQ line
   - add or remove an handler for an IRQ vector
   - handle an IRQ vector

**/


PUBLIC u8_t irq_init(void);
PUBLIC void irq_enable(u8_t irq);
PUBLIC void irq_disable(u8_t irq);
PUBLIC void irq_add_flih(u8_t irq, struct int_node* node);
PUBLIC void irq_remove_flih(u8_t irq, struct int_node* node);
PUBLIC void irq_handle_flih(u8_t irq, struct thread* th);

#endif
