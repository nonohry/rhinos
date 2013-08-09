/**

   virtmem_slab.c
   ==============


   Virtual memory slab allocator (small objects)

**/


/**
 
   Includes
   --------

   - define.h
   - types.h
   - const.h
   - llist.h
   - klib.h
   - start.h          : start_mem_total needed in VIRT_POOLIMIT macro
   - physmem.h        : physical allocation
   - paging.h         : mapping needed
   - virtmem_buddy.h  : buddy allocation/release needed
   - virtmem_slab.h   : self header

**/

#include <define.h>
#include <types.h>
#include "const.h"
#include <llist.h>
#include "klib.h"
#include "start.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem_buddy.h"
#include "virtmem_slab.h"






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
  struct vmem_slab* slab;
  struct vmem_bufctl* next;
  struct vmem_bufctl* prev;
} __attribute__ ((packed));



/**

   Structure: struct slab
   ----------------------

   Describe a slab.

   A slab is basically  a free bufctl container.
   Membres are:

   - free_objects : Number of free bufctl available
   - max_objects  : Maximum free objects a slab can contain
   - free_bufctls : List of free bufctl
   - cache        : Parent cache back pointer
   - next         : Next slab in linked list
   - prev         : Previous slab in linked list

**/

PUBLIC struct slab
{
  u16_t free_objects;
  u16_t max_objects;
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

PRIVATE u8_t virtmem_cache_grow_big(struct vmem_cache* cache, virtaddr_t addr);
PRIVATE u8_t virtmem_cache_grow_little(struct vmem_cache* cache, virtaddr_t addr); 
PRIVATE u8_t virtmem_slab_destroy(struct vmem_cache* cache,struct vmem_slab* slab);


/**

   Privates
   --------

   Caches for caches, slab and bufctl objects

**/

PRIVATE struct vmem_cache* slab_cache;
PRIVATE struct vmem_cache* bufctl_cache;
PRIVATE struct vmem_cache* cache_list;


/**

   Static: cache_cache
   -------------------

   Cache of cache objects

**/

static struct vmem_cache cache_cache =
  {
  name: "cache_cache",
  size: sizeof(struct vmem_cache),
  align: 0,
  min_slab_free: 0,
  flags: VIRT_CACHE_NOREAP,
  constructor: NULL,
  destructor: NULL,
  slabs_free: NULL,
  slabs_partial: NULL,
  slabs_full: NULL,
  next: NULL,
  prev: NULL
  };



/**

   Function: u8_t virtmem_cache_init(void)
   ---------------------------------------

   Slab allocator initialization.

   Add arbitrary virtual pages to the cache objects cache to avoid initial recursivity with virtual buddy.
   Then create slab and bufctl caches.

**/


PUBLIC u8_t virtmem_cache_init(void)
{
  virtaddr_t vaddr_init;
  physaddr_t paddr_init;
  u32_t i;


  /* Caches list intialization */
  LLIST_NULLIFY(cache_list);
  LLIST_SETHEAD(&cache_cache);
      
  /* `cache_cache` "manual" growth */
  for(i=0;i<VIRT_CACHE_STARTSLABS;i++)
    {
      /* Arbitrary virtual page */
      vaddr_init = i*CONST_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT;

      /* Pick up a physical page */
      paddr_init = (physaddr_t)phys_alloc(CONST_PAGE_SIZE);
      
      if (!paddr_init)
	{
	  return EXIT_FAILURE;
	}

      /* Mapping */
      if (paging_map(vaddr_init, paddr_init, PAGING_SUPER) != EXIT_SUCCESS)
	{
	  return EXIT_FAILURE;
	}
      
      /* Make `cache_cache` grow into that virtual page */
      if ( virtmem_cache_grow(&cache_cache,vaddr_init) != EXIT_SUCCESS )
	{
	  return EXIT_FAILURE;
	}
           
    }

  /* Slab objects cache */
  slab_cache = virtmem_cache_create("slab_cache",sizeof(struct vmem_slab),0,0,VIRT_CACHE_NOREAP,NULL,NULL);
  if ( slab_cache == NULL )
    {
      return EXIT_FAILURE;
    }

  /* Bufctl objects cache */
  bufctl_cache = virtmem_cache_create("bufctl_cache",sizeof(struct vmem_bufctl),0,0,VIRT_CACHE_NOREAP,NULL,NULL);
  if ( bufctl_cache == NULL )
    {
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}


/**

   Function: struct vmem_cache* virtmem_cache_create(const char* name, u16_t size, u16_t align, u16_t min_slab_free, u8_t flags, void (*ctor)(void*,u32_t), void (*dtor)(void*,u32_t))
   -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

   Cache creation.

   Simply allocate a cache object from ̀cache_cache` and fill the structure fields with arguments.
   Return a pointer to the cache object or NULL if creation fails.

**/


PUBLIC struct vmem_cache* virtmem_cache_create(const char* name, u16_t size, u16_t align, u16_t min_slab_free, u8_t flags, void (*ctor)(void*,u32_t), void (*dtor)(void*,u32_t))
{
  u8_t i;
  struct vmem_cache* cache;

  /* Cache allocation */
  cache = (struct vmem_cache*)virtmem_cache_alloc(&cache_cache, VIRT_CACHE_DEFAULT);
  if ( cache == NULL )
    {
      return NULL;
    }

  /* Name copy */
  i=0;
  while( (name[i]!=0)&&(i<VIRT_CACHE_NAMELEN-1) )
    {
      cache->name[i] = name[i];
      i++;
    }
  cache->name[i]=0;

  /* Fill fields */
  cache->size = size;
  cache->align = align;
  cache->align_offset = 0;
  cache->min_slab_free = min_slab_free;
  cache->flags = flags;
  cache->constructor = ctor;
  cache->destructor = dtor;
  cache->slabs_free = NULL;
  cache->slabs_partial = NULL;
  cache->slabs_full = NULL;

  /* Link to cache_cache */
  LLIST_ADD(cache_list,cache);

  return cache;

}


/**

   Function: u8_t virtmem_cache_free(struct vmem_cache* cache, void* buf)
   ----------------------------------------------------------------------

   Release a object pointed by `buf` and return it to `cache`.

   Objects in slab allocator are necessarily physically mapped, so we retrieve the physical descriptor and run through
   its bufctl list to find the one matching `buf`.
   Then, we deduct the parent slab object et update its object counter.
   At last, the slab is moved between cache slabs lists if needed

**/


PUBLIC u8_t virtmem_cache_free(struct vmem_cache* cache, void* buf)
{
  struct ppage_desc* pdesc;
  struct vmem_bufctl* bc;
  struct vmem_slab* slab;

  /* Get physical descriptor */
  pdesc = PHYS_GET_DESC( paging_virt2phys((virtaddr_t)buf) );
  if ( PHYS_PDESC_ISNULL(pdesc) )
    {
      return EXIT_FAILURE;
    }
 
  /* Run through bufctl list */
  if ( LLIST_ISNULL(pdesc->bufctl) )
    {
      return EXIT_FAILURE;
    }

  bc = LLIST_GETHEAD(pdesc->bufctl);
  while( bc->base != (virtaddr_t)buf )
    {
      bc = LLIST_NEXT(pdesc->bufctl,bc);
      /* Error if not found */
      if ( LLIST_ISHEAD(pdesc->bufctl,bc) )
	{
	  return EXIT_FAILURE;
	}
    }


  /* Get parent slab */
  slab = bc->slab;

  /* Righ cache ? */
  if (slab->cache != cache)
    {
      return EXIT_FAILURE;
    }

  /* Remove bufctl from physical descriptor */
  LLIST_REMOVE(pdesc->bufctl,bc);


  /* Update slab counter and free list */
  slab->count--;
  LLIST_ADD(slab->free_buf,bc);

  /* Update cache slabs lists if needed  */
  if (slab->count+1 == slab->max_objects)
    {
      /* Move from full to partial or free */
      LLIST_REMOVE(cache->slabs_full,slab);
      if (slab->count)
	{
	  LLIST_ADD(cache->slabs_partial,slab);
	}
      else
	{
	  LLIST_ADD(cache->slabs_free,slab);
	}
    }
  else
    {
      /* Move from partial to free */
      if (!slab->count)
	{
	  LLIST_REMOVE(cache->slabs_partial,slab);
	  LLIST_ADD(cache->slabs_free,slab);
	}
    }

  return EXIT_SUCCESS;
}


/**

   Function: void* virtmem_cache_alloc(struct vmem_cache* cache, u8_t flags)
   -------------------------------------------------------------------------

   Allocation from a cache.

   First, if cache is full, it is extended.
   Next, we get a slab then a bufctl  and update slab objects list and cache slabs lists.
   Last, we check the minimum free slabs required in `cache` and act accordingly
   

**/


PUBLIC void* virtmem_cache_alloc(struct vmem_cache* cache, u8_t flags)
{
  struct vmem_slab* slabs_list;
  struct vmem_slab* slab;
  struct vmem_bufctl* bufctl;
  struct ppage_desc* pdesc;
  u32_t count;

  /* Extend cache if needed */
  if ( (LLIST_ISNULL(cache->slabs_free)) && (LLIST_ISNULL(cache->slabs_partial)) )
    {
      if ( virtmem_cache_grow(cache, VIRT_CACHE_NOADDR) != EXIT_SUCCESS )
	{
	  return NULL;
	}

    }

  /* Get the working slab list */ 
  slabs_list = (LLIST_ISNULL(cache->slabs_partial)?cache->slabs_free:cache->slabs_partial);

  /* Pick up head item */
  slab = LLIST_GETHEAD(slabs_list);

  /* Pick up a bufctl */
  bufctl = LLIST_GETHEAD(slab->free_buf);
  LLIST_REMOVE(slab->free_buf,bufctl);

  /* Link bufctl to its physical descriptor */
  pdesc = PHYS_GET_DESC( paging_virt2phys(bufctl->base)  );
  if ( PHYS_PDESC_ISNULL(pdesc) )
    {
      return NULL;
    }
  LLIST_ADD(pdesc->bufctl,bufctl);
  
  /* Physical descriptor back pointer to cache */
  pdesc->cache = cache;
  
  /* Update slab allocated objects counter */
  slab->count++;

  /* Move slab between cache lists if needed */
  if (slabs_list == cache->slabs_free)
    {
      LLIST_REMOVE(cache->slabs_free,slab);
      if (slab->count == slab->max_objects)
	{
	  LLIST_ADD(cache->slabs_full,slab);
	}
      else
	{
	  LLIST_ADD(cache->slabs_partial,slab);
	}
    }
  else
    {
      if (slab->count == slab->max_objects)
	{
	  LLIST_REMOVE(cache->slabs_partial,slab);
	  LLIST_ADD(cache->slabs_full,slab);
	}
    }


  /* Check `min_slab_free` */
  if ( (cache->min_slab_free) && !(flags & VIRT_CACHE_NOMINCHECK) )
    {
      /* Count free slabs */
      count = 0;
      if (!LLIST_ISNULL(cache->slabs_free))
	{
	  slab = LLIST_GETHEAD(cache->slabs_free);
	  do
	    {
	      count++;
	      slab = LLIST_NEXT(cache->slabs_free,slab);
	    }while(!LLIST_ISHEAD(cache->slabs_free,slab));
	}
      
      /* Extend cache according to `min_slab_free`  */
      while (count < cache->min_slab_free )
	{
	  if (virtmem_cache_grow(cache, VIRT_CACHE_NOADDR) != EXIT_SUCCESS)
	    {
	      break;
	    }
	  count++;
	}
    }
  

  /* Return bufctl base address */
  return (void*)(bufctl->base);

}


/**

   Function: u8_t virtmem_cache_destroy(struct vmem_cache* cache)
   --------------------------------------------------------------

   Destoy a cache

   Cache must be empty. We simply free slab objects and return `cache` to `cache_cache`.

**/


PUBLIC u8_t virtmem_cache_destroy(struct vmem_cache* cache)
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
	  if ( virtmem_cache_free(slab_cache,slab) != EXIT_SUCCESS )
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
  if ( virtmem_cache_free(&cache_cache,cache) != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    }
 
  return EXIT_SUCCESS;
}



/**

   Function: u32_t virtmem_cache_reap(u8_t flags)
   ----------------------------------------------

   Cache reaping.

   Free memory cached by caches. Only `CACHE_REAPLEN` caches are checked and the one with maximum unused memory is reaped.
   Caches that has just grown are not reaped, as well as those that are flagged unreapables.
   `flags` can take 2 non default values:
   - VIRT_CACHE_FORCEREAP  : just extended caches can also be reaped 
   - VIRT_CACHE_BRUTALREAP : all caches can be reaped
   
   Return the number of relesaed pages or 0 if an error has occured.

**/


PUBLIC u32_t virtmem_cache_reap(u8_t flags)
{
  struct vmem_cache* cache;
  struct vmem_cache* cache_reap;
  struct vmem_slab* slab;
  u32_t pages, max_pages,i;

  if ( LLIST_ISNULL(cache_list) )
    {
      return 0;
    }

  /* Set up run through */
  cache = LLIST_GETHEAD(cache_list);
  cache_reap = NULL;
  i = 1;
  max_pages = 0;

  /* Run through caches list */
  do
    {
      pages = 0;

      /* Check flags */
      if ( ( !(cache->flags & VIRT_CACHE_NOREAP)  || (flags & VIRT_CACHE_BRUTALREAP) ) &&
	   ( !(cache->flags & VIRT_CACHE_JUSTGROWN) || (flags & VIRT_CACHE_FORCEREAP) || (flags & VIRT_CACHE_BRUTALREAP) ) &&
	   !(LLIST_ISNULL(cache->slabs_free)) )
	{
	  /* Run through free slabs list */
	  slab = LLIST_GETHEAD(cache->slabs_free);
	  do
	    {
	      /* Count freeable pages */
	      pages += slab->n_pages;
	      slab = LLIST_NEXT(cache->slabs_free,slab);
	      
	    }while(!LLIST_ISHEAD(cache->slabs_free,slab));

	  /* Update maximum freeable pages and involved cache */
	  if (pages > max_pages)
	    {
	      max_pages = pages;
	      cache_reap = cache;
	    }
	}
      
      /* Update growth flag in any case */
      cache->flags &= ~VIRT_CACHE_JUSTGROWN;
      cache = LLIST_NEXT(cache_list,cache);
      i++;

    }while( (!LLIST_ISHEAD(cache_list,cache))&&(i<VIRT_CACHE_REAPLEN)  );

  /* Shift cache_list head to the last checked cache */
  cache_list = cache;

  /* Do we free pages ? */
  if ( !max_pages )
    {
      return 0;
    }
 
  /* Destroy choosen cache free slabs */
  while(!LLIST_ISNULL(cache_reap->slabs_free))
    {
      slab = LLIST_GETHEAD(cache_reap->slabs_free);

      if ( virtmem_slab_destroy(cache_reap,slab) != EXIT_SUCCESS )
	{
	  return 0;
	}
     
      LLIST_REMOVE(cache_reap->slabs_free,slab);

      /* Return slab to ̀slab_cache` in case of non in-page slab */
       if (cache_reap->size > (CONST_PAGE_SIZE >> VIRT_CACHE_GROWSHIFT))
	{
	  if ( virtmem_cache_free(slab_cache,slab) != EXIT_SUCCESS )
	    {
	      return 0;
	    }
	}

    }

  /* Return number of released pages */
  return max_pages;
  
}


/**

   Function: u8_t virtmem_cache_grow(struct vmem_cache* cache, virtaddr_t addr)
   ----------------------------------------------------------------------------

   Extend a cache.
   Simply call to helpers according to objects size. If cache object size is small, the helper will
   allocate slabs and bufctls in only one virtual page. Otherwise, the helper will allocate slabs and bufctl from
   respective caches.
   
**/


PUBLIC u8_t virtmem_cache_grow(struct vmem_cache* cache, virtaddr_t addr)
{
  u8_t res;

  /* Call helpers according to cache objects size */
  if ( cache->size > (CONST_PAGE_SIZE >> VIRT_CACHE_GROWSHIFT) )
    {
      /* Big objects */
      res = virtmem_cache_grow_big(cache, addr);
    }
  else
    {
      /* Small objects */
      res = virtmem_cache_grow_little(cache, addr);
    }

  /* Set flags */
  if (res == EXIT_SUCCESS)
    {
      cache->flags |= VIRT_CACHE_JUSTGROWN;
    }

  return res;
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
	  if ( virtmem_cache_free(bufctl_cache,bc) != EXIT_SUCCESS )
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

   Function: u8_t virtmem_cache_grow_little(struct vmem_cache* cache, virtaddr_t addr)
   -----------------------------------------------------------------------------------

   Helper to extend cache providing on-page slabs.

   Slab is placed in front of page, then come successively bufctl and its object
   If `addr` is not set to ̀VIRT_CACHE_NOADDR` then cache will grow into the page pointed by `addr`.

**/


PRIVATE u8_t virtmem_cache_grow_little(struct vmem_cache* cache, virtaddr_t addr)
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


/**

   Function: u8_t virtmem_cache_grow_big(struct vmem_cache* cache, virtaddr_t addr)
   --------------------------------------------------------------------------------

   Helper to extend cache providing off-page slabs.

   
   Slabs and bufctl are allocated from their respective caches.
   If `addr` is not set to ̀VIRT_CACHE_NOADDR` then cache will grow into the page pointed by `addr`.

**/


PRIVATE u8_t virtmem_cache_grow_big(struct vmem_cache* cache, virtaddr_t addr)
{
  struct vmem_slab* slab;
  struct vmem_bufctl* bc;
  virtaddr_t page;
  u16_t i, wasted;
  u8_t np;

  /* Number of pages needed */
  np =  (PAGING_ALIGN_SUP(cache->size)) >> CONST_PAGE_SHIFT;

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
  
  
  /* Get a slab from `slab_cache` */
  slab = (struct vmem_slab*)virtmem_cache_alloc(slab_cache, VIRT_CACHE_DEFAULT);
  if ( slab == NULL )
    {
      return EXIT_FAILURE;
    }

  /* Slab initialization */
  slab->count = 0;
  slab->max_objects = (np*CONST_PAGE_SIZE)/cache->size;
  slab->cache = cache;
  slab->n_pages = np;
  slab->start = page;
  LLIST_NULLIFY(slab->free_buf);

  /* Compute alignement */
  if (cache->align)
    {
      /* Remaining space will be used for cache coloring */
      wasted = (np*CONST_PAGE_SIZE)-(slab->max_objects*cache->size);
      if (wasted)
	{
	  slab->start += cache->align_offset;
	  cache->align_offset = (cache->align_offset+cache->align)%wasted;
	}
    }

  /* Link slab to cache */
  LLIST_ADD(cache->slabs_free,slab);

  /* Create bufctls and make them point to the page */
  for(i=0; i<slab->max_objects; i++)
    {
      /* Allocate bufctl from `bufctl_cache` */
      bc = (struct vmem_bufctl*)virtmem_cache_alloc(bufctl_cache, VIRT_CACHE_DEFAULT);
      if ( bc == NULL )
	{
	  return EXIT_FAILURE;
	}
    
      /* Initialize bufctl */
      bc->base = slab->start + i*cache->size;
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
