/*
 * Header de irq.c
 *
 */

#ifndef IRQ_H
#define IRQ_H

/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "const.h"
#include "interrupt.h"


/*========================================================================
 * Constantes
 *========================================================================*/

#define IRQ_VECTORS   16


/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC u8_t irq_init(void);
PUBLIC void irq_enable(u8_t irq);
PUBLIC void irq_disable(u8_t irq);
PUBLIC void irq_add_flih(u8_t irq, struct int_node* node);
PUBLIC void irq_remove_flih(u8_t irq, struct int_node* node);
PUBLIC void irq_handle_flih(u8_t irq, struct context_cpu* ctx);

#endif

