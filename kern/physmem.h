/**

   physmem.h
   =========

   Header of physical memory manager
   Define useful constants and macros 

**/

#ifndef PHYSMEM_C
#define PHYSMEM_C


/**

   Includes
   --------

   - define.h
   - types.h
   - const.h
   - virtmem_slab.h  : struct vmem_cache &  struct vmem_bufctl needed
   - virtmem_buddy.h : struct vmem_area needed

**/   

#include <define.h>
#include <types.h>
#include "const.h"
#include "virtmem_slab.h"
#include "virtmem_buddy.h"


/**

   Constants: Physical memory relatives
   ------------------------------------


**/

#define PHYS_PAGE_MAX_BUDDY      21        /* 32 - 12 + 1 = 21 */
#define PHYS_POOL_AREA_START     CONST_PAGE_NODE_POOL_ADDR

#define PHYS_UNMAP_NONE       0
#define PHYS_UNMAP_UNMAP      1
#define PHYS_UNMAP_FREE       2

#define PHYS_UNMAP_DEFAULT    0
#define PHYS_UNMAP_NOFREE     1


/**

   Macro: PHYS_ALIGN_INF(__addr)
   -----------------------------

   Align `__addr` on  lower page size boundary

**/


#define PHYS_ALIGN_INF(__addr)					\
  ( ((__addr) >> CONST_PAGE_SHIFT) << CONST_PAGE_SHIFT )


/**

   Macro: PHYS_ALIGN_SUP(__addr)
   -----------------------------

   Align `__addr` on upper page size boundary

**/

#define PHYS_ALIGN_SUP(__addr)						\
  ( (((__addr)&0xFFFFF000) == (__addr))?((__addr) >> CONST_PAGE_SHIFT) << CONST_PAGE_SHIFT:(((__addr) >> CONST_PAGE_SHIFT)+1) << CONST_PAGE_SHIFT )


/**

   Macro: PHYS_GET_DESC(__addr)
   ----------------------------

   Get the page descriptor bounded to `__addr`. 
   Every physical page address has its own page descriptor 
   located at `PHYS_POOL_AREA_START + ((__addr) >> CONST_PAGE_SHIFT)*sizeof(struct ppage_desc)`

**/

#define PHYS_GET_DESC(__addr)						\
  ( (struct ppage_desc*)(PHYS_POOL_AREA_START + ((__addr) >> CONST_PAGE_SHIFT)*sizeof(struct ppage_desc)) )


/**

   Macro: PHYS_PDESC_ISNULL(__pdescaddr)
   -------------------------------------

   Tell if the page descriptor is null. 
   Null is defined here as equal to PHYS_POOL_AREA_START.

**/


#define PHYS_PDESC_ISNULL(__pdescaddr)		\
  ( ((physaddr_t)(__pdescaddr)) == PHYS_POOL_AREA_START )



/**

   Macro: PHYS_NULLIFY_DESC(__desc)
   --------------------------------

   Nullify all the fields of the page descriptor `__desc`

**/
   

#define PHYS_NULLIFY_DESC(__desc)			\
  {							\
    (__desc)->start = 0;				\
    (__desc)->size = 0;					\
    (__desc)->maps = 0;					\
    (__desc)->index = 0;				\
    (__desc)->cache = NULL;				\
    (__desc)->bufctl = NULL;				\
    (__desc)->area = NULL;				\
    (__desc)->prev = NULL;				\
    (__desc)->next = NULL;				\
  }


/**
   Structure: struct ppage_desc 
   ----------------------------

   Contains the description of a physical memory area and is used as item in the buddy system.
   Each CONST_PAGE_SIZE page ownes a page descriptor but a page descriptor can represent several
   contiguous pages area.
   All the page descriptors are linked in a double linked list.
   Members are:
   
   - start   : area base address
   - size    : area size in bytes
   - maps    : number of virtual mappings
   - index   : level in the buddy system
   - cache   : list of struct vmem_cache in the page
   - bufctl  : list of struct vmem_bufctl in the page
   - area    : list of struct vmem_area in the page
   - prev    : previous item in the linked list
   - next    : next item in the linked list

**/

PUBLIC struct ppage_desc 
{
  physaddr_t start;
  u32_t size;
  u16_t maps;
  u8_t  index;
  struct vmem_cache* cache;
  struct vmem_bufctl* bufctl;
  struct vmem_area* area;
  struct ppage_desc* prev;
  struct ppage_desc* next;
}__attribute__((packed));


/**

   Prototypes
   ----------

   Give access to intialization, allocation & freeing and mapping/unmapping operations.

**/

PUBLIC u8_t phys_init(void);
PUBLIC void* phys_alloc(u32_t size);
PUBLIC u8_t phys_free(void* addr);
PUBLIC u8_t phys_map(physaddr_t addr);
PUBLIC u8_t phys_unmap(physaddr_t addr, u8_t flag);

#endif
