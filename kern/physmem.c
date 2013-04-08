/**

   physmem.c
   =========

   Physical memory management

**/



/**

   Includes
   --------

   - types.h
   - const.h
   - llist.h
   - klib.h
   - start.h     : Memory map needed
   - physmem.h   : self header

**/

#include <types.h>
#include "const.h"
#include <llist.h>
#include "klib.h"
#include "start.h"
#include "physmem.h"


/**
   
   Privates
   --------

   - void phys_init_area(u32_t base, u32_t size)
   - struct ppage_desc* ppage_free[PHYS_PAGE_MAX_BUDDY] : the buddy system

**/

PRIVATE void phys_init_area(u32_t base, u32_t size);
PRIVATE struct ppage_desc* ppage_free[PHYS_PAGE_MAX_BUDDY];


/**

   Function:  u8_t phys_init(void)
   -------------------------------

   Physical memory manger initialization.
   Nullify the buddy system, then compute the size of memory needed to build the buddy system.
   Finally, use the boot memory map to fill the buddy system with free memory

**/

PUBLIC u8_t phys_init(void)
{
  s32_t i;
  u8_t ok=0;
  u32_t pool_size;
  struct multiboot_mmap_entry* entry;

  /* Nullify buddy system */  
  for(i=0; i<PHYS_PAGE_MAX_BUDDY; i++)
    {
      LLIST_NULLIFY(ppage_free[i]);
    }

  /* Compute buddy items pool max size */
  pool_size = ((start_mem_total) >> CONST_PAGE_SHIFT)*sizeof(struct ppage_desc);

  /* Fill buddy with free memory */
  for(entry=(struct multiboot_mmap_entry*)start_mbi->mmap_addr,i=0;i<start_mbi->mmap_length;i++,entry++)
    {
      if (entry->type == START_E820_AVAILABLE)
	{

	  physaddr_t base = (physaddr_t)entry->addr;
	  u32_t size = (u32_t)entry->len;

	  /* First page is system reserved */
	  if (base == 0)
	    {
	      size -= CONST_PAGE_SIZE;
	      base += CONST_PAGE_SIZE;
	    }
	
	  /* Interval [CONST_KERN_START - NODE_POOL_ADDR+pool_size] is not available */
	  if ( (base == CONST_KERN_START)&&(base+size > CONST_PAGE_NODE_POOL_ADDR+pool_size) )
	    {
	      /* Indicate we have protected the in-used physical memory */
	      ok = 1;
	      
	      /* update interval size */
	      size -= CONST_PAGE_NODE_POOL_ADDR+pool_size - base;
	      base = CONST_PAGE_NODE_POOL_ADDR+pool_size;
	
	    }

	  /* Enter interval in the buddy */
	  phys_init_area(base,size);
	}
    }

  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}


/**

   Function: void* phys_alloc(u32_t size)
   --------------------------------------

   Allocate 2-aligned `size` bytes of physical memory.
   This is a classical binary buddy system implementation: divide areas by two until we found a good one.
   Return a 2-aligned `size` bytes physical memory area address or NULL if it fails.

**/

PUBLIC void* phys_alloc(u32_t size)
{

  u32_t i,j;
  s32_t ind;
  struct ppage_desc* pdesc;


  /* Get the upper power of two */
  size = size - 1;
  size = size | (size >> 1);
  size = size | (size >> 2);
  size = size | (size >> 4);
  size = size | (size >> 8);
  size = size | (size >> 16);
  size = size + 1;
  
  /* Use it to compute the index in the buddy system */
  ind = klib_msb(size) - CONST_PAGE_SHIFT;
  
  /* If no area at `ind`, find the first upper level available */
  for(i=ind;LLIST_ISNULL(ppage_free[i])&&(i<PHYS_PAGE_MAX_BUDDY);i++)
    {}

  /* No area available ? Error */
  if ( i >= PHYS_PAGE_MAX_BUDDY )
    {
      return NULL;
    }

  
  /* Recursively divide an upper level area to meet `ind` level */
  for(j=i;j>ind;j--)
    {
      struct ppage_desc* pd1;
     
      /* Get the first page decriptor available in the level */
      pdesc = LLIST_GETHEAD(ppage_free[j]);
      LLIST_REMOVE(ppage_free[j],pdesc);

      /* Get a new physical page descriptor in the pool */
      pd1 = PHYS_GET_DESC(pdesc->start + (pdesc->size >> 1));
      PHYS_NULLIFY_DESC(pd1);

      /* Make 2 descriptors with one */

      pd1->start = pdesc->start+ (pdesc->size >> 1);
      pd1->size = (pdesc->size >> 1);
      pd1->index = pdesc->index-1;

      pdesc->size = (pdesc->size >> 1);
      pdesc->index = pdesc->index-1;

      /* Add the 2 descriptors to the previous level */
      LLIST_ADD(ppage_free[j-1],pd1);
      LLIST_ADD(ppage_free[j-1],pdesc);

    }

  /* Now we have at least 2 page descriptors at the desired level. Get one */
  pdesc = LLIST_GETHEAD(ppage_free[ind]);
  LLIST_REMOVE(ppage_free[ind],pdesc);

  /* Return the adress */ 
  return (void*)(pdesc->start);
}



/**

   Function: u8_t phys_free(void* addr)
   ------------------------------------

   Free the physical memory area pointed by `addr`.
   In fact, it simply re-enters the page descriptor in the buddy system, using the 
   classical binary system implementation. That is to say if the page descriptor describes an area
   that share bondaries with another at the same level (a buddy), the 2 areas are merged in the upper level, and so on
   
**/

PUBLIC u8_t phys_free(void* addr)
{
  struct ppage_desc* pdesc;

  /* Get the physical descriptor  bounded to `addr` */
  pdesc = PHYS_GET_DESC((physaddr_t)addr);
  if ( PHYS_PDESC_ISNULL(pdesc) )
    {
      return EXIT_FAILURE;
    }

  /* Nil size ? Error */
  if ( !pdesc->size )
    {
      return EXIT_FAILURE;
    }

  /* Recursively insert the descriptor */
  while((pdesc->index < PHYS_PAGE_MAX_BUDDY-1)&&(!LLIST_ISNULL(ppage_free[pdesc->index])))
    {
      struct ppage_desc* buddy;
      
      /* Get the corresponding level in the buddy */
      buddy = LLIST_GETHEAD(ppage_free[pdesc->index]);
      
      /* Find a sharing boundaries buddy */
      while ( (pdesc->start+pdesc->size != buddy->start)
	      && (buddy->start+buddy->size != pdesc->start))
	{
	  buddy = LLIST_NEXT(ppage_free[pdesc->index],buddy);
	  if (LLIST_ISHEAD(ppage_free[pdesc->index],buddy))
	    {
	      /* No buddy, insert at that level */
	      LLIST_ADD(ppage_free[pdesc->index],pdesc);
	      return EXIT_SUCCESS;
	    }
	}
      
      /* We found a buddy here ! Merge the descriptor */
      pdesc->start = (pdesc->start<buddy->start?pdesc->start:buddy->start);
      pdesc->size <<= 1;
      pdesc->index++;
      /* Remove our buddy */
      LLIST_REMOVE(ppage_free[buddy->index],buddy);
      /* And free it */
      PHYS_NULLIFY_DESC(buddy);

    }
  
  /* Here, the corresponding level is empty, or we get to the last one.
     Just add the descriptor at that level */
  LLIST_ADD(ppage_free[pdesc->index],pdesc);
  
  return EXIT_SUCCESS;
}



/**

   Function: u8_t phys_map(physaddr_t addr)
   ----------------------------------------

   Indicate a mapping between the physical address `addr` and a virtual addr.
   We also use this function to compute the number of physical pages pointed by a page table 
   in order to free the page table when it no longer points to physical pages.

**/


PUBLIC u8_t phys_map(physaddr_t addr)
{
  struct ppage_desc* pdesc;
  
  /* Get the page descriptor */
  pdesc = PHYS_GET_DESC(addr);

  if (pdesc->size)
    {
      /* Increment number of mapping */
      pdesc->maps++;
      return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;
}


/**

   Function: u8_t phys_unmap(physaddr_t addr, u8_t flag)
   -----------------------------------------------------

   Decrement the number of mapping that involve the physical page pointed by `addr`.
   If `flag` is set to PHYS_UNMAP_DEFAULT and there is no more mapping, the page returns to the buddy system.
   Nothing to do otherwise.

   Return PHYS_UNMAP_FREE if the page is freed in the buddy, PHYS_UNMAP_NONE otherwise.

**/

PUBLIC u8_t phys_unmap(physaddr_t addr, u8_t flag)
{
  struct ppage_desc* pdesc;
  
  /* Get page descriptor */
  pdesc = PHYS_GET_DESC(addr);
  if ( PHYS_PDESC_ISNULL(pdesc) )
    {
      return PHYS_UNMAP_NONE;
    }
 
  if ((pdesc->size)&&(pdesc->maps))
    {
      /* Decrement mapping number */
      pdesc->maps--;

      /* no mapping anymore and the flag is set to PHYS_UNMAP_DEFAULT (free page) ? */
      if ( (!(pdesc->maps))&&(flag == PHYS_UNMAP_DEFAULT) )
	{
	  /* Free the page */
	  if ( phys_free((void*)addr) == EXIT_SUCCESS )
	    {
	      return PHYS_UNMAP_FREE;
	    }
	  else
	    {
	      /* Error during page release. Get back to original state */
	      pdesc->maps++;
	    }
	}
    }

  return PHYS_UNMAP_NONE;
}


/**

   Function: void phys_init_area(u32_t base, u32_t size)
   -----------------------------------------------------

   Helper to insert a `size` bytes area starting at `base` in the buddy system. 
   In fact, it implements a glutton knapsack algorithm to insert successively power of two aligned areas
   until we consume all the `size` bytes.

**/
   
PRIVATE void phys_init_area(u32_t base, u32_t size)
{
  u32_t power;
  u8_t ind;
  struct ppage_desc* pdesc;

  base = PHYS_ALIGN_SUP(base);

  /* Recursively insert lower power of two size areas */
  while (size >= CONST_PAGE_SIZE)
    {
      /* Lower power of 2 */
      power = size;
      power = power | (power >> 1);
      power = power | (power >> 2);
      power = power | (power >> 4);
      power = power | (power >> 8);
      power = power | (power >> 16);
      power = power - (power >> 1);

      /* Compute the corresponding index in the buddy */
      ind = klib_msb(power) - CONST_PAGE_SHIFT;

      /* Get the page descriptor bounded to the `base` address  */
      pdesc = PHYS_GET_DESC(base);
      PHYS_NULLIFY_DESC(pdesc);
 
      /* Update page descriptor */
      pdesc->start = base;
      pdesc->size = power;
      pdesc->index = ind;

      /* Add it in that level */
      LLIST_ADD(ppage_free[ind],pdesc);
      
      /* Update remaining memory */
      size -= power;
      base += power;
    }

  return;
}
