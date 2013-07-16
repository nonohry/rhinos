/**
 
  x86_lib.h
  ==========

  Give access to assembly utilities defined in x86_lib.s

 **/



#ifndef X86_LIB_H
#define X86_LIB_H


/**

   Includes
   --------

   - define.h
   - types.h
   - x86_const.h
    
**/


#include <define.h>
#include <types.h>
#include "x86_const.h"


/** 

    Macro: X86_ALIGN_INF(__addr)
   -------------------------------

   Align `__addr` on  lower page size boundary

**/

#define X86_ALIGN_INF(__addr)			\
  ( ((__addr) >> X86_CONST_PAGE_SHIFT) << X86_CONST_PAGE_SHIFT )


/**

   Macro: X86_ALIGN_SUP(__addr)
   -------------------------------

   Align `__addr` on  upper page size boundary

**/

#define X86_ALIGN_SUP(__addr)			 \
  ( (((__addr)&0xFFFFF000) == (__addr))?((__addr) >> X86_CONST_PAGE_SHIFT) << X86_CONST_PAGE_SHIFT:(((__addr) >> X86_CONST_PAGE_SHIFT)+1) << X86_CONST_PAGE_SHIFT )



/**

   Prototypes
   ----------

   Give access to assembly definitions

**/

EXTERN void x86_outb(u16_t,u8_t);
EXTERN void x86_inb(u16_t,u8_t*);
EXTERN void x86_mem_copy(addr_t src, addr_t dest, size_t len);
EXTERN void x86_mem_set(u32_t val, addr_t dest, size_t len);
EXTERN void x86_load_pd(physaddr_t pd);

#endif
