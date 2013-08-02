/**

   vm_pool.c
   =========

   Virtual pages pool, implemented as a stack
   

**/



/**

   Includes
   --------

   - define.h
   - types.h
   - arch_const.h  : Kernel boudaries needed
   - boot.h        : boot info
   - vm_pool.h     : self header

**/


#include <define.h>
#include <types.h>
#include <arch_const.h>
#include "boot.h"
#include "vm_pool.h"


/**

   Constants: pool relatives
   -------------------------

**/


#define MASK               (ARCH_CONST_PAGE_SIZE-1)


/**

   Macro: IS_ALIGNED(__addr)
   -------------------------

   Helper to check if `__addr` is aligned on page size

**/

#define IS_ALIGNED(__addr) (!((__addr) & (MASK)))


/**
   
   Global: stack
   -------------

   Virtual pages stack

**/

virtaddr_t* stack;


/**

   Global: top
   -----------

   Virtual pages stack top

**/

u32_t top;



/**

   Function: u8_t vm_pool_setup(void)
   ----------------------------------

   Initialize virtual page stack, filling it with free pages

**/


PUBLIC u8_t vm_pool_setup(void)
{
  virtaddr_t vaddr;

  /* Set stack */
  stack = (virtaddr_t*)(boot.vm_stack);

  /* Set top */
  top = 0;

  /* Fill stack */
  for(vaddr = 0;
      vaddr < ARCH_CONST_KERN_HIGHMEM;
      vaddr += ARCH_CONST_PAGE_SIZE)
    {
      /* Avoid kernel pages */
      if ( (vaddr >= ARCH_CONST_KERN_START)&&(vaddr < ARCH_CONST_KERN_END) )
	{
	  continue;
	}

      /* Avoid bitmap, vm stack et vm internal structures */
      if ( (vaddr >= (virtaddr_t)boot.bitmap)&&(vaddr < (virtaddr_t)boot.start) )
	{
	  continue;
	}

      /* Push page by freeing it*/
      vm_pool_free(vaddr);

    }

  return EXIT_SUCCESS;
}



/**

   Function: virtaddr_t vm_pool_alloc(void)
   ----------------------------------------

   Allocate a page.

   Simply return top of stack

**/


PUBLIC virtaddr_t vm_pool_alloc(void)
{
  if ( (top+1) < boot.vm_stack_size )
    {
      return stack[top++];
    }
  
  return VM_POOL_ERROR;
}



/**

   Function: u8_t vm_pool_free(u32_t addr)
   ---------------------------------------

   Release `addr` by pushing it on top of stack

**/


PUBLIC u8_t vm_pool_free(virtaddr_t vaddr)
{
  if ( (top) && (IS_ALIGNED(vaddr)) )
    {
      top--;
      stack[top] = vaddr;
      return EXIT_SUCCESS;
    }
        
  return EXIT_FAILURE;
}
