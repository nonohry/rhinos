/*
 * Virtmem_buddy.h
 * Header de virtmem_buddy.c
 *
 */

#ifndef VIRTMEM_BUDDY_H
#define VIRTMEM_BUDDY_H


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "const.h"


/*========================================================================
 * Constantes
 *========================================================================*/

#define VIRT_BUDDY_NOMAP       0x0
#define VIRT_BUDDY_MAP         0x1
#define VIRT_BUDDY_NOMINCHECK  0x2

#define VIRT_BUDDY_STARTSLABS  3
#define VIRT_BUDDY_MINSLABS    2

#define VIRT_BUDDY_MAX         21
#define VIRT_BUDDY_HIGHTMEM    (1<<30)     /* (unsigned)-1 = 4G */


/*========================================================================
 * Macros
 *========================================================================*/

#define VIRT_BUDDY_POOLLIMIT   (PAGING_ALIGN_SUP( CONST_PAGE_NODE_POOL_ADDR+((bootinfo->mem_total) >> CONST_PAGE_SHIFT)*sizeof(struct ppage_desc) ))


/*========================================================================
 * Structures
 *========================================================================*/


/* Zone de memoire virtuelle contingue */

PUBLIC struct vmem_area
{
  virtaddr_t base;
  u32_t size;
  u8_t index;
  struct vmem_area* prev;
  struct vmem_area* next;
};


/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC u8_t  virtmem_buddy_init();
PUBLIC void* virtmem_buddy_alloc(u32_t size, u8_t flags);
PUBLIC u8_t  virtmem_buddy_free(void* addr);


#endif
