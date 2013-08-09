/**

   vmem_slab.c
   ===========

   Virtual memory slab allocator

**/



/**
 
   Includes
   --------

   - define.h
   - types.h
   - llist.h
   - arch_const.h
   - vmem_slab.h      : self header

**/

#include <define.h>
#include <types.h>
#include <llist.h>
#include <arch_const.h>
#include "vmem_slab.h"



/**

   Structure: struct bufctl
   -----------------------------

   Describe a bufctl.

   A bufctl is just an helper structure to point to a virtual memory area. 
   Members are:

   - base  : Virtual memory area base address
   - slab  : Parent slab back pointer
   - next  : Next bufctl in linked list
   - prev  : Previous bufctl in linked list

**/


PUBLIC struct bufctl
{
  virtaddr_t base;
  struct bufctl* next;
  struct bufctl* prev;
} __attribute__ ((packed));



/**

   Structure: struct slab
   ----------------------

   Describe a slab.

   A slab is basically  a free bufctl container.
   Membres are:

   - free_objects : Number of free bufctl available
   - free_bufctls : List of free bufctl
   - cache        : Parent cache back pointer
   - next         : Next slab in linked list
   - prev         : Previous slab in linked list

**/

PUBLIC struct slab
{
  u16_t free_objects;
  struct bufctl* free_bufctls;
  struct vmem_cache* cache;
  struct slab* next;
  struct slab* prev;
} __attribute__ ((packed));




/**

   Privates
   --------

   Helpers to make cache grow and to destroy a slab

**/

PRIVATE u8_t vmem_cache_grow(struct vmem_cache* cache);
PRIVATE u8_t virtmem_slab_destroy(struct vmem_cache* cache,struct vmem_slab* slab);


/**

   Global: Caches list
   -------------------

**/

PRIVATE struct vmem_cache* cache_list;


/**

   Global: cache_cache
   --------------------

   Cache of cache objects

**/

struct vmem_cache cache_cache =
  {
  name: "cache_cache",
  size: sizeof(struct vmem_cache),
  slabs_free: NULL,
  slabs_partial: NULL,
  slabs_full: NULL,
  next: NULL,
  prev: NULL
  };



/**

   Function: u8_t vmem_cache_setup(void)
   ----------------------------------------

   Slab allocator initialization.
   
   Just set `cache_cache` as head list

**/


PUBLIC u8_t vmem_cache_setup(void)
{
  /* Caches list initialization */
  LLIST_NULLIFY(cache_list);
  LLIST_SETHEAD(&cache_cache);

  return EXIT_SUCCESS;
}


/**

   Function: struct vmem_cache* vmem_cache_create(const char* name, u16_t size )
   -----------------------------------------------------------------------------

   Cache creation.

   Simply allocate a cache object from ̀cache_cache` and fill the structure fields with arguments.
   Return a pointer to the cache object or NULL if creation fails.

**/


PUBLIC struct vmem_cache* vmem_cache_create(const char* name, u16_t size)
{
  u8_t i;
  struct vmem_cache* cache;

  /* Cache allocation */
  cache = (struct vmem_cache*)vmem_cache_alloc(&cache_cache, VIRT_CACHE_DEFAULT);
  if ( cache == NULL )
    {
      return NULL;
    }

  /* Name copy */
  i=0;
  while( (name[i]!=0)&&(i<VMEM_CACHE_NAMELEN-1) )
    {
      cache->name[i] = name[i];
      i++;
    }
  cache->name[i]=0;

  /* Fill fields */
  cache->size = size;
  cache->slabs_free = NULL;
  cache->slabs_partial = NULL;
  cache->slabs_full = NULL;

  /* Link to cache_cache */
  LLIST_ADD(cache_list,cache);

  return cache;

}



/**

   Function: void* vmem_cache_alloc(struct vmem_cache* cache)
   ----------------------------------------------------------

   Allocation from a cache.

   First, if cache is full, it is extended.
   Next, we get a slab then a bufctl  and update slab objects list and cache slabs lists.
  
**/


PUBLIC void* vmem_cache_alloc(struct vmem_cache* cache)
{
  struct vmem_slab* list;
  struct vmem_slab* slab;
  struct vmem_bufctl* bufctl;
  
  /* Extend cache if needed */
  if ( (LLIST_ISNULL(cache->slabs_free)) && (LLIST_ISNULL(cache->slabs_partial)) )
    {
      if ( vmem_cache_grow(cache) != EXIT_SUCCESS )
	{
	  return NULL;
	}

    }

  /* Get the working slab list */ 
  list = (LLIST_ISNULL(cache->slabs_partial)?cache->slabs_free:cache->slabs_partial);

  /* Pick up head item */
  slab = LLIST_GETHEAD(list);

  /* Pick up a bufctl */
  bufctl = LLIST_GETHEAD(slab->free_bufctls);
  LLIST_REMOVE(slab->free_bufctls,bufctl);

  /* Update slab's free objects counter */
  slab->free_objects--;

  /* Move slab among cache lists if needed */
  if (list == cache->slabs_free)
    {
      LLIST_REMOVE(cache->slabs_free,slab);
      LLIST_ADD((slab->free_objects?:cache->slabs_partial,cache->slabs_full), slab);
    }
  else if ( !(slab->free_objects) )
    {
      LLIST_REMOVE(cache->slabs_partial,slab);
      LLIST_ADD(cache->slabs_full,slab);
    }

  /* Return bufctl base address */
  return (void*)(bufctl->base);

}



/**

   Function: u8_t vmem_cache_free(struct vmem_cache* cache, void* buf)
   -------------------------------------------------------------------

   Release a object pointed by `buf` and return it to `cache`.

   Only on page slab are manipulated so bufctl and buf are contiguous, and we retrieve bufctl using
   bufctl = buf - sizeof(bufctl)

   Then we retrieve slab just by aligning buf or bufctl on page: slab = bufctl >> ARCH_CONST_PAGE_SHIFT
   Now we can update slab's counter and cache slabs lists.

**/


PUBLIC u8_t vmem_cache_free(struct vmem_cache* cache, void* buf)
{
  struct bufctl* bc;
  struct slab* slab;

  /* Get bufctl */
  bc = (struct bufctl*)((virtaddr_t)buf - sizeof(struct bufctl));
 
  /* Get slab */
  slab = ( struct slab*)(bc >> ARCH_CONST_PAGE_SHIFT);

  /* Right cache ? */
  if (slab->cache != cache)
    {
      return EXIT_FAILURE;
    }

  /* Update slab counter and free list */
  slab->free_ojects++;
  LLIST_ADD(slab->free_buf,bc);

  /* Update cache slabs lists if needed  */
  if ( slab->free_objects-1 )
    {
      /* Move from partial to free if needed */
      if (!slab->free_objects)
	{
	  LLIST_REMOVE(cache->slabs_partial,slab);
	  LLIST_ADD(cache->slabs_free,slab);
	}
    }
  else
    {
      /* Move from full to partial or free */
      LLIST_REMOVE(cache->slabs_full,slab);
      if  (slab->free_objects == (ARCH_CONST_PAGE_SIZE - sizeof(struct slab))/(cache->size+sizeof(struct bufctl)) )
     	{
	  LLIST_ADD(cache->slabs_free,slab);
	}
      else
	{
	  LLIST_ADD(cache->slabs_partial,slab);
	}
    }
 

  return EXIT_SUCCESS;
}



/**

   Function: u8_t vmem_cache_destroy(struct vmem_cache* cache)
   --------------------------------------------------------------

   Destoy a cache

   Cache must be empty. We simply free slab objects and return `cache` to `cache_cache`.

**/


PUBLIC u8_t vmem_cache_destroy(struct vmem_cache* cache)
{
  u8_t i;
  struct vmem_slab* slab;

  /* Cache must be empty */
  if ( (!LLIST_ISNULL(cache->slabs_partial))
       || (!LLIST_ISNULL(cache->slabs_full)) )
    {
      return EXIT_FAILURE;
    }
 
  /* Run through free slabs list */
  while(!LLIST_ISNULL(cache->slabs_free))
    {
      slab = LLIST_GETHEAD(cache->slabs_free);
      /* Destroy slab */
      virtmem_slab_destroy(cache,slab);
      /* Detroy link */
      LLIST_REMOVE(cache->slabs_free,slab);
      /* Return slab to ̀slab_cache` if it is not an in-page slab */
       if (cache->size > (CONST_PAGE_SIZE >> VIRT_CACHE_GROWSHIFT))
	{
	  if ( vmem_cache_free(slab_cache,slab) != EXIT_SUCCESS )
	    {
	      return EXIT_FAILURE;
	    }
	}
    }
  
  /* Zeroing structure */
  for(i=0;i<VIRT_CACHE_NAMELEN;i++)
    {
      cache->name[i] = 0;
    }
  cache->size = 0;
  cache->align = 0;
  cache->constructor = NULL;
  cache->destructor = NULL;
  LLIST_REMOVE(cache_list,cache);

  /* Return to `cache_cache` */
  if ( vmem_cache_free(&cache_cache,cache) != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    }
 
  return EXIT_SUCCESS;
}



/**

   Function: u8_t virtmem_slab_destroy(struct vmem_cache* cache,struct vmem_slab* slab)
   ------------------------------------------------------------------------------------

   Helper to destroy a slab.
   
   Simply release slab objects, applying desctructor if defined. Slab must be empty.

**/


PRIVATE u8_t virtmem_slab_destroy(struct vmem_cache* cache,struct vmem_slab* slab)
{
  virtaddr_t page;

  /* Slab must be empty */
  if ( slab->count )
    {
      return EXIT_FAILURE;
    }
 
  /* Destroy objects and bufctls */
  while(!LLIST_ISNULL(slab->free_buf))
    {
      struct vmem_bufctl* bc = LLIST_GETHEAD(slab->free_buf);
      
      /* Apply destructor on objects */
      if (cache->destructor != NULL)
	{
	  cache->destructor((void*)(bc->base),cache->size);
	}

      /* Zeroing bufctl */
      bc->base = 0;
      bc->slab = NULL;
      LLIST_REMOVE(slab->free_buf,bc);

      /* Return bufctl to `bufctl_cache` in case of off-page slab */
      if (cache->size > (CONST_PAGE_SIZE >> VIRT_CACHE_GROWSHIFT))
	{
	  if ( vmem_cache_free(bufctl_cache,bc) != EXIT_SUCCESS )
	    {
	      return EXIT_FAILURE;
	    }
	}

    }

  /* Free virtual pages */
  page = PAGING_ALIGN_INF((virtaddr_t)slab->start);
  if ( virtmem_buddy_free((void*)page) != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    }

  /* Zeroing slab */
  slab->max_objects = 0;
  slab->n_pages = 0;
  slab->cache = NULL;
  slab->start = 0;

  return EXIT_SUCCESS;
}



/**

   Function: u8_t vmem_cache_grow(struct vmem_cache* cache)
   -----------------------------------------------------------

   Helper to extend cache providing on-page slabs.

   Slab is placed in front of page, then come successively bufctl and its object
   If `addr` is not set to ̀VIRT_CACHE_NOADDR` then cache will grow into the page pointed by `addr`.

**/


PRIVATE u8_t vmem_cache_grow(struct vmem_cache* cache)
{ 
  struct vmem_slab* slab;
  virtaddr_t buf;
  virtaddr_t page;
  u16_t buf_size, wasted;
  u8_t np;

  /* Number of pages needed */
  np = (PAGING_ALIGN_SUP(cache->size)) >> CONST_PAGE_SHIFT;

  /* Get a physically mapped virtual page */
  if (addr == VIRT_CACHE_NOADDR)
    {
      page = (virtaddr_t)virtmem_buddy_alloc(np*CONST_PAGE_SIZE, VIRT_BUDDY_MAP | VIRT_BUDDY_NOMINCHECK);
      if ( ((void*)page) == NULL )
	{
	  return EXIT_FAILURE;
	}
    }
  else
    {
      /* Check wether provided page is mapped */
      struct ppage_desc* pdesc = PHYS_GET_DESC( paging_virt2phys(addr)  );
      if ( PHYS_PDESC_ISNULL(pdesc) )
	{
	  return EXIT_FAILURE;
	}
      page = addr;
    }
  
  /* Place slab in front of page */
  slab = (struct vmem_slab*)page;
  slab->count = 0;
  slab->cache = cache;
  slab->n_pages = np;
  slab->start = page+sizeof(struct vmem_slab);
  LLIST_NULLIFY(slab->free_buf);


  /* Size of (bufctl+object) */
  buf_size = sizeof(struct vmem_bufctl) + cache->size;

  /* Compute number of object in page */
  slab->max_objects = (np*CONST_PAGE_SIZE - sizeof(struct vmem_slab)) / buf_size;

  /* Compute alignement */
  if (cache->align)
    {
      /* Remaining space */
      wasted = (np*CONST_PAGE_SIZE)-(slab->max_objects*cache->size);
      if (wasted)
	{
	  /* Remaining space will be used for cache coloring */
	  slab->start += cache->align_offset;
	  cache->align_offset = (cache->align_offset+cache->align)%wasted;
	}
    }

  /* Link new slab to cache */
  LLIST_ADD(cache->slabs_free,slab);

  /* Create bufctl and objects into the page */
  for(buf = slab->start;
      buf < (page+CONST_PAGE_SIZE-buf_size);
      buf += buf_size)
    {
      struct vmem_bufctl* bc = (struct vmem_bufctl*)buf;

      /* Initialize bufctl */
      bc->base = buf+sizeof(struct vmem_bufctl);
      bc->slab = slab;
      /* Apply constructor */
      if ( cache->constructor != NULL)
	{
	  cache->constructor((void*)(bc->base),cache->size);
	}
      /* Add bufctl to slab free list */
      LLIST_ADD(slab->free_buf,bc);
    }

  return EXIT_SUCCESS;
}
