#ifndef I8259_H
#define I8259_H

/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "const.h"

/*========================================================================
 * Constantes
 *========================================================================*/

/* Ports des PICs */

#define PIC_MASTER_PORT     0x20
#define PIC_SLAVE_PORT      0xA0

/* ICWs */

#define PIC_ICW1            0x11
#define PIC_ICW2_MASTER     0x20
#define PIC_ICW2_SLAVE      0x28
#define PIC_ICW3_MASTER     0x04
#define PIC_ICW3_SLAVE      0x02
#define PIC_ICW4            0x01

/* Vecteurs */

#define PIC_VECTORS         16

/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC u8_t pic_init();
PUBLIC void pic_enable_irq(u8_t n);
PUBLIC void pic_disable_irq(u8_t n);

#endif
