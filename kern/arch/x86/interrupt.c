/**

   interrupt.c
   ===========

   Interrupt management

**/



/**

   Includes
   --------

   - define.h
   - types.h
   - x86_const.h
   - interrupt.h   : self header

**/

#include <define.h>
#include <types.h>
#include "x86_const.h"
#include "interrupt.h"




/**

   Constants: Segment descriptor attributes 
   ----------------------------------------

**/


#define INT_SEG_PRESENT     0x80 /* 10000000b = 0x80  */
#define INT_SEG_DPL_0       0x00 /* 00000000b = 0x00  */
#define INT_SEG_DPL_1       0x20 /* 00100000b = 0x40  */
#define INT_SEG_DPL_2       0x40 /* 01000000b = 0x40  */
#define INT_SEG_DPL_3       0x60 /* 01100000b = 0x60  */


/**

   Constant: INT_IDT_SIZE
   ----------------------

   IDT size

**/

#define INT_IDT_SIZE            52


/**

   Externs
   -------

   Assembly interrupt service routines

**/

EXTERN void hwint_00(void);
EXTERN void hwint_01(void);
EXTERN void hwint_02(void);
EXTERN void hwint_03(void);
EXTERN void hwint_04(void);
EXTERN void hwint_05(void);
EXTERN void hwint_06(void);
EXTERN void hwint_07(void);
EXTERN void hwint_08(void);
EXTERN void hwint_09(void);
EXTERN void hwint_10(void);
EXTERN void hwint_11(void);
EXTERN void hwint_12(void);
EXTERN void hwint_13(void);
EXTERN void hwint_14(void);
EXTERN void hwint_15(void);

EXTERN void swint_syscall(void);

EXTERN void excep_00(void);
EXTERN void excep_01(void);
EXTERN void excep_02(void);
EXTERN void excep_03(void);
EXTERN void excep_04(void);
EXTERN void excep_05(void);
EXTERN void excep_06(void);
EXTERN void excep_07(void);
EXTERN void excep_08(void);
EXTERN void excep_09(void);
EXTERN void excep_10(void);
EXTERN void excep_11(void);
EXTERN void excep_12(void);
EXTERN void excep_13(void);
EXTERN void excep_14(void);
EXTERN void excep_16(void);
EXTERN void excep_17(void);
EXTERN void excep_18(void);



/**

   Structure: struct idt_desc
   --------------------------

   GDT descriptor.
   Members are:
   
   - limit : table size
   - base  : table base address

**/


PUBLIC struct idt_desc
{
  u16_t limit;
  lineaddr_t base;
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

   Global: idt_desc
   ----------------

   idt descriptor

**/

PUBLIC struct idt_desc idt_desc;


/**

   Static: idt
   -----------

   Interrupt Descriptors Table

**/

struct gate_desc idt[INT_IDT_SIZE];



/**

   Privates
   --------

**/

PRIVATE void create_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags);
PRIVATE void create_gate_desc(struct gate_desc* gate, u16_t seg, u32_t off);


/**

   Function: u8_t interrupt_setup(void)
   ------------------------------------

   Initiliaze interrupt system

   Create IDT

**/


PUBLIC u8_t interrupt_setup(void)
{

/* IDT Descriptor */
  idt_desc.limit = sizeof(idt) - 1;
  idt_desc.base = (lineaddr_t) idt;

  /* Exception handler */
  create_int_gate(&idt[0] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_00, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[1] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_01, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[2] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_02, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[3] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_03, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[4] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_04, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[5] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_05, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[6] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_06, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[7] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_07, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[8] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_08, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[9] , X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_09, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[10], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_10, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[11], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_11, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[12], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_12, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[13], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_13, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[14], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_14, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[16], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_16, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[17], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_17, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[18], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_18, INT_SEG_PRESENT | INT_SEG_DPL_0);

  /* ISR */
  create_int_gate(&idt[32], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_00, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[33], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_01, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[34], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_02, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[35], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_03, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[36], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_04, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[37], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_05, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[38], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_06, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[39], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_07, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[40], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_08, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[41], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_09, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[42], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_10, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[43], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_11, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[44], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_12, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[45], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_13, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[46], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_14, INT_SEG_PRESENT | INT_SEG_DPL_0);
  create_int_gate(&idt[47], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_15, INT_SEG_PRESENT | INT_SEG_DPL_0);

  /* Syscall handler */
  create_int_gate(&idt[50], X86_CONST_KERN_CS_SELECTOR, (lineaddr_t)swint_syscall, INT_SEG_PRESENT | INT_SEG_DPL_3);

  return EXIT_SUCCESS;
}



/**

   Function: void create_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags)
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



PRIVATE void create_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags)
{
  u8_t magic = 14;  /* 14 = 00001110b */

  /* Generic initialization */
  create_gate_desc(gate,seg,off);

  /* Attributes (bits 32 to 48) */
  gate->attributes = magic | flags;

  return;
}



/**

   Function: void create_gate_desc(struct gate_desc* gate, u16_t seg, u32_t off)
   ---------------------------------------------------------------------------

   Create the gate decriptor `gate` with segment selector `seg` and  offset `off`.
   Gate descriptors structures are described in `create_int_gate` and `create_trap_gate`

**/


PRIVATE void create_gate_desc(struct gate_desc* gate, u16_t seg, u32_t off)
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

