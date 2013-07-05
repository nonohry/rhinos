/**

   seg.h
   =====

   Segment descriptors header
 
**/

#ifndef SEG_H
#define SEG_H


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


#define SEG_PRESENT     0x80 /* 10000000b = 0x80  */
#define SEG_DPL_0       0x00 /* 00000000b = 0x00  */
#define SEG_DPL_1       0x20 /* 00100000b = 0x40  */
#define SEG_DPL_2       0x40 /* 01000000b = 0x40  */
#define SEG_DPL_3       0x60 /* 01100000b = 0x60  */
#define SEG_DATA_CODE   0x10 /* 00010000b = 0x10  */

#define SEG_DPL_SHIFT   5

#define SEG_RO          0x00 
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

#define SEG_TSS         0x09
#define SEG_LDT         0x02


#define SEG_LIMIT       0x0F /* 00001111b = 0x0F */
#define SEG_AVL         0x10 /* 00001000b = 0x10 */
#define SEG_D           0x40 /* 01000000b = 0x40 */
#define SEG_B           0x40 /* 01000000b = 0x40 */
#define SEG_GRANULAR    0x80 /* 10000000b = 0x40 */

#define SEG_GRANULAR_LIMIT    0xFFFFFL


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

   Prototypes
   ----------

   Give access to all segment descriptor creation primitives

**/

PUBLIC void init_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PUBLIC void init_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PUBLIC void init_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags);
PUBLIC void init_trap_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags);
PUBLIC void init_tss_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PUBLIC void init_ldt_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);

#endif
