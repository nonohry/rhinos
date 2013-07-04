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
#include "const.h"
#include "klib.h"
#include "pic.h"


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

PUBLIC u8_t pic_init(void)
{

  /* Send ICW1 to both master and slave */
  klib_outb(PIC_MASTER_PORT,PIC_ICW1);
  klib_outb(PIC_SLAVE_PORT,PIC_ICW1);

  /* Send an ICW2 to master and another to slave */
  klib_outb(PIC_MASTER_PORT+1,PIC_ICW2_MASTER);
  klib_outb(PIC_SLAVE_PORT+1,PIC_ICW2_SLAVE);

  /* Send an ICW3 to master and another to slave */
  klib_outb(PIC_MASTER_PORT+1,PIC_ICW3_MASTER);
  klib_outb(PIC_SLAVE_PORT+1,PIC_ICW3_SLAVE);

  /* Send ICW4 to both master and slave */
  klib_outb(PIC_MASTER_PORT+1,PIC_ICW4);
  klib_outb(PIC_SLAVE_PORT+1,PIC_ICW4);

  /* Inhibate interrupts because there is no 
     interrupt manager yet. Only the line 2 (cascade mode)
     is activated */
  klib_outb(PIC_MASTER_PORT+1,0xFB);
  klib_outb(PIC_SLAVE_PORT+1,0xFF);

  return EXIT_SUCCESS;
}


/**
   
   Function: void pic_enable_irq(u8_t n)
   -------------------------------------

   Enable n th IRQ line

**/

PUBLIC void pic_enable_irq(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < PIC_VECTORS)
    {
      u8_t read;
      u16_t port;

      /* Slave or Master ? */
      port = (n<8)?PIC_MASTER_PORT+1:PIC_SLAVE_PORT+1;

      /* Activate  line */
      klib_inb(port,&read);
      read &= ~(1<<n);
      klib_outb(port,read);
    }

  return;
}


/**

   Function: void pic_disable_irq(u8_t n)
   --------------------------------------

   Disable th n th IRQ line

**/

PUBLIC void pic_disable_irq(u8_t n)
{
  /* IRQ : 0 -> 15 */
  if (n < PIC_VECTORS)
    {
      u8_t read;
      u16_t port;

      /* Slave or Master ? */
      port = (n<8)?PIC_MASTER_PORT+1:PIC_SLAVE_PORT+1;

      /* Disable line */
      klib_inb(port,&read);
      read |= (1<<n);
      klib_outb(port,read);
    }

  return;
}
