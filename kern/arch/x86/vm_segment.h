/**

   vm_segment.h
   =============

   Virtual Memory Segmentation Setup Header 

**/


#ifndef VM_SEGMENT_H
#define VM_SEGMENT_H


/**

   Includes
   --------

   - define.h
   - types.h
 
**/

#include <define.h>
#include <types.h>



/**

   Constants: Segment descriptor attributes 
   ----------------------------------------

**/


#define VM_SEG_PRESENT     0x80 /* 10000000b = 0x80  */
#define VM_SEG_DPL_0       0x00 /* 00000000b = 0x00  */
#define VM_SEG_DPL_1       0x20 /* 00100000b = 0x40  */
#define VM_SEG_DPL_2       0x40 /* 01000000b = 0x40  */
#define VM_SEG_DPL_3       0x60 /* 01100000b = 0x60  */
#define VM_SEG_DATA_CODE   0x10 /* 00010000b = 0x10  */

#define VM_SEG_DPL_SHIFT   5

#define VM_SEG_RO          0x00 
#define VM_SEG_RO_ACC      0x01 
#define VM_SEG_RW          0x02
#define VM_SEG_RW_ACC      0x03
#define VM_SEG_RO_ED       0x04
#define VM_SEG_RO_ED_ACC   0x05
#define VM_SEG_RW_ED       0x06
#define VM_SEG_RW_ED_ACC   0x07

#define VM_SEG_EO          0x08
#define VM_SEG_EO_ACC      0x09 
#define VM_SEG_ER          0x0A
#define VM_SEG_ER_ACC      0x0B
#define VM_SEG_EO_ED       0x0C
#define VM_SEG_EO_ED_ACC   0x0D
#define VM_SEG_ER_ED       0x0E
#define VM_SEG_ER_ED_ACC   0x0F

#define VM_SEG_TSS         0x09
#define VM_SEG_LDT         0x02

#define VM_SEG_LIMIT       0x0F /* 00001111b = 0x0F */
#define VM_SEG_AVL         0x10 /* 00001000b = 0x10 */
#define VM_SEG_D           0x40 /* 01000000b = 0x40 */
#define VM_SEG_B           0x40 /* 01000000b = 0x40 */
#define VM_SEG_GRANULAR    0x80 /* 10000000b = 0x40 */

#define VM_SEG_GRANULAR_LIMIT    0xFFFFFL


/**
 
   Constants: Indexes in GDT
   -------------------------

**/


#define VM_GDT_NULL_INDEX          0
#define VM_GDT_KERN_CS_INDEX       1
#define VM_GDT_KERN_XS_INDEX       2                  /* DS,ES,FS,SS */
#define VM_GDT_USER_CS_INDEX       3
#define VM_GDT_USER_XS_INDEX       4                  /* DS,ES,FS,SS */
#define VM_GDT_TSS_INDEX           5
#define VM_GDT_MAX_INDEX           VM_GDT_TSS_INDEX


/**

   Constants: IDT & GDT size
   -------------------------

**/

#define VM_GDT_SIZE            VM_GDT_MAX_INDEX+1
#define VM_IDT_SIZE            52


/**

   Constant: VM_SHIFT_SELECTOR
   -------------------------------

   Index << VM_SHIFT_SELECTOR = Selector

**/

#define VM_SHIFT_SELECTOR      3


/**

   Constants: Segments bases and limits
   -----------------------------------

**/

#define VM_GDT_KERN_BASE         0x0
#define VM_GDT_KERN_LIMIT        0x0
#define VM_GDT_USER_BASE         0x0
#define VM_GDT_USER_LIMIT        0x0



/**

   Structure: struct seg_desc
   --------------------------

   Describe a segment decriptor.
   Members definition follows Intel definition:

      31                24 23  22  21  20 19     16 15 14  13 12 11     8 7             0
   +------------------+---+---+---+---+--------+---+-----+---+--------+--------------+
   |                  |   | D |   | A | Seg    |   |  D  |   |        |              |
   |   Base 31:24     | G | / | 0 | V | Limit  | P |  P  | S |  Type  | Base 23:16   |  (4->7 bytes)
   |                  |   | B |   | L | 19:16  |   |  L  |   |        |              |
   +------------------+---+---+---+---+--------+---+-----+---+--------+--------------+


   31                                        16 15                                   0
   +-------------------------------------------+-------------------------------------+
   |                                           |                                     |
   |           Base Address 15:0               |       Segment Limit 15:0            |  (0->3 bytes)
   |                                           |                                     |
   +-------------------------------------------+-------------------------------------+

**/

PUBLIC struct seg_desc 
{
  u16_t limit_low;
  u16_t base_low;
  u8_t base_middle;
  u8_t attributes;     /* |P|DPL|S| Type |              */
  u8_t granularity;    /* |G|D/B|0|AVL|Seg Limit 19:16| */
  u8_t base_high;
} __attribute__ ((packed));


/**

   Structure: struct gate_desc
   ---------------------------

   Describe a gate decriptor.
   Members definition follows Intel definition:

   31                                         16 15 14  13 12        8 7      5 4    0
   +-------------------------------------------+---+-----+------------+--------+-----+
   |                                           |   |  D  |            |        |/////|
   |        Offset 31:16                       | P |  P  | 0 D 1 1 ?  | 0 0 0  |/////|  (4->7 byte)
   |                                           |   |  L  |            |        |/////|
   +-------------------------------------------+---+-----+------------+--------------+


   31                                        16 15                                   0
   +-------------------------------------------+-------------------------------------+
   |                                           |                                     |
   |           Segment Selector                |            Offset 15:0              |  (0->3 byte)
   |                                           |                                     |
   +-------------------------------------------+-------------------------------------+

**/


PUBLIC struct gate_desc
{
  u16_t offset_low;
  u16_t segment;
  u8_t zero;
  u8_t attributes;   /* |P|DPL|0111?| */
  u16_t offset_high;
} __attribute__ ((packed));



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

   Globals: GDT, IDT descriptors & TSS
   -----------------------------------

**/

PUBLIC struct tss tss;                        /* TSS */
PUBLIC struct table_desc gdt_desc;            /* GDT descriptor */


/** 

    Prototypes
    ----------

    Give access to segmentation setup

**/

PUBLIC u8_t vm_segment_setup(void);

#endif
