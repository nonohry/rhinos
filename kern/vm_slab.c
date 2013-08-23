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
   - vm_pool.h      : virtual page allocation & release
   - vm_slab.h      : self header

**/

#include <define.h>
#include <types.h>
#include <llist.h>
#include <arch_const.h>
#include "vm_pool.h"
#include "vm_slab.h"



/**

   Structure: struct bufctl
   -----------------------------

   Describe a bufctl.

   A bufctl is just an helper structure to point to a virtual memory area. 
   Members are:

   - base  : Virtual memory area base address
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
  struct vm_cache* cache;
  struct slab* next;
  struct slab* prev;
} __attribute__ ((packed));




/**

   Privates
   --------

   Helper to make cache grow

**/

PRIVATE u8_t vm_cache_grow(struct vm_cache* cache);



/**

   Global: Caches list
   -------------------

**/

struct vm_cache* cache_list;


/**

   Global: cache_cache
   --------------------

   Cache of cache objects

**/

struct vm_cache cache_cache =
  {
  name: "cache_cache",
  size: sizeof(struct vm_cache),
  slabs_free: NULL,
  slabs_partial: NULL,
  slabs_full: NULL,
  next: NULL,
  prev: NULL
  };



/**

   Function: u8_t vm_cache_setup(void)
   ----------------------------------------

   Slab allocator initialization.
   
   Just set `cache_cache` as head list

**/


PUBLIC u8_t vm_cache_setup(void)
{
  /* Caches list initialization */
  cache_list = &cache_cache;
  LLIST_SETHEAD(cache_list);

  return EXIT_SUCCESS;
}


/**

   Function: struct vm_cache* vm_cache_create(const char* name, u16_t size )
   -------------------------------------------------------------------------

   Cache creation.

   Simply allocate a cache object from ̀cache_cache` and fill the structure fields with arguments.
   Return a pointer to the cache object or NULL if creation fails.

**/


PUBLIC struct vm_cache* vm_cache_create(const char* name, u16_t size)
{
  u8_t i;
  struct vm_cache* cache;

  /* Cache allocation */
  cache = (struct vm_cache*)vm_cache_alloc(&cache_cache);
  if ( cache == NULL )
    {
      return NULL;
    }

  /* Name copy */
  i=0;
  while( (name[i]!=0)&&(i<VM_CACHE_NAMELEN-1) )
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

   Function: void* vm_cache_alloc(struct vm_cache* cache)
   --------------------------------------------------------

   Allocation from a cache.

   First, if cache is full, it is extended.
   Next, we get a slab then a bufctl  and update slab objects list and cache slabs lists.
   Return base address pointed by bufctl or NULL if allocation fails

  
**/


PUBLIC void* vm_cache_alloc(struct vm_cache* cache)
{
  struct slab* list;
  struct slab* slab;
  struct bufctl* bufctl;
  
  /* Extend cache if needed */
  if ( (LLIST_ISNULL(cache->slabs_free)) && (LLIST_ISNULL(cache->slabs_partial)) )
    {
      if ( vm_cache_grow(cache) != EXIT_SUCCESS )
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
      if (slab->free_objects)
	{
	  LLIST_ADD(cache->slabs_partial,slab);
	}
      else
	{
	  LLIST_ADD(cache->slabs_full,slab);
	}
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

   Function: u8_t vm_cache_free(struct vm_cache* cache, void* buf)
   ---------------------------------------------------------------

   Release a object pointed by `buf` and return it to `cache`.

   Only on page slab are manipulated so bufctl and buf are contiguous, and we retrieve bufctl using
   bufctl = buf - sizeof(bufctl)

   Then we retrieve slab just by aligning buf or bufctl on page: slab = bufctl >> ARCH_CONST_PAGE_SHIFT
   Now we can update slab's counter and cache slabs lists.

**/


PUBLIC u8_t vm_cache_free(struct vm_cache* cache, void* buf)
{
  struct bufctl* bc;
  struct slab* slab;

  /* Get bufctl */
  bc = (struct bufctl*)((virtaddr_t)buf - sizeof(struct bufctl));
 
  /* Get slab */
  slab = (struct slab*)( ((virtaddr_t)bc >> ARCH_CONST_PAGE_SHIFT) << ARCH_CONST_PAGE_SHIFT );
  

  /* Right cache ? */
  if (slab->cache != cache)
    {
      return EXIT_FAILURE;
    }

  /* Update slab counter and free list */
  slab->free_objects++;
  LLIST_ADD(slab->free_bufctls,bc);

  /* Update cache slabs lists if needed  */
  if ( slab->free_objects-1 )
    {
      /* Move from partial to free if needed */
      if (slab->free_objects == (ARCH_CONST_PAGE_SIZE - sizeof(struct slab))/(cache->size+sizeof(struct bufctl)) )
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

   Function: u8_t vm_cache_destroy(struct vm_cache* cache)
   --------------------------------------------------------------

   Destoy a cache

   Cache must be empty, ie partial and full list must be empty.
   Then, all on-page slabs are released by simply releasing their virtual page.
   At last, `cache` is returned to `cache_cache`.

**/


PUBLIC u8_t vm_cache_destroy(struct vm_cache* cache)
{
  u8_t i;
  struct slab* slab;

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
      /* Detroy link */
      LLIST_REMOVE(cache->slabs_free,slab);
      /* Because of on-page slabs, we destroy slab by freeing its virtual page */
      if ( vm_pool_free((virtaddr_t)slab) != EXIT_SUCCESS )
	{
	  return EXIT_FAILURE;
	}
    }
  
  /* Zeroing structure */
  for(i=0;i<VM_CACHE_NAMELEN;i++)
    {
      cache->name[i] = 0;
    }
  cache->size = 0;
  LLIST_REMOVE(cache_list,cache);

  /* Return to `cache_cache` */
  if ( vm_cache_free(&cache_cache,cache) != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    }
 
  return EXIT_SUCCESS;
}


/**

   Function: u8_t vm_cache_grow(struct vm_cache* cache)
   ----------------------------------------------------

   Helper to extend cache providing on-page slabs.

   A virtual page is get from virtual pool.
   Slab is placed in front of this page, then come successively bufctl and its object.

   The slab is then linked in `cache` free slabs list.
 

**/


PRIVATE u8_t vm_cache_grow(struct vm_cache* cache)
{ 
  struct slab* slab;
  virtaddr_t buf;
  virtaddr_t page;
  size_t size;

  /* Allocate a virtual page from pool */
  page = vm_pool_alloc();
  if ( page == VM_POOL_ERROR )
    {
      return EXIT_FAILURE;
    }
   
  /* Place slab in front of page */
  slab = (struct slab*)page;
  
  /* Initialize it */
  slab->free_objects = (ARCH_CONST_PAGE_SIZE - sizeof(struct slab))/(cache->size+sizeof(struct bufctl));
  
  slab->cache = cache;
  LLIST_NULLIFY(slab->free_bufctls);

  /* Link new slab to cache free slab list */
  LLIST_ADD(cache->slabs_free,slab);

  /* Size of 2-uplet bufctl + buffer */
  size = cache->size + sizeof(struct bufctl);
  
  /* Create bufctl and objects into the page */
  for(buf = page+sizeof(struct slab);
      buf < (page+ARCH_CONST_PAGE_SIZE-size);
      buf += size)
    {
      struct bufctl* bc = (struct bufctl*)buf;

      /* Initialize bufctl */
      bc->base = buf+sizeof(struct bufctl);
    
      /* Add bufctl to slab free list */
      LLIST_ADD(slab->free_bufctls,bc);
    }

  return EXIT_SUCCESS;
}
