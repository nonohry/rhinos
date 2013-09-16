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

   Globals: TSS
   ------------

**/


PUBLIC struct tss tss;                        /* TSS */



/**

    Prototypes
    ----------

    Give access to segmentation setup

**/

PUBLIC u8_t vm_segment_setup(void);

#endif
