/**

   virtmem_slab.h
   ==============

   Virtual memory slab allocator header

**/

#ifndef VIRTMEM_SLAB_H
#define VIRTMEM_SLAB_H


/**
 
   Includes
   --------

   - types.h
   - const.h

**/

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

   Constant: VIRT_CACHE_GROWSHIFT
   ------------------------------

   Binary shift to determine the need for an on-page or off-page slab

**/

#define VIRT_CACHE_GROWSHIFT   3   /* 2^3 = 8 */


/**

   Structure: struct vmem_bufctl
   -----------------------------

   Describe a bufctl.

   A bufctl is just an helper structure to point to a virtual memory area. 
   Members are:

   - base  : Virtual memory area base address
   - slab  : Parent slab back pointer
   - next  : Next bufctl in linked list
   - prev  : Previous bufctl in linked list

**/


PUBLIC struct vmem_bufctl
{
  virtaddr_t base;
  struct vmem_slab* slab;
  struct vmem_bufctl* next;
  struct vmem_bufctl* prev;
} __attribute__ ((packed));




/**

   Structure: struct vmem_slab
   ---------------------------

   Describe a slab.

   A slab is basically  a free bufctl container.
   Membres are:

   - count       : Number of free bufctl (or free objects, or free virtualmemory area)  available
   - max_objects : Maximum free objects a slab can contain
   - n_pages     : Number of pages needed to store an object
   - start       : First bufctl virtual address
   - free_buf    : List of free bufctl
   - cache       : Parent cache back pointer
   - next        : Next slab in linked list
   - prev        : Previous slab in linked list

**/

PUBLIC struct vmem_slab
{
  u16_t count;
  u16_t max_objects;
  u8_t  n_pages;
  virtaddr_t start;
  struct vmem_bufctl* free_buf;
  struct vmem_cache* cache;
  struct vmem_slab* next;
  struct vmem_slab* prev;
} __attribute__ ((packed));



/**

   Structure: struct vmem_cache
   ----------------------------

   Describe a cache.

   Members are:

   - name          : Cache name
   - size          : Objects size
   - align         : Cache alignement
   - align_offset  : Effective offset in bufctl position in slabs (computed according to `align`)
   - min_slab_free : Minimum of free slab available in cache. Used to avoid recursivity with virtual buddy
   - flags         : Cache flags, controling reaping
   - constructor   : Function applied during object creation
   - destructor    : Function applied on objects during a slab destruction
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
  u16_t align;
  u16_t align_offset;
  u16_t min_slab_free;
  u8_t flags;
  void (*constructor)(void*,u32_t);
  void (*destructor)(void*,u32_t);
  struct vmem_slab* slabs_free;
  struct vmem_slab* slabs_partial;
  struct vmem_slab* slabs_full;
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
