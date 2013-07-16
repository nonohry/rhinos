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

   Structure: struct wm_allocator
   ------------------------------

   Describe the watermark allocator.
   Members are:
   
      - base  : base address, corresponding to the page in which allocations occur
      - wm    : watermark, id est current first available byte in page
      - limit : limit address

**/

struct wm_allocator
{
  addr_t base;
  addr_t wm;
  addr_t limit;
};



/**

   Gloabl: mem_wm
   --------------

   Global watermark allocator

**/


struct wm_allocator mem_wm;


/**

   Function: u8_t mem_setup(void)
   ------------------------------

   Memory manager initialization.
   Reserve a physical page for allocation and identity map it

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

  /* Hope there is enough room */
  if (e->len <  ARCH_CONST_PAGE_SIZE)
    {
      return EXIT_FAILURE;
    }
 
  /* Initialize allocator (note that values are page aligned) */
  mem_wm.base  = boot.start;
  mem_wm.limit = mem_wm.base + ARCH_CONST_PAGE_SIZE;
  mem_wm.wm = mem_wm.base;

  /* identity map  */
  if (arch_vm_map(mem_wm.base,mem_wm.base) != EXIT_SUCCESS)
    {
      return EXIT_FAILURE;
    }
  
  /* Nullify page  */
  arch_memset(0,mem_wm.base,ARCH_CONST_PAGE_SIZE);

  /* Update first available byte */
  boot.start = mem_wm.limit;

  return EXIT_SUCCESS;
}



/**

   Function: void* mem_alloc(size_t n)
   -----------------------------------

   Allocate a memory chunk of `n` bytes

   Simply update watermark and return its previous value if allocator's limit is not reached.
   Chunk size is in fact rounded to superior even.

**/
 
PUBLIC void* mem_alloc(size_t n)
{
  
  size_t size;

  /* Allocate an even size */
  size = (n&1?n+1:n);
  
  /* Limit not reached */
  if (mem_wm.wm+size < mem_wm.limit)
    {
      /* Update wm */
      mem_wm.wm += size;
      return (void*)(mem_wm.wm - size);
    }

  return NULL;
}


/**

   Function: u8_t mem_free(void* addr)
   -----------------------------------

   Free memory starting at `addr`

   In this basic watermark allocator, there is no memory release.

**/
 
PUBLIC u8_t mem_free(void* addr)
{
  /* Always succeeds */
  return EXIT_SUCCESS;
}
