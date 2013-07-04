/**
 
  klib.h
  ===========

  Header for klib_c.c and klib_s.s
  Give access to assembly utilities as well as C ones

 **/



#ifndef KLIB_H
#define KLIB_H


/**

   Includes
   --------

   - types.h
   - const.h
    
**/

#include <define.h>
#include <types.h>
#include "const.h"


/**

   Constants: Serial Port Relatives
   --------------------------------

**/

#define KLIB_SERIAL_PORT   0x3f8
#define KLIB_SERIAL_MASK   0x20
#define KLIB_BOCHS_PORT    0xe9


/**

   Prototypes
   ----------

   Assembly routines prototypes

**/

EXTERN void klib_bochs_print(char*,...);
EXTERN void klib_outb(u16_t,u8_t);
EXTERN void klib_inb(u16_t,u8_t*);
EXTERN u32_t klib_msb(u32_t);
EXTERN u32_t klib_lsb(u32_t);
EXTERN void klib_load_CR3(physaddr_t);
EXTERN void klib_set_pg_cr0(void);
EXTERN void klib_flush_tlb(void);
EXTERN void klib_invlpg(virtaddr_t);
EXTERN void klib_mem_set(u32_t, addr_t, u32_t);
EXTERN void klib_mem_copy(addr_t, addr_t, u32_t);
EXTERN void klib_sti(void);
EXTERN void klib_idle(void);


/**

   Prototypes
   ----------

   C routines prototypes

**/

PUBLIC void klib_serial_init(void);
PUBLIC void klib_printf(const char*,...);

#endif
