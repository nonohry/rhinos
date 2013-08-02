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
#include "boot.h"
#include "pager0.h"

#include <arch_io.h>


/**

   Constants: FREE & USED
   ----------------------

   State of a page

**/


#define FREE   0
#define USED   1
#define ERROR -1


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

PRIVATE s8_t pager0_getState(u32_t i);
PRIVATE u8_t pager0_setState(u32_t i, u8_t state);
PRIVATE physaddr_t pager0_alloc(void);
PRIVATE u8_t pager0_free(physaddr_t paddr); 

/**

   Function: u8_t pager0_setup(void)
   ---------------------------------

   Initilise pager0

   Create physical page bitmap

**/


u8_t pager0_setup(void)
{
  u8_t i;
  u32_t j;
  struct boot_mmap_entry* mmap;
    
  /* Set bitmap */
  bitmap = (u8_t*)(boot.bitmap);
  
  /* Run through memory map to fill bitmap */
  mmap = (struct boot_mmap_entry*)boot.mmap_addr;
  for(i=0;i<boot.mmap_length;i++)
    {
      
      for(j=(u32_t)mmap[i].addr;j<(u32_t)(mmap[i].addr+mmap[i].len);j+=ARCH_CONST_PAGE_SIZE)
	{

	  /* In use kernel memory and page 0 */
	  if ( ((j >= ARCH_CONST_KERN_START)&&(j < boot.start))||(j == 0) )
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


PRIVATE s8_t pager0_getState(u32_t i)
{
  if ( (i>>3) <= boot.bitmap_size )
    {
      return ((bitmap[i>>3] & (1 << (i%8)))?USED:FREE);
    }

  return ERROR;
}


/**

   Function: void pager0_setState(u32_t i, u8_t state)
   ---------------------------------------------------

   Set state of page `i` as `state`

**/


PRIVATE u8_t pager0_setState(u32_t i, u8_t state)
{
  if ( (i>>3) <= boot.bitmap_size )
    {
      if (state == FREE)
        {
	  bitmap[i>>3] &= ~(1<<(i%8));
        }
      else
        {
	  bitmap[i>>3] |= (1<<(i%8));
        }

        return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;

}


/**

   Function: physaddr_t pager0_alloc(void)
   ---------------------------------------

   Allocate a physical page from bitmap
   
   Simply run through bitmap and return first page available

**/


PRIVATE physaddr_t pager0_alloc(void)
{
  physaddr_t p;

  for(p=0;p<=boot.bitmap_size*8;p++)
    {
      /* Free page found */
      if ( pager0_getState(p) == FREE )
	{
	  /* Try to mark it as USED */
	  if ( pager0_setState(p,USED) == EXIT_SUCCESS )
	    {
	      return p;
	    }
	  else
	    {
	      /* Error on that page, find another one */
	      continue;
	    } 
	}
    }

  /* Not able to find a free page */
  return EXIT_FAILURE;
}


/**

   Function: u8_t pager0_free(physaddr_t paddr)
   --------------------------------------------

   Release a physical page into bitmap
   
   Just set page state in bitmap accordingly

**/


PRIVATE u8_t pager0_free(physaddr_t paddr)
{
  if ( pager0_getState(paddr) == USED )
    {
      return pager0_setState(paddr,FREE);
    }

  return EXIT_FAILURE;
}
