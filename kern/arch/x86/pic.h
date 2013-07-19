/**

   pic.h
   =====

   PIC i8259A header
   
**/

#ifndef PIC_H
#define PIC_H


/**
   
   Includes 
   --------

   - define.h
   - types.h
 
**/

#include <define.h>
#include <types.h>


/**

   Prototypes
   ----------

   Give access to PIC initialization as well as IRQ enabling/disabling primitives

**/ 

PUBLIC u8_t pic_setup();
PUBLIC u8_t pic_enable_irq(u8_t n);
PUBLIC u8_t pic_disable_irq(u8_t n);

#endif
