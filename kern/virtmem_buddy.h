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
#include <wmalloc.h>


/*========================================================================
 * Constantes
 *========================================================================*/

#define VIRT_BUDDY_NOMAP       0
#define VIRT_BUDDY_MAP         1

#define VIRT_BUDDY_STARTSLABS  3
#define VIRT_BUDDY_MINSLABS    2

/*========================================================================
 * Structures
 *========================================================================*/


/* Zone de memoire virtuelle contingue */
struct vmem_area
{
  virtaddr_t base;
  u32_t size;
  u8_t index;
  struct vmem_area* prev;
  struct vmem_area* next;
};


/* DEBUG: WaterMark Allocator */

struct virt_buddy_wm_alloc
{
  u32_t base;
  u32_t size;
  u32_t offset;
};


/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC void  virtmem_buddy_init();
PUBLIC void* virtmem_buddy_alloc(u32_t size, u8_t flags);
PUBLIC void  virtmem_buddy_free(void* addr);


#endif
