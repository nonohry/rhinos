/**

   pit.c
   =====

   i82C54 Programmable Interval Timer management
 
**/




/**

   Includes
   --------

   - define.h
   - types.h
   - const.h
   - klib.h
   - interrupt.h : Needed to create clock handler
   - irq.h       : Needed to activate IRQ line
   - thread.h    : struct thread needed
   - sched.h     : sched_schedule needed
   - pit.h       : self header
   
**/

#include <define.h>
#include <types.h>
#include "const.h"
#include "klib.h"
#include "interrupt.h"
#include "irq.h"
#include "thread.h"
#include "sched.h"
#include "pit.h"



/**

   Privates
   --------

   Interrupt handler and its node

**/


PRIVATE void pit_handler(struct thread* th);
PRIVATE struct int_node pit_irq_node;


/**

   Function: u8_t pit_init(void)
   -----------------------------

   Programmable Interval Timer initialization.
   Configure clock mode and frequency.

**/

PUBLIC u8_t pit_init(void)
{
  u32_t ticks;

  /* Compute number of clock pulsations before triggering interrupt */
  ticks = PIT_MAX_FREQ/PIT_FREQ;

  /* counter are 16 bits values
     max value is 2^16=65535 and 65536 is in fact 0 */
   
  if (ticks <= 65536)
    {
      /* 65536 == 0 */
      ticks = (ticks==65536?0:ticks);

      /* Send control word to active Mode 2 (Rate Generator) */
      klib_outb(PIT_CWREG,PIT_MODE2);

      /* Send frequency for this mode */
      klib_outb(PIT_COUNTER0,(u8_t)ticks);        /* LSB first */
      klib_outb(PIT_COUNTER0,(u8_t)(ticks>>8));   /* MSB last */
    }
  
  /* Create an irq node to setup handler */
  pit_irq_node.flih = pit_handler;
  irq_add_flih(0,&pit_irq_node);

  return EXIT_SUCCESS;
}


/**
 
   Function:  void pit_handler(struct thread* th)
   ----------------------------------------------

   First level interrupt handler in charge of clock.
   For the moment, just call the scheduler.

**/

PRIVATE void pit_handler(struct thread* th)
{

  /* Scheduler */
  sched_schedule(SCHED_FROM_PIT);

  return;
}


/**
   
   Function: u16_t pit_read(void)
   ------------------------------

   Read the PIT

**/

PUBLIC u16_t pit_read(void)
{
  u8_t r1,r2;

  klib_outb(PIT_CWREG,PIT_LATCH);  /* Activate counter latch */
  klib_inb(PIT_COUNTER0,&r1);      /* Read LSB first */
  klib_inb(PIT_COUNTER0,&r2);      /* Read MSB last  */

  return ( r1 | (r2<<8) );
}
