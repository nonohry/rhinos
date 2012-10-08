#ifndef PROT_H
#define PROT_H

/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "const.h"
#include "seg.h"
#include "irq.h"

/*========================================================================
 * Constantes
 *========================================================================*/


/* Indexes de la GDT */

#define TABLES_NULL_INDEX          0
#define TABLES_KERN_CS_INDEX       1
#define TABLES_KERN_XS_INDEX       2                  /* DS,ES,FS,SS */
#define TABLES_USER_CS_INDEX       3
#define TABLES_USER_XS_INDEX       4                  /* DS,ES,FS,SS */
#define TABLES_TSS_INDEX           5
#define TABLES_MAX_INDEX           TABLES_TSS_INDEX   /* Nombre maximum d index d une GDT */

/* Taille de la GDT & IDT */

#define TABLES_GDT_SIZE            TABLES_MAX_INDEX+1  /* Debute a 0 */
#define TABLES_IDT_SIZE            52

#define TABLES_SHIFT_SELECTOR      3    /* INDEX << SHIFT_SELECTOR = SELECTOR */


/* Limite des segments  */

#define TABLES_KERN_BASE         0x0        /* Adresse de base du noyau */
#define TABLES_KERN_LIMIT_4G     0x0        /* Limite de l'espace Noyau (4G) */
#define TABLES_USER_BASE         0x0        /* Adresse de base de l espace utilisateur */
#define TABLES_USER_LIMIT_4G     0x0        /* Limite de l'espace utilisateur (4G) */

/*========================================================================
 * Structures
 *========================================================================*/


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
PUBLIC struct tss tss; /* TSS */
PUBLIC struct table_desc gdt_desc;     /* Descripteur de la GDT */
PUBLIC struct table_desc idt_desc;     /* Descripteur de l'IDT */

PUBLIC  u8_t gdt_init();
PUBLIC  u8_t idt_init();

#endif
