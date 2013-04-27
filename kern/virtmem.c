/**

   virtmem.c
   =========
   
   Virtual memory interface

**/


/**
 
   Includes
   --------

   - types.h
   - const.h
   - virtmem_buddy.h : Buddy system primitives needed
   - virtmem_slab.h  : Slab allocator primitives needed
   - virtmem.h       : Self header

**/


#include <types.h>
#include "const.h"
#include "virtmem_buddy.h"
#include "virtmem_slab.h"
#include "virtmem.h"


/**

   Private: virt_caches
   --------------------

   Caches for small object allocation (4B to 16KB)

**/

PRIVATE struct virtmem_caches virt_caches[] = 
  {
    {"Virt_Alloc 4B", 4, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 8B", 8, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 16B", 16, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 32B", 32, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 64B", 64, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 128B", 128, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 256B", 256, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 512B", 512, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 1KB", 1024, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 2KB", 2048, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 4KB", 4096, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 8KB", 8192, VIRT_CACHE_DEFAULT},
    {"Virt_Alloc 16KB", 16384, VIRT_CACHE_DEFAULT},
    {NULL, 0, 0}
  };


/**

   Function: u8_t virt_init(void)
   ------------------------------

   Virtual memory interface initialization

   Initialize virtual memory subsystem (buddy & slab allocator)
   Create caches for small objects.

**/

PUBLIC u8_t virt_init(void)
{
  u8_t i;

  /* Initialize subsystems */
   if ( virtmem_cache_init() != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    }

  if ( virtmem_buddy_init() != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    }


  /* Create small objects caches */
  for(i=0;virt_caches[i].size;i++)
    {
      virt_caches[i].cache = virtmem_cache_create(virt_caches[i].name,virt_caches[i].size,0,0,virt_caches[i].flags,NULL,NULL);
      if ( virt_caches[i].cache == NULL )
	{
	  return EXIT_FAILURE;
	}
    }

  return EXIT_SUCCESS;
}


/**

   Function: void* virt_alloc(u32_t size)
   --------------------------------------

   Allocate a virtual memory of power of two rounded `size`.
   Try to allocate in small objet caches. If `size` is too large, allocate in buddy.

   Return start address of virtual memory block if alloacion succeed, NULL otherwise.

**/

PUBLIC void* virt_alloc(u32_t size)
{
  u8_t i;

  /* Look for a fitting cache */
  for(i=0;virt_caches[i].size;i++)
    {
      if (virt_caches[i].size >= size)
	{
	  return virtmem_cache_alloc(virt_caches[i].cache,VIRT_CACHE_DEFAULT);
	}
    }


  /* Cache not found, try in buddy */
  return virtmem_buddy_alloc(size,VIRT_BUDDY_NOMAP);
}


/**

   Function: u8_t virt_free(void* addr)
   ------------------------------------

   Free memory block starting by `addr`.
   Try to release memory in caches then in buddy if it fails.

**/


PUBLIC u8_t virt_free(void* addr)
{
  u8_t i;

  /* Try release in caches */
  for(i=0;virt_caches[i].size;i++)
    {
      if (virtmem_cache_free(virt_caches[i].cache, addr) == EXIT_SUCCESS)
	{
	  return EXIT_SUCCESS;
	}
    }

  /* Try in buddy otherwise */
  return virtmem_buddy_free(addr);
}
