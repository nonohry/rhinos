/*
 * Virtmem_buddy.c
 * Allocateur de memoire virtuelle (>=4096)
 *
 */


#include <types.h>
#include <wmalloc.h>
#include "start.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem_buddy.h"


/***********************
 * Declarations PRIVATE
 ***********************/

PRIVATE struct virt_buddy_wm_alloc virt_wm;


/*********************************
 * Initialisation de l'allocateur
 *********************************/

PUBLIC void  virtmem_buddy_init()
{
  /* DEBUG: Initialise le WaterMark */
  WMALLOC_INIT(virt_wm,PAGING_ALIGN_SUP(PHYS_PAGE_NODE_POOL_ADDR+((bootinfo->mem_size) >> PHYS_PAGE_SHIFT)*sizeof(struct ppage_node)),(1<<31));
  return;
}


/**************
 * Allocation
 **************/

PUBLIC void* virtmem_buddy_alloc(u32_t size)
{
  /* DEBUG: Allcoation via WaterMark */
  return (void*)WMALLOC_ALLOC(virt_wm,size);
}


/*************
 * Liberation
 *************/

PUBLIC void  virtmem_buddy_free(void* addr)
{
  /* DEBUG: Liberation via WaterMark */
  WMALLOC_FREE(virt_wm,addr);
  return;
}
