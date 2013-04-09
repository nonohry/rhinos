/**

   pic.h
   =====

   PIC i8259A header
   
**/

#ifndef I8259_H
#define I8259_H


/**
   
   Includes 
   --------

   - types.h
   - const.h
 
**/

#include <types.h>
#include "const.h"


/**

   Constants: PIC initilization ports (master and slave)
   -----------------------------------------------------

   Where to send initialization commands to configure the PIC

**/

#define PIC_MASTER_PORT     0x20
#define PIC_SLAVE_PORT      0xA0


/**

   Constant: PIC_ICW1
   ------------------

   First Initialization Control Word. Its structure is:

   +---+---+---+---+------+-----+------+-----+
   | 0 | 0 | 0 | 1 | LTIM | ADI | SNGL | IC4 |
   +---+---+---+---+------+-----+------+-----+

   with:

   - LTIM: LTIM=0 signifie Edge Triggered, LTIM=1 pour Level Triggered
   - ADI: Adress Interval, no effect on 8086
   - SNGL: Only one PIC (SNGL=1) or in cascade (SNGL=0)
   - IC4: ICW4 must be read (IC4=1) or not (IC4=0)

**/

#define PIC_ICW1            0x11


/**

   Constant: PIC_ICW2
   ------------------

   Second Initialization Control Word. Its structure is:

   +----+----+----+----+----+---+---+---+
   | b5 | b4 | b3 | b2 | b1 | 0 | 0 | 0 |
   +----+----+----+----+----+---+---+---+ 
   
   Offset in IDT

**/   

#define PIC_ICW2_MASTER     0x20
#define PIC_ICW2_SLAVE      0x28


/**

   Constant: PIC_ICW3
   ------------------

   Third Initialization Control Word. It has 2 different structures
   wether it addresses the master or the slave.

   ### Master structure:

   +----+----+----+----+----+----+----+----+
   | S7 | S6 | S5 | S4 | S3 | S2 | S1 | S0 |
   +----+----+----+----+----+----+----+----+

   S(i) is set to 1 if the line is connected to a slave

   
   ### Slace Structure:

   
   +---+---+---+------+-----+-----+------+-----+
   | 0 | 0 | 0 | SFNM | BUF | M/S | AEOI | uPM |
   +---+---+---+------+-----+-----+------+-----+ 

   - SFNM : Fully Nested mode if set
   - BUF  : Buffer mod if set
   - M/S  : Slave buffer mode if not set and BUF set. Master buffer mode if set and BUF set
   - AEOI:  Automatic end of interrupt. If not set, the OS must aknowledge the end of interrupt by sending word 0x20
            to master intialization port and to slave intialization port if IRQ come from slave part.
   - PM   : 8086 mode if set, MCS-80/85 mode otherwise

**/

#define PIC_ICW3_MASTER     0x04
#define PIC_ICW3_SLAVE      0x02


/**

   Constant: PIC_ICW4
   ------------------

   Fourth  Initialization Control Word. Its structure is:

   +----+----+----+----+----+----+----+----+
   | M7 | M6 | M5 | M4 | M3 | M2 | M1 | M0 |
   +----+----+----+----+----+----+----+----+ 

   Inhibate IRQ line i if M(i) is set

**/

#define PIC_ICW4            0x01


/**

   Constant: PIC_VECTORS
   ---------------------

   IRQ number

**/

#define PIC_VECTORS         16


/**

   Prototypes
   ----------

   Give access to PIC initialization as well as IRQ enabling/disabling primitives

**/ 

PUBLIC u8_t pic_init();
PUBLIC void pic_enable_irq(u8_t n);
PUBLIC void pic_disable_irq(u8_t n);

#endif
