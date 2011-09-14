/*
 * Virtmem.c
 * Interface de la memoire virtuelle
 *
 */


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "assert.h"
#include "virtmem_buddy.h"
#include "virtmem_slab.h"
#include "virtmem.h"


/*========================================================================
 * Declaration Private
 *========================================================================*/

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


/*========================================================================
 * Initilisation
 *========================================================================*/

PUBLIC void  virt_init(void)
{
  u8_t i;

  /* Initialise les backends de memoire virtuelle */
  ASSERT_FATAL( virtmem_cache_init()==EXIT_SUCCESS);
  ASSERT_FATAL( virtmem_buddy_init()==EXIT_SUCCESS);

  /* Cree les caches d allocation */
  for(i=0;virt_caches[i].size;i++)
    {
      virt_caches[i].cache = virtmem_cache_create(virt_caches[i].name,virt_caches[i].size,0,0,virt_caches[i].flags,NULL,NULL);
      ASSERT_FATAL( virt_caches[i].cache != NULL );
    }

  return;
}


/*========================================================================
 * Allocation
 *========================================================================*/


PUBLIC void* virt_alloc(u32_t size)
{
  u8_t i;

  /* Cherche un cache de bonne taille */
  for(i=0;virt_caches[i].size;i++)
    {
      if (virt_caches[i].size >= size)
	{
	  return virtmem_cache_alloc(virt_caches[i].cache,VIRT_CACHE_DEFAULT);
	}
    }

  /* Sinon, alloue dans le buddy */
  return virtmem_buddy_alloc(size,VIRT_BUDDY_NOMAP);
}


/*========================================================================
 * Liberation
 *========================================================================*/


PUBLIC u8_t  virt_free(void* addr)
{
  u8_t i;

  /* Tente de liberer dans les caches */
  for(i=0;virt_caches[i].size;i++)
    {
      if (virtmem_cache_free(virt_caches[i].cache, addr) == EXIT_SUCCESS)
	{
	  return EXIT_SUCCESS;
	}
    }

  /* Sinon, tente de liberer dans le buddy */
  return virtmem_buddy_free(addr);
}
