/**

   segment.c
   =========

   Virtual Memory Segmentation Setup


**/


/**

   Includes
   --------

   - define.h
   - types.h
   - x86_const.h
   - vm_segment.h    : self header

**/


#include <define.h>
#include <types.h>
#include "x86_const.h"
#include "vm_segment.h"


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

   Structure: struct gdt_desc
   --------------------------

   GDT descriptor.
   Members are:
   
   - limit : table size
   - base  : table base address

**/


PUBLIC struct gdt_desc
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

   Privates
   --------

**/

PRIVATE void create_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PRIVATE void create_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PRIVATE void create_tss_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PRIVATE void create_seg_desc(struct seg_desc *desc, lineaddr_t base, u32_t size);


/**

   Globals: IDT descriptors & TSS
   ------------------------------

**/

PUBLIC struct tss tss;                        /* TSS */
PUBLIC struct gdt_desc gdt_desc;            /* GDT descriptor */



/**

   Static: gdt
   -----------

   Global Descriptors Table

**/

struct seg_desc gdt[VM_GDT_SIZE];


/**

   Function: u8_t vm_segment_setup(void)
   -------------------------------------

   Global Descriptors Table  initialization.
   Set up a memory flat model by defining only 2 segmented spaces
   
   1- Kernel space : Ring 0 space from 0  to 4GB
   2- User space   : Ring 3 space from 0 to 4GB

   Each space has a code segment for programs code and a data segment for everything else.
   
   Kernel uses only one Task State Segment for task switching (just to save ESP0). That TSS is also
   defined in the GDT.

**/
 

PUBLIC u8_t vm_segment_setup(void)
{
 
  /* GDT descriptor */
  gdt_desc.limit = sizeof(gdt) - 1;  
  gdt_desc.base = (lineaddr_t) gdt; /* Due to memory flat model set up by bootloader, in-used adresses are linear adresses */

  /* Kernel space segments */  
  create_code_seg(&gdt[VM_GDT_KERN_CS_INDEX],(lineaddr_t) VM_GDT_KERN_BASE, VM_GDT_KERN_LIMIT, X86_CONST_RING0);
  create_data_seg(&gdt[VM_GDT_KERN_XS_INDEX],(lineaddr_t) VM_GDT_KERN_BASE, VM_GDT_KERN_LIMIT, X86_CONST_RING0);

  /* User space segments */ 
  create_code_seg(&gdt[VM_GDT_USER_CS_INDEX],(lineaddr_t) VM_GDT_USER_BASE, VM_GDT_USER_LIMIT, X86_CONST_RING3);
  create_data_seg(&gdt[VM_GDT_USER_XS_INDEX],(lineaddr_t) VM_GDT_USER_BASE, VM_GDT_USER_LIMIT, X86_CONST_RING3);

  /* Global TSS */
  create_tss_seg(&gdt[VM_GDT_TSS_INDEX], (lineaddr_t)&tss, sizeof(tss), X86_CONST_RING0);

  return EXIT_SUCCESS;
}




/**

   Function: void create_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
   ------------------------------------------------------------------------------------------

   Create the code segment descriptor `desc` starting at `base` address with len `size` and acces 
   level set a `dpl`. Attributes are set to:

   - VM_SEG_PRESENT   : Segment is present in memory
   - VM_SEG_DATA_CODE : Segment is a code or data segment
   - VM_SEG_ER        : Access is Execute and Read

   Granularity is set to VM_SEG_D so we are in 32 bits world.

**/


PRIVATE void create_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  create_seg_desc(desc,base,size);

  /* D Flag (32bits) */
  desc->granularity |= VM_SEG_D;

  /* Attributes */
  desc->attributes = (dpl << VM_SEG_DPL_SHIFT) | VM_SEG_PRESENT | VM_SEG_DATA_CODE | VM_SEG_ER; 

  return;

}




/**

   Function: void create_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
   ------------------------------------------------------------------------------------------

   Create the code segment descriptor `desc` starting at `base` address with len `size` and acces 
   level set a `dpl`. Attributes are set to:

   - VM_SEG_PRESENT   : Segment is present in memory
   - VM_SEG_DATA_CODE : Segment is a code or data segment
   - VM_SEG_RW        : Access is  Read and Write

   Granularity is set to VM_SEG_B so segment limit is set to 0xFFFFFFFF.

**/


PRIVATE void create_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  create_seg_desc(desc,base,size);

  /* B Flag*/
  desc->granularity |= VM_SEG_B;

 /* Attributes */
  desc->attributes = (dpl << VM_SEG_DPL_SHIFT) | VM_SEG_PRESENT | VM_SEG_DATA_CODE | VM_SEG_RW;

  return;

}




/**

   Function: void create_tss_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
   -----------------------------------------------------------------------------------------

   Initialize TSS segment descriptor `desc` starting at `base` address with len `size` and acces 
   level set a `dpl`. Attributes are set to:

   - VM_SEG_PRESENT   : Segment is present in memory
   - VM_SEG_TSS       : Segment is a TSS segment
  
**/


PRIVATE void create_tss_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  create_seg_desc(desc,base,size);

  /* Attributes */
  desc->attributes = (dpl << VM_SEG_DPL_SHIFT) | VM_SEG_PRESENT | VM_SEG_TSS;

  return;

}




/**

   Function: void create_seg_desc(struct seg_desc *desc, lineaddr_t base, u32_t size)
   --------------------------------------------------------------------------------

   Initialize segment descriptor `desc` starting at `base` address with len `size`.
   Segment descriptor is a 8 bytes structures:

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


   - Segment Limit : Segment size, depend on G bit value: 
                       - Bit G not set, size goes frm 1byte to 1Mbyte, 1 byte increment
		       - Bit G set, size goes from 4Kbyte to 4Gbytes, 4Kbytes increment
   - Base Address  : First byte location in the segment.
   - Type          : Segment type (code, data, ...) and access rights (Read, Write, ...)
   - S Flag        : System (S=0) or Code/Data (S=1) Segment
   - DPL           : Descriptor Privilege Level. Goes from 0 (highest) to 3.
   - P Flag        : Segment present in memory or not
   - D/B Flag      : Depend on segment type:
                        - Executable Code Segment : D flag, adress size (D=1 for 32 bits)
			- Stack segment           : B flag, stack pointer size (B=1 for 32 bits)
			- Data segment            : B flag, segment limit (B=1 for 0xFFFFFFFF)
   - G Flag        : Granularity, id est Segment Limit scale. If G is set, limit is count as 4Kbytes unity.
   - AVL           : Available bit

**/
   

PRIVATE void create_seg_desc(struct seg_desc *desc, lineaddr_t base, u32_t size)
{

  /* Base address */
  desc->base_low = base;
  desc->base_middle = base >> 16;
  desc->base_high = base >> 24;

  /* Set granularity if needed */
  size--;                             /* -1 is 4Gbytes */
  if (size > VM_SEG_GRANULAR_LIMIT)
    {
      desc->limit_low = size >> 12;                           /* Divide size by 4KB (ie shift by 12) */
      desc->granularity = VM_SEG_GRANULAR | size >> (16 + 12)  ; /* upper limit */
    }
  else
    {
      desc->limit_low = size;          /* Limit bits 15:00  */
      desc->granularity = size >> 16;  /* Limit bits 19:16  */
    }

  
  return;
}




