/**
 
   virtmem_buddy.c
   ---------------

   Virtual memory buddy allocator (big objects)


**/



/**

   Includes
   ---------

   - types.h
   - const.h
   - llist.h
   - klib.h
   - physmem.h       : physical memory allocation needed
   - paging.h        : paging_map needed
   - virtmem_slab.h  : slab allocation needed
   - virtmem_buddy.h : self header


**/

#include <types.h>
#include "const.h"
#include <llist.h>
#include "klib.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem_slab.h"
#include "virtmem_buddy.h"


/**

   Private: area_cache
   -------------------

   buddy items cache

**/

PRIVATE struct vmem_cache* area_cache;


/**

   Privates
   --------

   buddy system and list of allocated areas

**/

PRIVATE struct vmem_area* buddy_free[VIRT_BUDDY_MAX];
PRIVATE struct vmem_area* buddy_used;


/**

   Privates
   --------

   Helpers

**/

PRIVATE struct vmem_area* virtmem_buddy_alloc_area(u32_t size, u8_t flags);
PRIVATE u8_t virtmem_buddy_free_area(struct vmem_area* area);
PRIVATE u8_t virtmem_buddy_init_area(u32_t base, u32_t size);
PRIVATE u8_t virtmem_buddy_map_area(struct vmem_area* area);


/**

   Function: virtmem_buddy_init(void)
   ----------------------------------

   Virtual buddy initialization
   
   Nullify buddy lists and create buddy items cache
   Provide arbitrary pages to make cache grow into in order to avoid recursivity with slab allocator
   Add those arbitrary pages to `buddy_used`
   Enter all the available memory in the buddy.

**/

PUBLIC u8_t virtmem_buddy_init(void)
{
  struct vmem_area* area;
  virtaddr_t vaddr_init;
  physaddr_t paddr_init;
  u32_t i;


  /* Nullify buddy lists */
  for(i=0;i<VIRT_BUDDY_MAX;i++)
    {
      LLIST_NULLIFY(buddy_free[i]);
    }
  LLIST_NULLIFY(buddy_used);

  /* Create buddy items cache */
  area_cache = virtmem_cache_create("area_cache",sizeof(struct vmem_area),0,VIRT_BUDDY_MINSLABS,VIRT_CACHE_NOREAP,NULL,NULL);
  if ( area_cache == NULL)
    {
      return EXIT_FAILURE;
    }

  /* Provide pages for "manual" cache growth */
  for(i=0;i<VIRT_BUDDY_STARTSLABS;i++)
    {
      /* Set virtual address */
      vaddr_init = (i+VIRT_CACHE_STARTSLABS)*CONST_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT;

      /* Retrieve a physical page */
      paddr_init = (physaddr_t)phys_alloc(CONST_PAGE_SIZE);
      if (!paddr_init)
	{
	  return EXIT_FAILURE;
	}

      /* Map virtaul address with physical page */
      if (paging_map(vaddr_init, paddr_init, PAGING_SUPER) != EXIT_SUCCESS)
	{
	  return EXIT_FAILURE;
	}

      /* make cache grow into the virtual page */
      if ( virtmem_cache_grow(area_cache,vaddr_init) != EXIT_SUCCESS )
	{
	  return EXIT_FAILURE;
	}

    }

  /* Set "manual" initialization pages marked as used */
  for(i=0;i<VIRT_CACHE_STARTSLABS+VIRT_BUDDY_STARTSLABS;i++)
    {
      /* Create a fitting area for pages */
      area=(struct vmem_area*)virtmem_cache_alloc(area_cache, VIRT_CACHE_DEFAULT);
      if ( area == NULL ) 
	{ 
	  return EXIT_FAILURE;
	}
     
      area->base = i*CONST_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT;
      area->size = CONST_PAGE_SIZE;
      area->index = 0;
      /* Add area to used areas */
      LLIST_ADD(buddy_used,area);
    }

  /* Enter kernel available memory into buddy */
  if ( virtmem_buddy_init_area( (VIRT_CACHE_STARTSLABS+VIRT_BUDDY_STARTSLABS)*CONST_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT,
				CONST_KERN_HIGHMEM - ((VIRT_CACHE_STARTSLABS+VIRT_BUDDY_STARTSLABS)*CONST_PAGE_SIZE+VIRT_BUDDY_POOLLIMIT) ) != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}


/**

   Function: void* virtmem_buddy_alloc(u32_t size, u8_t flags)
   -----------------------------------------------------------

   Allocate a virtual memory chunk of `size` bytes (rounded to upper power of two).
   If ̀flags`  is set to `VIRT_BUDDY_MAP` then the memory will be mapped physically

   Return a pointer to the first byte of memory or NULL if allocation fails

**/

PUBLIC void* virtmem_buddy_alloc(u32_t size, u8_t flags)
{
  struct vmem_area* area;
  struct ppage_desc* pdesc;

  /* Minimum size */
  if (size < CONST_PAGE_SIZE )
    {
      size = CONST_PAGE_SIZE;
    }

  /* Allocate an area in the buddy */
  area = virtmem_buddy_alloc_area(size, flags);
  if ( area == NULL)
    {
      return NULL;
    }
 
  /* Physical map if needed */
  if ( flags & VIRT_BUDDY_MAP )
    {
      if ( virtmem_buddy_map_area(area)==EXIT_FAILURE )
	{
	  /* free area if mapping fails */
	  virtmem_buddy_free_area(area);
	  return NULL;
	}
      else
	{
	  /* link area to the physical memory descriptor */
	  pdesc = PHYS_GET_DESC( paging_virt2phys((virtaddr_t)area->base)  );
	    if (!PHYS_PDESC_ISNULL(pdesc))
	      {
		LLIST_REMOVE(buddy_used, area);
		LLIST_ADD(pdesc->area, area);
	      }
	}
    }

  /* Return area base address */
  return (void*)area->base;

}


/**

   Function: u8_t virtmem_buddy_free(void* addr)
   ---------------------------------------------

   Release virtual memory starting at `addr`

   Try to get the corresponding area using physical memory descriptor.
   Otherwise, run through `buddy_used` to find it
   Physically unmap memory if needed then make the area go back to the buddy
   

**/

PUBLIC u8_t virtmem_buddy_free(void* addr)
{
  struct vmem_area* area;
  struct ppage_desc* pdesc;
  virtaddr_t va;


  /* Get the area using physical descriptor */
  area = NULL;
  pdesc = PHYS_GET_DESC( paging_virt2phys((virtaddr_t)addr)  );
  if ( (!PHYS_PDESC_ISNULL(pdesc)) && (!LLIST_ISNULL(pdesc->area)) )
    {
      /* Run through pdesc area list */
      area = LLIST_GETHEAD(pdesc->area);
      do
	{
	  /* Found ! */
	  if (area->base == (virtaddr_t)addr)
	    {
	      break;
	    }
	  area = LLIST_NEXT(pdesc->area,area);
	}while(!LLIST_ISHEAD(pdesc->area,area));
    }

  /* Otherwise, look for the area in `buddy_used` */
  if ( (area == NULL) || (area->base != (virtaddr_t)addr) )
    {
      if (!LLIST_ISNULL(buddy_used))
	{
	  area = LLIST_GETHEAD(buddy_used);
	  do
	    {
	      /* Found ! */
	      if (area->base == (virtaddr_t)addr)
		{
		  LLIST_REMOVE(buddy_used,area);
		  break;
		}
	      area = LLIST_NEXT(buddy_used, area);
	    }while(!LLIST_ISHEAD(buddy_used, area));
	}
    }
  
 
  if (  (area != NULL) && (area->base == (virtaddr_t)addr) )
    {
      /* Physically unmap - no effect on unmapped memory */ 
      for(va=area->base;va<(area->base+area->size);va+=CONST_PAGE_SIZE)
	{
	  paging_unmap(va);
	}
      
      /* go back to buddy */
      if ( virtmem_buddy_free_area(area) != EXIT_SUCCESS )
	{
	  return EXIT_FAILURE;
	}
     
      return EXIT_SUCCESS;
      
    }

  return EXIT_FAILURE;
}



/**

   struct vmem_area* virtmem_buddy_alloc_area(u32_t size, u8_t flags)
   ------------------------------------------------------------------

   Helper in charge of allocating an area from buddy.
   Allocation occurs according to usual binary buddy algorithm
   

**/

PRIVATE struct vmem_area* virtmem_buddy_alloc_area(u32_t size, u8_t flags)
{

  u32_t i,j;
  int ind;
  struct vmem_area* area;

  /* Get upper power of two */
  size = size - 1;
  size = size | (size >> 1);
  size = size | (size >> 2);
  size = size | (size >> 4);
  size = size | (size >> 8);
  size = size | (size >> 16);
  size = size + 1;
  
  /* Deduct index */
  ind = klib_msb(size) - CONST_PAGE_SHIFT;
  
  /* Find first buddy level available */ 
  for(i=ind;LLIST_ISNULL(buddy_free[i])&&(i<VIRT_BUDDY_MAX);i++)
    {}

  /* No memory available: Error */
  if ( i>=VIRT_BUDDY_MAX )
    {
      return NULL;
    }
  
  /* Recursively divide an upper level area to meet `ind` level */
  for(j=i;j>ind;j--)
    {
      struct vmem_area* ar1;
     
      area = LLIST_GETHEAD(buddy_free[j]);
      LLIST_REMOVE(buddy_free[j],area);

      /* Allocate a new area */
      ar1 = (struct vmem_area*)virtmem_cache_alloc(area_cache, (flags&VIRT_BUDDY_NOMINCHECK?VIRT_CACHE_NOMINCHECK:VIRT_CACHE_DEFAULT));
      if ( ar1 == NULL )
	{
	  return NULL;
	}
      
      /* Divide an area */
      ar1->base = area->base + (area->size >> 1);
      ar1->size = (area->size >> 1);
      ar1->index = area->index-1;

      area->size = (area->size >> 1);
      area->index = area->index-1;

      LLIST_ADD(buddy_free[j-1],ar1);
      LLIST_ADD(buddy_free[j-1],area);

    }

  /* Here, an area is necessarily available at desired level */
  area = LLIST_GETHEAD(buddy_free[ind]);
  /* Update lists */
  LLIST_REMOVE(buddy_free[ind],area);
  LLIST_ADD(buddy_used,area);
  
  return area;
}


/**

   Function: u8_t virtmem_buddy_free_area(struct vmem_area* area)
   --------------------------------------------------------------

   Helper to reintegrate an area into the buddy.
   Used usual binary buddy algorithm. 

**/


PRIVATE u8_t virtmem_buddy_free_area(struct vmem_area* area)
{

  /* Recursively reintegrate area */
  while((area->index < VIRT_BUDDY_MAX-1)&&(!LLIST_ISNULL(buddy_free[area->index])))
    {
      struct vmem_area* buddy;
      
      /* look for a buddy  */
      buddy = LLIST_GETHEAD(buddy_free[area->index]);
      
      while ( (area->base+area->size != buddy->base)
	      && (buddy->base+buddy->size != area->base))
	{
	  buddy = LLIST_NEXT(buddy_free[area->index],buddy);
	  if (LLIST_ISHEAD(buddy_free[area->index],buddy))
	    {
	      /* No buddy, insert at that level */
	      LLIST_ADD(buddy_free[area->index],area);
	      return EXIT_SUCCESS;
	    }
	}
      
      /* Buddy found here, merge areas */
      area->base = (area->base<buddy->base?area->base:buddy->base);
      area->size <<= 1;
      area->index++;
      /* Remove buddy */
      LLIST_REMOVE(buddy_free[buddy->index],buddy);
      /* Free buddy */
      virtmem_cache_free(area_cache,buddy);

    }
  
  /* Reach last level or an empty one, insert area */
  LLIST_ADD(buddy_free[area->index],area);
  
  return EXIT_SUCCESS;
}


/**

   Function: u8_t virtmem_buddy_init_area(u32_t base, u32_t size)
   --------------------------------------------------------------

   Helper to enter an entire memory area defined by a base address and a size in the buddy.
   Implements a glutton knapsack algorithm to insert successively power of two aligned areas
   until we consume all the `size` bytes.   

**/

PRIVATE u8_t virtmem_buddy_init_area(u32_t base, u32_t size)
{
  u32_t power;
  u8_t ind;
  struct vmem_area* area;

  base = PAGING_ALIGN_SUP(base);

  while (size >= CONST_PAGE_SIZE)
    {
      /* Lower power of two */
      power = size;
      power = power | (power >> 1);
      power = power | (power >> 2);
      power = power | (power >> 4);
      power = power | (power >> 8);
      power = power | (power >> 16);
      power = power - (power >> 1);

      /* Deduct index */
      ind = klib_msb(power) - CONST_PAGE_SHIFT;

      /* Create an area */
      area = (struct vmem_area*)virtmem_cache_alloc(area_cache, VIRT_CACHE_DEFAULT);
      if ( area == NULL )
	{
	  return EXIT_FAILURE;
	}
    
      /* Fill the area */
      area->base = base;
      area->size = power;
      area->index = ind;

      /* Insert into buddy */
      LLIST_ADD(buddy_free[ind],area);
      
      /* Remaining memory */
      size -= power;
      base += power;
    }

  return EXIT_SUCCESS;
}



/**

   Function: u8_t virtmem_buddy_map_area(struct vmem_area* area)
   -------------------------------------------------------------

   Helper to physically map an area.

**/


PRIVATE u8_t virtmem_buddy_map_area(struct vmem_area* area)
{
  u32_t n,base,sum;
  physaddr_t paddr,pa;
  virtaddr_t va;
   
  n = area->size;
  base = area->base;
  sum = 0;

   
  while( (sum < area->size)&&(n) )
    {
      while (sum < area->size)
	{
	  /* Try to physically allocate */
	  paddr = (physaddr_t)phys_alloc(n);
	  if (paddr)
	    {
	      /* Increment total sum allocated */
	      sum += n;
	      
	      /* Mapping */
	      for(va=base,pa=paddr;
		  va<base+n;
		  va+=CONST_PAGE_SIZE,pa+=CONST_PAGE_SIZE)
		{
		  paging_map(va,pa,PAGING_SUPER);
		}
	      
	      /* Shift base to allocate */
	      base += n;
	      
	    }
	  else
	    {
	      /* Cannot allocate such a size */
	      break;
	    }
	}
      
      /* Divide size by two */
      n >>= 1;
    }
  
  /* Fail to map all the area, roll back mappings */
  if ( sum < area->size )
    {
      for(va=area->base;va<(area->base+area->size);va+=CONST_PAGE_SIZE)
	{
	  paging_unmap(va);
	}
      return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}
