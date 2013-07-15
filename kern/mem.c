/**

   mem.c
   =====

   Basic memory manager for kernel initializations

**/



/**

   Includes
   --------

   - define.h
   - types.h
   - arch_const.h  : page size
   - arch_io.h     : output primitives
   - boot.h        : boot information
   - mem.h

**/


#include <define.h>
#include <types.h>
#include <arch_const.h>
#include <arch_io.h>
#include "boot.h"
#include "mem.h"


/**

   Constants: Memory states
   ------------------------

**/


#define MEM_FREE     0
#define MEM_USED     1



/**

   Structure: struct mem_desc
   --------------------------

   Internal structure used by memory manger to track memory chunks state
   Members are self-explanatory


**/


struct mem_desc
{
  physaddr_t addr;
  u32_t len;
  u8_t state;
}__attribute__((packed));



/**

   Function: u8_t mem_init(void)
   -----------------------------

   Memory manager initialization.
   Allocate a page to create internal structures

**/


PUBLIC u8_t mem_init(void)
{

  struct boot_mmap_entry* e;
  u16_t i;

  /* look for memory map entry containing kernel */
  for(i=0,e=(struct boot_mmap_entry*)boot.mmap_addr;
      (i<boot.mmap_length)&&(e->addr != ARCH_CONST_KERN_START);
      i++,e++)
    {}

  /* Not found ... */
  if (i>=boot.mmap_length)
    {
      return EXIT_FAILURE;
    }

  /* Type error */
  if (e->type != BOOT_AVAILABLE)
    {
      return EXIT_FAILURE;
    }
      
      

  arch_printf("Kernel in 0x%x (0x%x,%d)\n",(u32_t)e->addr,(u32_t)e->len,e->type);

  
  return EXIT_SUCCESS;
}
