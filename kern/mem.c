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
   - arch_vm.h     : architecture dependant virtual memory
   - boot.h        : boot information
   - mem.h

**/


#include <define.h>
#include <types.h>
#include <arch_const.h>
#include <arch_io.h>
#include <arch_vm.h>
#include "boot.h"
#include "mem.h"


/**

   Gloabl: mem_addr
   ----------------

   Memory page address containing internal structures

**/


addr_t mem_addr;


/**

   Function: u8_t mem_setup(void)
   ------------------------------

   Memory manager initialization.
   Allocate a page to create internal structures

**/


PUBLIC u8_t mem_setup(void)
{

  struct boot_mmap_entry* e;
  u16_t i;

  /* look for the memory map entry containing kernel */
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
     
  /* Assume `boot.start` is in the entry */
  if ((boot.start > (e->addr+e->len))||(boot.start < e->addr))
    {
      return EXIT_FAILURE;
    }
 
  /* Assign `mem_addr` */
  mem_addr = boot.start;

  /* identity map it */
  if (arch_vm_map(mem_addr,mem_addr) != EXIT_SUCCESS)
    {
      return EXIT_FAILURE;
    }
  
  /* Nullify page  */
  arch_memset(0,mem_addr,ARCH_CONST_PAGE_SIZE);

  return EXIT_SUCCESS;
}
