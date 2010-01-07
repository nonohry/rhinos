#ifndef I8259_H
#define I8259_H

/*************
 * Includes 
 *************/

#include "types.h"


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


/***************
 * Prototypes
 ***************/

PUBLIC void i8259_init();
PUBLIC void irq_enable(u8_t n);
PUBLIC void irq_disable(u8_t n);
#endif
