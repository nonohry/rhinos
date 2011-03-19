/*
 * Header de irq.c
 *
 */

#ifndef IRQ_H
#define IRQ_H

/************
 * Includes
 ************/

#include <types.h>

/*************
 * Constantes
 *************/

#define IRQ_VECTORS   16

/***************
 * Definitions
 ***************/

/* alias pour le handler */
typedef void (*irq_flih_t)(void);


/* Structure irq_node */
struct irq_node
{
  void (*flih)(void);      /* First Level Interrupt Handler */
  struct irq_node* prev;   /* Noeud precedent */
  struct irq_node* next;   /* Noeud suivant */
};



/***************
 * Prototypes
 ***************/

PUBLIC void irq_init(void);
PUBLIC void irq_enable(u8_t irq);
PUBLIC void irq_disable(u8_t irq);
PUBLIC void irq_boot_add_flih(u8_t irq, irq_flih_t func);

#endif

