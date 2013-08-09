/**

   vmem_slab.h
   ===========

   Virtual memory slab allocator header

**/

#ifndef VMEM_SLAB_H
#define VMEM_SLAB_H


/**
 
   Includes
   --------

   - define.h
   - types.h
   - const.h

**/

#include <define.h>
#include <types.h>
#include "const.h"


/**
   
   Constant: VMEM_CACHE_NAMELEN
   ----------------------------

   Cache name length

**/
 

#define VMEM_CACHE_NAMELEN     32


/**

   Structure: struct vmem_cache
   ----------------------------

   Describe a cache.

   Members are:

   - name          : Cache name
   - size          : Objects size
   - slab_free     : List of free slabs
   - slab_partial  : List of slabs in used
   - slab_full     : List of slabs which all objects are allocated
   - next          : Next cache in linked list
   - prev          : Previous cache in linked list 

**/


PUBLIC struct vmem_cache
{
  char name[VMEM_CACHE_NAMELEN];
  u16_t size;
  struct slab* slabs_free;
  struct slab* slabs_partial;
  struct slab* slabs_full;
  struct vmem_cache* next;
  struct vmem_cache* prev;
} __attribute__ ((packed));



/**
 
   Prototypes
   ----------

   Give access to caches initialization as well as caches manipulation and allocation/release primitives

**/

PUBLIC u8_t vmem_cache_setup(void);
PUBLIC void* vmem_cache_alloc(struct vmem_cache* cache);
PUBLIC u8_t vmem_cache_free(struct vmem_cache* cache, void* buf);
PUBLIC struct vmem_cache* vmem_cache_create(const char* name, u16_t size);
PUBLIC u8_t vmem_cache_destroy(struct vmem_cache* cache);


#endif
