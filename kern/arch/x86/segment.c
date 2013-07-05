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
   - const.h
   - segment.h    : self header

**/


#include <define.h>
#include <types.h>
#include "const.h"
#include "segment.h"


/**

   Privates
   --------

**/

PRIVATE void init_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PRIVATE void init_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PRIVATE void init_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags);
PRIVATE void init_tss_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PRIVATE void init_seg_desc(struct seg_desc *desc, lineaddr_t base, u32_t size);
PRIVATE void init_gate_desc(struct gate_desc* gate, u16_t seg, u32_t off);
PRIVATE u8_t gdt_init(void);

PRIVATE struct seg_desc gdt[VM_GDT_SIZE];  /* GDT */


/**

   Function: void init_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
   ------------------------------------------------------------------------------------------

   Create the code segment descriptor `desc` starting at `base` address with len `size` and acces 
   level set a `dpl`. Attributes are set to:

   - VM_SEG_PRESENT   : Segment is present in memory
   - VM_SEG_DATA_CODE : Segment is a code or data segment
   - VM_SEG_ER        : Access is Execute and Read

   Granularity is set to VM_SEG_D so we are in 32 bits world.

**/


PRIVATE void init_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  init_seg_desc(desc,base,size);

  /* D Flag (32bits) */
  desc->granularity |= VM_SEG_D;

  /* Attributes */
  desc->attributes = (dpl << VM_SEG_DPL_SHIFT) | VM_SEG_PRESENT | VM_SEG_DATA_CODE | VM_SEG_ER; 

  return;

}




/**

   Function: void init_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
   ------------------------------------------------------------------------------------------

   Create the code segment descriptor `desc` starting at `base` address with len `size` and acces 
   level set a `dpl`. Attributes are set to:

   - VM_SEG_PRESENT   : Segment is present in memory
   - VM_SEG_DATA_CODE : Segment is a code or data segment
   - VM_SEG_RW        : Access is  Read and Write

   Granularity is set to VM_SEG_B so segment limit is set to 0xFFFFFFFF.

**/


PRIVATE void init_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  init_seg_desc(desc,base,size);

  /* B Flag*/
  desc->granularity |= VM_SEG_B;

 /* Attributes */
  desc->attributes = (dpl << VM_SEG_DPL_SHIFT) | VM_SEG_PRESENT | VM_SEG_DATA_CODE | VM_SEG_RW;

  return;

}



/**

   Function: void init_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags)
   -------------------------------------------------------------------------------------

  
   Initialize interrupt gate descriptor `gate` with segment selector `seg` and  offset `off`.
   Interrupt gates have the following 8 bytes structure:


   31                                         16 15 14  13 12        8 7      5 4    0
   +-------------------------------------------+---+-----+------------+--------+-----+
   |                                           |   |  D  |            |        |/////|
   |        Offset 31:16                       | P |  P  | 0 D 1 1 0  | 0 0 0  |/////|  (4->7 byte)
   |                                           |   |  L  |            |        |/////|
   +-------------------------------------------+---+-----+------------+--------------+


   31                                        16 15                                   0
   +-------------------------------------------+-------------------------------------+
   |                                           |                                     |
   |           Segment Selector                |            Offset 15:0              |  (0->3 byte)
   |                                           |                                     |
   +-------------------------------------------+-------------------------------------+


   with fields:

   - DPL      : Descriptor Privilege Level
   - Offset   : ISR Offset in segment
   - P        : Present flag
   - Selector : ISR code Segment Selector
   - D        : Gate size (1=32bits, 0=16bits)

**/



PRIVATE void init_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags)
{
  u8_t magic = 14;  /* 14 = 00001110b */

  /* Generic initialization */
  init_gate_desc(gate,seg,off);

  /* Attributes (bits 32 to 48) */
  gate->attributes = magic | flags;

  return;
}





/**

   Function: void init_tss_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
   -----------------------------------------------------------------------------------------

   Initialize TSS segment descriptor `desc` starting at `base` address with len `size` and acces 
   level set a `dpl`. Attributes are set to:

   - VM_SEG_PRESENT   : Segment is present in memory
   - VM_SEG_TSS       : Segment is a TSS segment
  
**/


PRIVATE void init_tss_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  init_seg_desc(desc,base,size);

  /* Attributes */
  desc->attributes = (dpl << VM_SEG_DPL_SHIFT) | VM_SEG_PRESENT | VM_SEG_TSS;

  return;

}




/**

   Function: void init_seg_desc(struct seg_desc *desc, lineaddr_t base, u32_t size)
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
   

PRIVATE void init_seg_desc(struct seg_desc *desc, lineaddr_t base, u32_t size)
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




/**

   Function: void init_gate_desc(struct gate_desc* gate, u16_t seg, u32_t off)
   ---------------------------------------------------------------------------

   Create the gate decriptor `gate` with segment selector `seg` and  offset `off`.
   Gate descriptors structures are described in `init_int_gate` and `init_trap_gate`

**/


PRIVATE void init_gate_desc(struct gate_desc* gate, u16_t seg, u32_t off)
{

  /* Offset */
  gate->offset_low = off;
  gate->offset_high = off >> 16;

  /* Code segment */
  gate->segment = seg;

  /* Zero byte */
  gate->zero = 0;

  return;
}


/**

   Function: u8_t gdt_init(void)
   -----------------------------

   Global Descriptors Table  initialization.
   Set up a memory flat model by defining only 2 segmented spaces
   
   1- Kernel space : Ring 0 space from 0  to 4GB
   2- User space   : Ring 3 space from 0 to 4GB

   Each space has a code segment for programs code and a data segment for everything else.
   
   Kernel uses only one Task State Segment for task switching (just to save ESP0). That TSS is also
   defined in the GDT.

**/
 

PRIVATE u8_t gdt_init(void)
{
 
  /* GDT descriptor */
  gdt_desc.limit = sizeof(gdt) - 1;  
  gdt_desc.base = (lineaddr_t) gdt; /* Due to memory flat model set up by bootloader, in-used adresses are linear adresses */

  /* Kernel space segments */  
  init_code_seg(&gdt[VM_GDT_KERN_CS_INDEX],(lineaddr_t) VM_GDT_KERN_BASE, VM_GDT_KERN_LIMIT, CONST_RING0);
  init_data_seg(&gdt[VM_GDT_KERN_XS_INDEX],(lineaddr_t) VM_GDT_KERN_BASE, VM_GDT_KERN_LIMIT, CONST_RING0);

  /* User space segments */ 
  init_code_seg(&gdt[VM_GDT_USER_CS_INDEX],(lineaddr_t) VM_GDT_USER_BASE, VM_GDT_USER_LIMIT, CONST_RING3);
  init_data_seg(&gdt[VM_GDT_USER_XS_INDEX],(lineaddr_t) VM_GDT_USER_BASE, VM_GDT_USER_LIMIT, CONST_RING3);

  /* Global TSS */
  init_tss_seg(&gdt[VM_GDT_TSS_INDEX], (lineaddr_t)&tss, sizeof(tss), CONST_RING0);

  return EXIT_SUCCESS;
}


