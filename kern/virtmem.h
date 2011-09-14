/*
 * Virtmem.h
 * Header de virtmem.c
 *
 */

#ifndef VIRTMEM_H
#define VIRTMEM_H


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "virtmem_slab.h"


/*========================================================================
 * Structures
 *========================================================================*/

struct virtmem_caches
{
  const char* name;
  u32_t size;
  u8_t flags;
  struct vmem_cache* cache;
};


/*========================================================================
 * Prototypes
 *========================================================================*/


PUBLIC void  virt_init(void);
PUBLIC void* virt_alloc(u32_t size);
PUBLIC u8_t  virt_free(void* addr);

#endif
