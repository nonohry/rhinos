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
   - x86_lib.h
   - pit.h       : self header
   
**/


#include <define.h>
#include <types.h>
#include "x86_lib.h"
#include "pit.h"




/**

   Constants: PIT Frequency relatives
   ----------------------------------

**/

#define PIT_MAX_FREQ     1193182
#define PIT_FREQ         100        /* 100Hz */


/**

   Constants: I/O Ports 
   --------------------

**/

#define PIT_COUNTER0     0x40
#define PIT_COUNTER1     0x41
#define PIT_COUNTER2     0x42
#define PIT_CWREG        0x43


/**

   Constant: PIT_MODE2
   -------------------

   Control word to set mode 2.
   Mode 2 is Rate Generator: Output line is set to 1 every n clock pulsation, where n is the counter value.
   In other word, an interrupt is triggered at rate 1193182/n Hz

**/   

#define PIT_MODE2        0x34   /* 0x34=00110100b, Mode 2 pour le compteur 0 */


/**
   
   Constant: PIT_LATCH
   -------------------

   Counter Latch allow counter reads without affecting PIT. It is a command, which structure is:

   +-----+-----+-----+-----+-----+-----+-----+-----+
   | SC1 | SC0 |  0  |  0  |  0  |  0  |  0  |  0  |
   +-----+-----+-----+-----+-----+-----+-----+-----+

   - SC0 & SC1 : Indicate which counter to read. i82C54 has 3 counters :
        1- Counter 0 : connected to IRQ 0 line
	2- Counter 1 : RAM refresh
	3- Counter 2 : connected to loud speakers

**/

#define PIT_LATCH        0x00   /* 00000000b Counter Latch pour le compteur 0 */



/**

   Function: u8_t pit_setup(void)
   -----------------------------

   Programmable Interval Timer initialization.
   Configure clock mode and frequency.

**/

PUBLIC u8_t pit_setup(void)
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
      x86_outb(PIT_CWREG,PIT_MODE2);

      /* Send frequency for this mode */
      x86_outb(PIT_COUNTER0,(u8_t)ticks);        /* LSB first */
      x86_outb(PIT_COUNTER0,(u8_t)(ticks>>8));   /* MSB last */
    }
  
  return EXIT_SUCCESS;
}
