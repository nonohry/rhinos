/*
 * Virtmem_buddy.c
 * Allocateur de memoire virtuelle (gros objets)
 *
 */


#include <types.h>
#include <llist.h>
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
PRIVATE struct vmem_area* buddy_used;

PRIVATE void virtmem_print_buddy_used(void);

/*========================================================================
 * Initialisation de l'allocateur
 *========================================================================*/

PUBLIC void  virtmem_buddy_init()
{
  struct vmem_area* area;
  virtaddr_t vaddr_init;
  physaddr_t paddr_init;
  u32_t i;

  /* Initialise les listes */
  LLIST_NULLIFY(buddy_used);

  /* Cree le cache des noeuds du buddy */
  area_cache = virtmem_cache_create("area_cache",sizeof(struct vmem_area),0,VIRT_BUDDY_MINSLABS,VIRT_CACHE_NOREAP,NULL,NULL);

  /* Initialisation manuelle du cache */
  for(i=0;i<VIRT_BUDDY_STARTSLABS;i++)
    {
       /* Cree une adresse virtuelle mappee pour les initialisations */
      vaddr_init = (i+VIRT_CACHE_STARTSLABS)*PAGING_PAGE_SIZE + PAGING_ALIGN_SUP( PHYS_PAGE_NODE_POOL_ADDR+((bootinfo->mem_total) >> PHYS_PAGE_SHIFT)*sizeof(struct ppage_desc) );
      paddr_init = (physaddr_t)phys_alloc(PAGING_PAGE_SIZE);
      paging_map(vaddr_init, paddr_init, TRUE);
      /* Fait grossir cache_cache dans cette page */
      if ( virtmem_cache_grow(area_cache,vaddr_init) == EXIT_FAILURE )
	{
	  bochs_print("Cannot initialize virtual buddy allocator !\n");
	}
    }

  /* Entre les pages des initialisations manuelles dans buddy_used */
  for(i=0;i<VIRT_CACHE_STARTSLABS+VIRT_BUDDY_STARTSLABS;i++)
    {
      area=(struct vmem_area*)virtmem_cache_alloc(area_cache);
      area->base = i*PAGING_PAGE_SIZE + PAGING_ALIGN_SUP( PHYS_PAGE_NODE_POOL_ADDR+((bootinfo->mem_total) >> PHYS_PAGE_SHIFT)*sizeof(struct ppage_desc) );
      area->size = PAGING_PAGE_SIZE;
      area->index = 0;
      LLIST_ADD(buddy_used,area);
    }



  /* DEBUG: Initialise le WaterMark */
  WMALLOC_INIT(virt_wm,20480+PAGING_ALIGN_SUP(PHYS_PAGE_NODE_POOL_ADDR+((bootinfo->mem_total) >> PHYS_PAGE_SHIFT)*sizeof(struct ppage_desc)),(1<<31));

  virtmem_print_slaballoc();
  virtmem_print_buddy_used();

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


/*========================================================================
 * DEBUG: print buddy_used
 *========================================================================*/


PRIVATE void virtmem_print_buddy_used(void)
{
    struct vmem_area* area;
    if (LLIST_ISNULL(buddy_used))
      {
	bochs_print("~");
      }
    else
      {
	area = LLIST_GETHEAD(buddy_used);
	do
	  {
	    bochs_print("[0x%x (0x%x - %d)] ",area->base,area->size,area->index);
	    area=LLIST_NEXT(buddy_used,area);
	  }while(!LLIST_ISHEAD(buddy_used,area));
      }

    bochs_print("\n");

    return;
}
