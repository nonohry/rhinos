/**

   pager0.c
   ========

   Kernel page fault handler 

**/



/**

   Includes
   --------

   - define.h
   - types.h
   - pager0.h   : self header
   
**/


#include <define.h>
#include <types.h>
#include <arch_const.h>
#include <arch_vm.h>
#include "boot.h"
#include "pager0.h"

#include <arch_io.h>


/**

   Constants: FREE & USED
   ----------------------

   State of a page

**/


#define FREE 0
#define USED 1



/**

   Global: bitmap
   --------------

   Bitmap representing physical page availability

**/

u8_t* bitmap;



/** 

    Privates
    --------

    Helpers

**/

PRIVATE u8_t pager0_getState(u32_t i);
PRIVATE void pager0_setState(u32_t i, u8_t state);


/**

   Function: u8_t pager0_setup(void)
   ---------------------------------

   Initilise pager0

   Create physical page bitmap

**/


u8_t pager0_setup(void)
{
  u8_t i;
  virtaddr_t vaddr;
  u32_t j,bitmap_size,mem=0;
  struct boot_mmap_entry* mmap;
  
  /* Run through memory map to get memory amount */
  mmap = (struct boot_mmap_entry*)boot.mmap_addr;
  for(i=0;i<boot.mmap_length;i++)
    {
      /* Update mem */
      mem += mmap[i].len;
      
    }
  
  /* Compute number of page */
  bitmap_size = mem / ARCH_CONST_PAGE_SIZE;
  
  /* Reserve a bitmap of npages bits */
  bitmap = (u8_t*)(boot.start);
  boot.start +=  (((bitmap_size >> X86_CONST_PAGE_SHIFT)+1) << X86_CONST_PAGE_SHIFT);

  /* Identity map */
  for(vaddr = (virtaddr_t)bitmap;
      vaddr < boot.start;
      vaddr += ARCH_CONST_PAGE_SIZE)
    {
      if (arch_vm_map(vaddr,(physaddr_t)vaddr) != EXIT_SUCCESS)
	{
	  return EXIT_FAILURE;
	}
    }

  
  /* Run through memory map to fill bitmap */
  for(i=0;i<boot.mmap_length;i++)
    {
      
      for(j=(u32_t)mmap[i].addr;j<(u32_t)(mmap[i].addr+mmap[i].len);j+=ARCH_CONST_PAGE_SIZE)
	{
	  /* In use kernel memory */
	  if ( (j >= ARCH_CONST_KERN_START)&&(j < boot.start) )
      	    {
	      pager0_setState(j/ARCH_CONST_PAGE_SIZE,USED);
	    }
	  else
	    {
	      /* Other memory depend on memory map type */
	      pager0_setState(j/ARCH_CONST_PAGE_SIZE,(mmap[i].type == BOOT_AVAILABLE?FREE:USED));
	    }
	}
    }


  return EXIT_SUCCESS;
}



/**

   Function: u8_t pager0_getState(u32_t i)
   ---------------------------------------

   Return state of page `i` in bitmap

**/


PRIVATE u8_t pager0_getState(u32_t i)
{
  return ((bitmap[i>>3] & (1 << (i%8)))?USED:FREE);
}


/**

   Function: void pager0_setState(u32_t i, u8_t state)
   ---------------------------------------------------

   Set state of page `i` as `state`

**/


PRIVATE void pager0_setState(u32_t i, u8_t state)
{
        if (state == FREE)
        {
                bitmap[i>>3] &= ~(1<<(i%8));
        }
        else
        {
                bitmap[i>>3] |= (1<<(i%8));
        }

        return;
}
