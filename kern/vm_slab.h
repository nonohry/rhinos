/**

   vm_slab.h
   =========

   Virtual memory slab allocator header

**/

#ifndef VM_SLAB_H
#define VM_SLAB_H


/**
 
   Includes
   --------

   - define.h
   - types.h
 
**/

#include <define.h>
#include <types.h>


/**
   
   Constant: VMEM_CACHE_NAMELEN
   ----------------------------

   Cache name length

**/
 

#define VM_CACHE_NAMELEN     32


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


PUBLIC struct vm_cache
{
  char name[VM_CACHE_NAMELEN];
  u16_t size;
  struct slab* slabs_free;
  struct slab* slabs_partial;
  struct slab* slabs_full;
  struct vm_cache* next;
  struct vm_cache* prev;
} __attribute__ ((packed));



/**
 
   Prototypes
   ----------

   Give access to caches initialization as well as caches manipulation and allocation/release primitives

**/

PUBLIC u8_t vm_cache_setup(void);
PUBLIC void* vm_cache_alloc(struct vm_cache* cache);
PUBLIC u8_t vm_cache_free(struct vm_cache* cache, void* buf);
PUBLIC struct vm_cache* vm_cache_create(const char* name, u16_t size);
PUBLIC u8_t vm_cache_destroy(struct vm_cache* cache);


#endif
