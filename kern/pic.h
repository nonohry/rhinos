#ifndef I8259_H
#define I8259_H

/*************
 * Includes 
 *************/

#include <types.h>

/***************
 * Constantes
 ***************/

/* Ports des PICs */

#define IRQ_MASTER_PORT     0x20
#define IRQ_SLAVE_PORT      0xA0

/* ICWs */

#define IRQ_ICW1            0x11
#define IRQ_ICW2_MASTER     0x20
#define IRQ_ICW2_SLAVE      0x28
#define IRQ_ICW3_MASTER     0x04
#define IRQ_ICW3_SLAVE      0x02
#define IRQ_ICW4            0x01

/* Vecteurs */

#define PIC_VECTORS         16

/***************
 * Prototypes
 ***************/

PUBLIC void pic_init();
PUBLIC void pic_enable_irq(u8_t n);
PUBLIC void pic_disable_irq(u8_t n);

#endif
