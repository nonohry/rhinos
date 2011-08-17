/*
 * Virtmem_buddy.c
 * Allocateur de memoire virtuelle (gros objets)
 *
 */


#include <types.h>
#include <wmalloc.h>
#include "klib.h"
#include "start.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem_slab.h"
#include "virtmem_buddy.h"


/*========================================================================
 * Declarations PRIVATE
 *========================================================================*/

/* DEBUG: WaterMArk Allocator */
PRIVATE struct virt_buddy_wm_alloc virt_wm;
PRIVATE struct vmem_cache* area_cache;


/*========================================================================
 * Initialisation de l'allocateur
 *========================================================================*/

PUBLIC void  virtmem_buddy_init()
{
  virtaddr_t vaddr_init;
  physaddr_t paddr_init;
  u32_t i;

  /* Cree le cache des noeuds du buddy */
  area_cache = virtmem_cache_create("area_cache",sizeof(struct vmem_area),0,VIRT_BUDDY_MINSLABS,VIRT_CACHE_NOREAP,NULL,NULL);

  /* Initialisation manuelle du cache */
  for(i=0;i<VIRT_BUDDY_MINSLABS;i++)
    {
       /* Cree une adresse virtuelle mappee pour les initialisations */
      vaddr_init = (i+1)*PAGING_PAGE_SIZE + PAGING_ALIGN_SUP( PHYS_PAGE_NODE_POOL_ADDR+((bootinfo->mem_total) >> PHYS_PAGE_SHIFT)*sizeof(struct ppage_desc) );
      paddr_init = (physaddr_t)phys_alloc(PAGING_PAGE_SIZE);
      paging_map(vaddr_init, paddr_init, TRUE);
      /* Fait grossir cache_cache dans cette page */
      if ( virtmem_cache_grow(area_cache,vaddr_init) == EXIT_FAILURE )
	{
	  bochs_print("Cannot initialize virtual buddy allocator !\n");
	}
    }


  /* DEBUG: Initialise le WaterMark */
  WMALLOC_INIT(virt_wm,20480+PAGING_ALIGN_SUP(PHYS_PAGE_NODE_POOL_ADDR+((bootinfo->mem_total) >> PHYS_PAGE_SHIFT)*sizeof(struct ppage_desc)),(1<<31));

  virtmem_cache_alloc(area_cache);
  virtmem_print_slaballoc();


  return;
}


/*========================================================================
 * Allocation
 *========================================================================*/

PUBLIC void* virtmem_buddy_alloc(u32_t size, u8_t flags)
{
  virtaddr_t vaddr,vaddr2;

  /* DEBUG: Allocation via WaterMark */
  vaddr = vaddr2 = (virtaddr_t)WMALLOC_ALLOC(virt_wm,size);

  /* DEBUG: Mappage sur pages physiques */
  if (flags & VIRT_BUDDY_MAP)
    {
      u32_t i,n;
  
      /* DEBUG: Nombre de pages */
      n = ((size&0xFFFFF000) == size)?(size >> PAGING_PAGE_SHIFT) :((size >> PAGING_PAGE_SHIFT)+1);
      
      /* DEBUG: Mapping */
      for(i=0;i<n;i++)
	{
	  physaddr_t paddr;

	  paddr = (u32_t)phys_alloc(PAGING_PAGE_SIZE);
	  paging_map(vaddr2,paddr,TRUE);
	  vaddr2+=PAGING_PAGE_SIZE;

	}
      
    }

  return (void*)vaddr;

}


/*========================================================================
 * Liberation
 *========================================================================*/

PUBLIC void  virtmem_buddy_free(void* addr)
{
  /* DEBUG: Liberation via WaterMark */
  WMALLOC_FREE(virt_wm,addr);
  bochs_print("Liberation de 0x%x (buddy)\n",(u32_t)addr);

  return;
}
