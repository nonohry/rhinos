/**

   virtmem.h
   =========

   Virtual memory interface header

**/

#ifndef VIRTMEM_H
#define VIRTMEM_H


/**

   Includes
   --------

   - types.h
   - const.h
   - virtmem_slab.h : struct vmem_cache needed

**/

#include <define.h>
#include <types.h>
#include "const.h"
#include "virtmem_slab.h"


/**

   Structure: struct virtmem_caches
   --------------------------------

   Caches wrapper for small object allocation.
   Members are:

   - name  : cache name
   - size  : cache objects size
   - flags : cache flags
   - cache : pointer to cache

**/

PUBLIC struct virtmem_caches
{
  const char* name;
  u32_t size;
  u8_t flags;
  struct vmem_cache* cache;
};


/**
 
   Prototypes
   ----------

   Give access to virtual memory initialization, allocation and release

**/



PUBLIC u8_t  virt_init(void);
PUBLIC void* virt_alloc(u32_t size);
PUBLIC u8_t  virt_free(void* addr);

#endif
