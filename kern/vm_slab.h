/**

   vmem_slab.h
   ===========

   Virtual memory slab allocator header

**/

#ifndef VMEM_SLAB_H
#define VEM_SLAB_H


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
   
   Constants
   ---------

   Lenghts

**/
 

#define VIRT_CACHE_NAMELEN     32
#define VIRT_CACHE_REAPLEN     32


/**

   Constant: VIRT_CACHE_STARTSLABS
   -------------------------------

   Number of arbitrary virtual pages needed for intialization

**/

#define VIRT_CACHE_STARTSLABS  1


/**

   Constants
   ---------

   Reaping flags

**/

#define VIRT_CACHE_DEFAULT     0
#define VIRT_CACHE_NOREAP      1
#define VIRT_CACHE_BRUTALREAP  2
#define VIRT_CACHE_FORCEREAP   4
#define VIRT_CACHE_JUSTGROWN   8


/**

   Constants
   ---------

   Flags

**/

#define VIRT_CACHE_NOMINCHECK  3
#define VIRT_CACHE_NOADDR      0


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
  char name[VIRT_CACHE_NAMELEN];
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

PUBLIC u8_t virtmem_cache_init(void);
PUBLIC void* virtmem_cache_alloc(struct vmem_cache* cache, u8_t flags);
PUBLIC u8_t virtmem_cache_free(struct vmem_cache* cache, void* buf);
PUBLIC struct vmem_cache* virtmem_cache_create(const char* name, u16_t size, u16_t align, u16_t min_slab_free, u8_t flags, void (*ctor)(void*,u32_t), void (*dtor)(void*,u32_t));
PUBLIC u8_t virtmem_cache_destroy(struct vmem_cache* cache);
PUBLIC u32_t virtmem_cache_reap(u8_t flags);
PUBLIC u8_t virtmem_cache_grow(struct vmem_cache* cache, virtaddr_t addr);

#endif
