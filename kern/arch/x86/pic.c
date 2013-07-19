/**
 
   pic.c
   =====

   PIC i8259A management
 
**/


/**
   
   Includes 
   --------

   - define.h
   - types.h
   - const.h
   - klib.h
   - pic.h    : self header

**/

#include <define.h>
#include <types.h>
#include "x86_lib.h"
#include "pic.h"




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

   Function: u8_t pic_init(void)
   -----------------------------

   PIC i8259 initialization. 
   Send initialization control words (ICW) to master and slave parts
   
   1- ICW1 = 0x11 = 10001b         : ICW4 needed, cascade mode (master & slave)
                                     interval of 8, edge triggered mode (pulse mode)

   2- ICW2 Master = 0x20 = 100000b : Hardware interrupts (master part) offset in IDT. Set to 32 because
                                     Intel require first 32 entries in IDT are exceptions

   3- ICW2 Slave = 0x28 = 101000b  : Hardware interrupts (slave part) offset in IDT. Set to 40 
                                     (32 exceptions + 8 master hardware interrupts)

   4- ICW3 Master = 0x04 = 100b    : Set the IRQ line connected to the slave (line 2 here)

   5- ICW3 Slave = 0x02 = 10b      : Set the IRQ line connected to the master (line 2 here)
   
   6- ICW4 = 0x01 = 1b             : intel 8086 mode

**/

PUBLIC u8_t pic_setup(void)
{

  /* Send ICW1 to both master and slave */
  x86_outb(PIC_MASTER_PORT,PIC_ICW1);
  x86_outb(PIC_SLAVE_PORT,PIC_ICW1);

  /* Send an ICW2 to master and another to slave */
  x86_outb(PIC_MASTER_PORT+1,PIC_ICW2_MASTER);
  x86_outb(PIC_SLAVE_PORT+1,PIC_ICW2_SLAVE);

  /* Send an ICW3 to master and another to slave */
  x86_outb(PIC_MASTER_PORT+1,PIC_ICW3_MASTER);
  x86_outb(PIC_SLAVE_PORT+1,PIC_ICW3_SLAVE);

  /* Send ICW4 to both master and slave */
  x86_outb(PIC_MASTER_PORT+1,PIC_ICW4);
  x86_outb(PIC_SLAVE_PORT+1,PIC_ICW4);

  /* Inhibate interrupts because there is no 
     interrupt manager yet. Only the line 2 (cascade mode)
     is activated */
  x86_outb(PIC_MASTER_PORT+1,0xFB);
  x86_outb(PIC_SLAVE_PORT+1,0xFF);

  return EXIT_SUCCESS;
}


/**
   
   Function: u8_t pic_enable_irq(u8_t n)
   -------------------------------------

   Enable n th IRQ line

**/

PUBLIC u8_t pic_enable_irq(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < PIC_VECTORS)
    {
      u8_t read;
      u16_t port;

      /* Slave or Master ? */
      port = (n<8)?PIC_MASTER_PORT+1:PIC_SLAVE_PORT+1;

      /* Activate  line */
      x86_inb(port,&read);
      read &= ~(1<<n);
      x86_outb(port,read);

      return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;
}


/**

   Function: u8_t pic_disable_irq(u8_t n)
   --------------------------------------

   Disable th n th IRQ line

**/

PUBLIC u8_t pic_disable_irq(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < PIC_VECTORS)
    {
      u8_t read;
      u16_t port;

      /* Slave or Master ? */
      port = (n<8)?PIC_MASTER_PORT+1:PIC_SLAVE_PORT+1;

      /* Disable line */
      x86_inb(port,&read);
      read |= (1<<n);
      x86_outb(port,read);

      return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;
}
