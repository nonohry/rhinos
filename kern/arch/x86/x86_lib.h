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
    
**/

#include <define.h>
#include <types.h>


EXTERN void x86_outb(u16_t,u8_t);
EXTERN void x86_inb(u16_t,u8_t*);
EXTERN void x86_mem_copy(addr_t src, addr_t dest, u32_t len);


#endif
