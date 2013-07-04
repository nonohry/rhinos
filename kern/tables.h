/**

   tables.h
   ========

   GDT & IDT initialization header
   
**/

#ifndef TABLES_H
#define TABLES_H


/**
 
   Includes 
   --------

   - types.h
   - const.h
   - seg.h    : struct [seg|gate]_desc needed

**/

#include <define.h>
#include <types.h>
#include "const.h"
#include "seg.h"


/**
 
   Constants: Indexes in GDT
   -------------------------

**/


#define TABLES_NULL_INDEX          0
#define TABLES_KERN_CS_INDEX       1
#define TABLES_KERN_XS_INDEX       2                  /* DS,ES,FS,SS */
#define TABLES_USER_CS_INDEX       3
#define TABLES_USER_XS_INDEX       4                  /* DS,ES,FS,SS */
#define TABLES_TSS_INDEX           5
#define TABLES_MAX_INDEX           TABLES_TSS_INDEX


/**

   Constants: IDT & GDT size
   -------------------------

**/

#define TABLES_GDT_SIZE            TABLES_MAX_INDEX+1
#define TABLES_IDT_SIZE            52


/**

   Constant: TABLES_SHIFT_SELECTOR
   -------------------------------

   Index << TABLES_SHIFT_SELECTOR = Selector

**/

#define TABLES_SHIFT_SELECTOR      3


/**

   Constants: Segments bases and limits
   -----------------------------------

**/

#define TABLES_KERN_BASE         0x0
#define TABLES_KERN_LIMIT        0x0
#define TABLES_USER_BASE         0x0
#define TABLES_USER_LIMIT        0x0


/**

   Structure: struct table_desc
   ----------------------------

   table (IDT or GDT) descriptor.
   Members are:
   
   - limit : table size
   - base  : table base address

**/

PUBLIC struct table_desc
{
  u16_t limit;
  lineaddr_t base;
} __attribute__ ((packed));


/**

   Structure: struct tss
   ---------------------

   Describe a TSS.
   Stick to Intel TSS description

**/

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


/**

   Globals: GDT, IDT & TSS
   -----------------------

**/

PUBLIC struct seg_desc gdt[TABLES_GDT_SIZE];  /* GDT */
PUBLIC struct gate_desc idt[TABLES_IDT_SIZE]; /* IDT */
PUBLIC struct tss tss;                        /* TSS */
PUBLIC struct table_desc gdt_desc;            /* GDT descriptor */
PUBLIC struct table_desc idt_desc;            /* IDT descriptor */


/**

   Prototypes
   ----------

   Give access to tables initialization

**/

PUBLIC  u8_t gdt_init(void);
PUBLIC  u8_t idt_init(void);

#endif
