/**

   clock.c
   =======

   Clock management

**/




/**

   Includes
   --------

   - define.h
   - types.h
   - irq.h     : irq_node needed
   - thread.h  : thread switch needed
   - sched.h   : scheduler needed
   - clock.h   : self header

**/


#include <define.h>
#include <types.h>
#include <arch_io.h>
#include "irq.h"
#include "thread.h"
#include "sched.h"
#include "clock.h"


/**

   Private: void clock_handler(void)
   ---------------------------------

   Clock first level interrupt handler

**/


PRIVATE void clock_handler(void);


/**

   Static: clock_irq_node
   ----------------------

   Clock irq node

**/

static struct irq_node clock_irq_node;



/**

   Function: u8_t clock_setup(void)
   --------------------------------

   Set up the clock

   Create an `irq_node` for IRQ 0

**/

PUBLIC u8_t clock_setup(void)
{

  /* Create an irq node to setup handler */
  clock_irq_node.flih = clock_handler;
  irq_add_flih(0,&clock_irq_node);

  return EXIT_SUCCESS;
}


/**
 
   Function:  void clock_handler(void)
   ------------------------------------

   First level interrupt handler in charge of clock.
   For the moment, just call the scheduler.

**/

PRIVATE void clock_handler()
{

  struct thread* th;

  /* Scheduler */
  th = sched_elect();
  if (th)
    {
      arch_printf(" Elected: %s ", th->name);
    }
  thread_switch_to(th);

  return;
}
