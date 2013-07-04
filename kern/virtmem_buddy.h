/**

   virtmem_buddy.h
   ===============

   Virtual buddy header


**/

#ifndef VIRTMEM_BUDDY_H
#define VIRTMEM_BUDDY_H


/**
 
   Includes
   --------

   - types.h
   - const.h

**/

#include <define.h>
#include <types.h>
#include "const.h"


/**
 
   Constants: Flags
   ----------------

**/

#define VIRT_BUDDY_NOMAP       0x0
#define VIRT_BUDDY_MAP         0x1
#define VIRT_BUDDY_NOMINCHECK  0x2


/**

   Constants: Number of "manual" pages  used in initialization
   ------------------------------------------------------------

**/

#define VIRT_BUDDY_STARTSLABS  3
#define VIRT_BUDDY_MINSLABS    2


/**

   Constant: VIRT_BUDDY_MAX
   ------------------------

   Number of levels in buddy.

**/

#define VIRT_BUDDY_MAX         21


/**

   Macro: VIRT_BUDDY_POOLLIMIT
   ---------------------------

   Starting address of "manual" initialization pages

**/

#define VIRT_BUDDY_POOLLIMIT   (PAGING_ALIGN_SUP( CONST_PAGE_NODE_POOL_ADDR+((start_mem_total) >> CONST_PAGE_SHIFT)*sizeof(struct ppage_desc) ))


/**
 
   Structure: struct vmem_area
   ---------------------------

   Buddy item describing a virtualmemory area.
   Members are:

   - base  : base address
   - size  : size in bytes
   - index : buddy level
   - prev  : previous area in buddy level
   - next  : next area in buddy level

**/




PUBLIC struct vmem_area
{
  virtaddr_t base;
  u32_t size;
  u8_t index;
  struct vmem_area* prev;
  struct vmem_area* next;
};


/**
 
   Prototypes
   ----------

   Buddy initialization and allocation/release primitives

**/

PUBLIC u8_t  virtmem_buddy_init();
PUBLIC void* virtmem_buddy_alloc(u32_t size, u8_t flags);
PUBLIC u8_t  virtmem_buddy_free(void* addr);


#endif
