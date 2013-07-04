/**

   seg.c
   =====

   Segment descriptors initialization for segmentation

 **/



/**

   Includes
   --------

   - types.h
   - const.h
   - seg.h    : self header

**/

#include <define.h>
#include <types.h>
#include "const.h"
#include "seg.h"


/**

   Privates
   --------

   Common initialization code

**/

PRIVATE void init_seg_desc(struct seg_desc *desc, lineaddr_t base, u32_t size);
PRIVATE void init_gate_desc(struct gate_desc* gate, u16_t seg, u32_t off);




/**

   Function: void init_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
   ------------------------------------------------------------------------------------------

   Create the code segment descriptor `desc` starting at `base` address with len `size` and acces 
   level set a `dpl`. Attributes are set to:

   - SEG_PRESENT   : Segment is present in memory
   - SEG_DATA_CODE : Segment is a code or data segment
   - SEG_ER        : Access is Execute and Read

   Granularity is set to SEG_D so we are in 32 bits world.

**/

PUBLIC void init_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  init_seg_desc(desc,base,size);

  /* D Flag (32bits) */
  desc->granularity |= SEG_D;

  /* Attributes */
  desc->attributes = (dpl << SEG_DPL_SHIFT) | SEG_PRESENT | SEG_DATA_CODE | SEG_ER; 

  return;

}




/**

   Function: void init_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
   ------------------------------------------------------------------------------------------

   Create the code segment descriptor `desc` starting at `base` address with len `size` and acces 
   level set a `dpl`. Attributes are set to:

   - SEG_PRESENT   : Segment is present in memory
   - SEG_DATA_CODE : Segment is a code or data segment
   - SEG_RW        : Access is  Read and Write

   Granularity is set to SEG_B so segment limit is set to 0xFFFFFFFF.

**/

PUBLIC void init_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  init_seg_desc(desc,base,size);

  /* B Flag*/
  desc->granularity |= SEG_B;

 /* Attributes */
  desc->attributes = (dpl << SEG_DPL_SHIFT) | SEG_PRESENT | SEG_DATA_CODE | SEG_RW;

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


PUBLIC void init_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags)
{
  u8_t magic = 14;  /* 14 = 00001110b */

  /* Generic initialization */
  init_gate_desc(gate,seg,off);

  /* Attributes (bits 32 to 48) */
  gate->attributes = magic | flags;

  return;
}


/**

   Function: void init_trap_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags)
   --------------------------------------------------------------------------------------

   Initialize trap gate descriptor `gate` with segment selector `seg` and  offset `off`.
   Trap gates have the following 8 bytes structure:

   31                                         16 15 14  13 12        8 7      5 4    0
   +-------------------------------------------+---+-----+------------+--------+-----+
   |                                           |   |  D  |            |        |/////|
   |        Offset 31:16                       | P |  P  | 0 D 1 1 1  | 0 0 0  |/////|  (4->7 byte)
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

PUBLIC void init_trap_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags)
{
  u8_t magic = 15;  /* 15 = 00001111b */

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

   - SEG_PRESENT   : Segment is present in memory
   - SEG_TSS       : Segment is a TSS segment
  
**/

PUBLIC void init_tss_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  init_seg_desc(desc,base,size);

  /* Attributes */
  desc->attributes = (dpl << SEG_DPL_SHIFT) | SEG_PRESENT | SEG_TSS;

  return;

}


/**

   Function: void init_ldt_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
   -----------------------------------------------------------------------------------------

   Initialize LDT segment descriptor `desc` starting at `base` address with len `size` and acces 
   level set a `dpl`. Attributes are set to:
   
   - SEG_PRESENT   : Segment is present in memory
   - SEG_LDT       : Segment is a LDT segment
  
**/

PUBLIC void init_ldt_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl)
{

  /* Generic initialization */
  init_seg_desc(desc,base,size);

  /* Attributes */
  desc->attributes = (dpl << SEG_DPL_SHIFT) | SEG_PRESENT | SEG_LDT;

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
  if (size > SEG_GRANULAR_LIMIT)
    {
      desc->limit_low = size >> 12;                           /* Divide size by 4KB (ie shift by 12) */
      desc->granularity = SEG_GRANULAR | size >> (16 + 12)  ; /* upper limit */
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
