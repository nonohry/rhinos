#ifndef PROT_H
#define PROT_H

/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "seg.h"
#include "irq.h"

/*========================================================================
 * Constantes
 *========================================================================*/


/* Indexes de la GDT */

#define TABLES_NULL_INDEX     0
#define TABLES_CS_INDEX       1
#define TABLES_XS_INDEX       2                  /* DS,ES,FS,SS */
#define TABLES_TSS_INDEX      3
#define TABLES_LDT_INDEX      4                  /* Index de la premiere LDT */
#define TABLES_MAX_INDEX      TABLES_LDT_INDEX   /* Nombre maximum d index d une GDT */

/* Taille de la GDT & IDT */

#define TABLES_GDT_SIZE       TABLES_MAX_INDEX+1  /* Debute a 0 */
#define TABLES_IDT_SIZE       50

/* Selecteurs de segment */

#define TABLES_CS_SELECTOR	8    /*  CS = 0000000000001  0  00   =  8  */
#define	TABLES_DS_SELECTOR      16   /*  DS = 0000000000010  0  00   =  16 */
#define	TABLES_ES_SELECTOR	16   /*  ES = 0000000000010  0  00   =  16 */
#define	TABLES_SS_SELECTOR	16   /*  SS = 0000000000010  0  00   =  16 */
#define TABLES_TSS_SELECTOR     24   /* TSS = 0000000000011  0  00   =  24 */

#define TABLES_SHIFT_SELECTOR  3    /* INDEX << SHIFT_SELECTOR = SELECTOR */

/* Rings */

#define TABLES_RING0   0
#define TABLES_RING1   1
#define TABLES_RING2   2
#define TABLES_RING3   3

/* Limite des segments  */

#define TABLES_KERN_BASE         0x0        /* Adresse de base du noyau */
#define TABLES_KERN_LIMIT_4G     0x0        /* Limite de l'espace Noyau (4G) */
#define TABLES_KERN_TOP_STACK    0x7C00     /* ESP Noyau au boot */


/* Descripteur de Table (GDT & LDT) */

PUBLIC struct table_desc
{
  u16_t limit;
  lineaddr_t base;
} __attribute__ ((packed));

/* TSS */

PUBLIC struct tss
{
  u16_t previous;
  u16_t zero_0;
  u32_t esp0;
  u16_t ss0;
  u16_t zero_1;
  u32_t esp1;
  u16_t ss1;
  u16_t zero_2;
  u32_t esp2;
  u16_t ss2;
  u16_t zero_3;
  u32_t cr3;
  u32_t eip;
  u32_t eflags;
  u32_t eax;
  u32_t ecx;
  u32_t edx;
  u32_t ebx;
  u32_t esp;
  u32_t ebp;
  u32_t esi;
  u32_t edi;
  u16_t es;
  u16_t zero_4;
  u16_t cs;
  u16_t zero_5;
  u16_t ss;
  u16_t zero_6;
  u16_t ds;
  u16_t zero_7;
  u16_t fs;
  u16_t zero_8;
  u16_t gs;
  u16_t zero_9;
  u16_t ldt;
  u16_t zero_10;
  u16_t debug;
  u16_t iomap;
} __attribute__ ((packed));


/*========================================================================
 * Prototypes 
 *========================================================================*/

PUBLIC struct seg_desc gdt[TABLES_GDT_SIZE];  /* GDT */
PUBLIC struct gate_desc idt[TABLES_IDT_SIZE]; /* IDT */
PUBLIC struct table_desc gdt_desc;     /* Descripteur de la GDT */
PUBLIC struct table_desc idt_desc;     /* Descripteur de l'IDT */

PUBLIC void gdt_init();
PUBLIC void idt_init();

#endif
