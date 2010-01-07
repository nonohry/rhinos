#ifndef PROT_H
#define PROT_H

/*************
 * Includes 
 *************/

#include "types.h"

/**************
 * Constantes
 **************/


/* Indexes de la GDT */

#define NULL_INDEX     0
#define CS_INDEX       1
#define DS_INDEX       2
#define ES_INDEX       3
#define SS_INDEX       4
#define LDT_INDEX      5     /* Index de la premiere LDT */

/* Taille de la GDT */

#define GDT_SIZE       LDT_INDEX

/* Selecteurs de segment */

#define CS_SELECTOR	8    /* CS = 00000001  0  00   = (byte) 8  */
#define	DS_SELECTOR     16   /* DS = 00000010  0  00   = (byte) 16 */
#define	ES_SELECTOR	24   /* ES = 00000011  0  00   = (byte) 24 */
#define	SS_SELECTOR	32   /* SS = 00000100  0  00   = (byte) 32 */


/* Masques pour le champ attributes de seg_desc */

#define SEG_PRESENT     0x80 /* 10000000b = 0x80 - Segment Present en memoire */
#define SEG_DPL_0       0x00 /* 00000000b = 0x00 - Niveau de Privilege 0 */
#define SEG_DPL_1       0x20 /* 00100000b = 0x40 - Niveau de Privilege 1 */
#define SEG_DPL_2       0x40 /* 01000000b = 0x40 - Niveau de Privilege 2 */
#define SEG_DPL_3       0x60 /* 01100000b = 0x60 - Niveau de Privilege 3 */
#define SEG_DATA_CODE   0x10 /* 00010000b = 0x10 - Segment de code ou de donnees */

#define SEG_RO          0x00 /* Type - cf Doc Intel */
#define SEG_RO_ACC      0x01 
#define SEG_RW          0x02
#define SEG_RW_ACC      0x03
#define SEG_RO_ED       0x04
#define SEG_RO_ED_ACC   0x05
#define SEG_RW_ED       0x06
#define SEG_RW_ED_ACC   0x07

#define SEG_EO          0x08
#define SEG_EO_ACC      0x09 
#define SEG_ER          0x0A
#define SEG_ER_ACC      0x0B
#define SEG_EO_ED       0x0C
#define SEG_EO_ED_ACC   0x0D
#define SEG_ER_ED       0x0E
#define SEG_ER_ED_ACC   0x0F


/* Masques pour le champs granularity de seg_desc */

#define SEG_LIMIT       0x0F /* 00001111b = 0x0F */
#define SEG_AVL         0x10 /* 00001000b = 0x10 */
#define SEG_D           0x40 /* 01000000b = 0x40 */
#define SEG_B           0x40 /* 01000000b = 0x40 */
#define SEG_GRANULAR    0x80 /* 10000000b = 0x40 */

/* Limite des segments  */

#define GRANULAR_LIMIT  0xFFFFFL   /* Pas de granularite au dessous (L pour Long) */
#define KERN_LIMIT      0xC0000       /* Limit de l'espace Noyau */

/* IRQs */

#define IRQ_VECTORS     16

/*****************************
 * Structures (cf Doc Intel)
 *****************************/


/* Descripteur de Segment */

struct seg_desc 
{
  u16_t limit_low;
  u16_t base_low;
  u8_t base_middle;
  u8_t attributes;     /* |P|DPL|S| Type |            */
  u8_t granularity;    /* |G|D/B|0|AVL|Seg Lim 19:16| */
  u8_t base_high;
} __attribute__ ((packed));


/* Descripteur de Table (GDT & LDT) */

struct table_desc
{
  u16_t limit;
  u32_t base;
} __attribute__ ((packed));


/* Liste chainee des handlers d'IRQs */

struct irq_chained
{
  struct irq_chained* next;
  u8_t (*handler)();
  u32_t id;
} __attribute__ ((packed));


/**************
 * Prototypes 
 **************/

PUBLIC struct seg_desc gdt[GDT_SIZE]; /* GDT */
PUBLIC struct table_desc gdt_desc;    /* Descripteur de la GDT */
PUBLIC struct table_desc idt_desc;    /* Descripteur de l'IDT */
PUBLIC struct irq_chained* irq_handlers[IRQ_VECTORS];  /* Tableau des irq handlers */
PUBLIC u32_t  irq_active[IRQ_VECTORS];                 /* Tableau des bitmaps d'irq actives */


PUBLIC void pmode_init();
PUBLIC void init_code_seg(struct seg_desc *desc, u32_t base, u32_t size, u32_t dpl);
PUBLIC void init_data_seg(struct seg_desc *desc, u32_t base, u32_t size, u32_t dpl);

#endif
