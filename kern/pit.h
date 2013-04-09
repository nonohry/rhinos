/**

   pit.h
   =====

   PIT i82C54 header
   
**/


#ifndef PIT_H
#define PIT_H


/**
   
   Includes 
   --------

   - types.h
   - const.h
 
**/

#include <types.h>
#include "const.h"


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

   Prototypes 
   ----------

   Give acces to PIT intialization and reads/

**/

PUBLIC u8_t pit_init();
PUBLIC u16_t pit_read();

#endif
