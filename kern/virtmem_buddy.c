/*
 * Virtmem_buddy.c
 * Allocateur de memoire virtuelle (gros objets)
 *
 */


#include <types.h>
#include <llist.h>
#include "klib.h"
#include "start.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem_slab.h"
#include "virtmem_buddy.h"


/*========================================================================
 * Declarations PRIVATE
 *========================================================================*/

PRIVATE struct vmem_cache* area_cache;

PRIVATE struct vmem_area* buddy_free[VIRT_BUDDY_MAX];
PRIVATE struct vmem_area* buddy_used;

PRIVATE struct vmem_area* virtmem_buddy_alloc_area(u32_t size, u8_t flags);
PRIVATE void virtmem_buddy_free_area(struct vmem_area* area);
PRIVATE void virtmem_buddy_init_area(u32_t base, u32_t size);
PRIVATE void virtmem_buddy_map_area(struct vmem_area* area);
PRIVATE void virtmem_print_buddy_used(void);
PRIVATE void virtmem_print_buddy_free(void);


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
  for(i=0;i<VIRT_BUDDY_MAX;i++)
    {
      LLIST_NULLIFY(buddy_free[i]);
    }
  LLIST_NULLIFY(buddy_used);

  /* Cree le cache des noeuds du buddy */
  area_cache = virtmem_cache_create("area_cache",sizeof(struct vmem_area),0,VIRT_BUDDY_MINSLABS,VIRT_CACHE_NOREAP,NULL,NULL);
  if (area_cache==NULL)
    {
      bochs_print("Cannot initialise virtual buddy\n");
      return;
    }

  /* Initialisation manuelle du cache */
  for(i=0;i<VIRT_BUDDY_STARTSLABS;i++)
    {
       /* Cree une adresse virtuelle mappee pour les initialisations */
      vaddr_init = (i+VIRT_CACHE_STARTSLABS)*PAGING_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT;
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
      area=(struct vmem_area*)virtmem_cache_alloc(area_cache, VIRT_CACHE_DEFAULT);
      if (area==NULL)
	{
	  bochs_print("Cannot initialise virtual buddy\n");
	  return;
	}
      area->base = i*PAGING_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT;
      area->size = PAGING_PAGE_SIZE;
      area->index = 0;
      LLIST_ADD(buddy_used,area);
    }

  /* Initialise la memoire virtuelle disponible */
  virtmem_buddy_init_area( (VIRT_CACHE_STARTSLABS+VIRT_BUDDY_STARTSLABS)*PAGING_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT, 
			   VIRT_BUDDY_HIGHTMEM - ((VIRT_CACHE_STARTSLABS+VIRT_BUDDY_STARTSLABS)*PAGING_PAGE_SIZE+VIRT_BUDDY_POOLLIMIT) );


  /* DEBUG: tests */
  
  struct vmem_area* tab[300];
  for(i=0;i<300;i++)
    {
      tab[i] = (struct vmem_area*)virtmem_buddy_alloc(64000,VIRT_BUDDY_MAP);
    }
  virtmem_print_buddy_free();
  virtmem_print_buddy_used();
  virtmem_print_slaballoc();

  for(i=0;i<300;i++)
    {
      virtmem_buddy_free(tab[i]);
    }
  virtmem_print_buddy_free();
  virtmem_print_buddy_used();
  virtmem_print_slaballoc();

  return;
}


/*========================================================================
 * Allocation
 *========================================================================*/

PUBLIC void* virtmem_buddy_alloc(u32_t size, u8_t flags)
{
  struct vmem_area* area;

  /* Taille minimale */
  if (size < PAGING_PAGE_SIZE )
    {
      size = PAGING_PAGE_SIZE;
    }

  /* Allocation dans le buddy */
  area = virtmem_buddy_alloc_area(size, flags);
  if (area == NULL)
    {
      return NULL;
    }

  /* Mapping physique selon flags */
  if ( flags & VIRT_BUDDY_MAP )
    {
      virtmem_buddy_map_area(area);
    }

  /* Retourne l adresse de base */
  return (void*)area->base;

}


/*========================================================================
 * Liberation
 *========================================================================*/

PUBLIC void  virtmem_buddy_free(void* addr)
{
  
  struct vmem_area* area;
  virtaddr_t va;

  /* Cherche la vmem_area associee a l adresse */
  if (!LLIST_ISNULL(buddy_used))
    {
      area = LLIST_GETHEAD(buddy_used);
      do
	{
	  /* Sort si l adresse est trouvee */
	  if (area->base == (virtaddr_t)addr)
	    {
	      break;
	    }
	  /* Suivant */
	  area = LLIST_NEXT(buddy_used, area);
	  
	}while(!LLIST_ISHEAD(buddy_used, area));
      
      /* Demap physiquement (aucun effet si non mappee) */
      for(va=area->base;va<(area->base+area->size);va+=PAGING_PAGE_SIZE)
	{
	  paging_unmap(va);
	}

      /* Reintegre l area dans le buddy si trouvee */
      if (area->base == (virtaddr_t)addr)
	{
	  virtmem_buddy_free_area(area);
	}

    }

  return;
}



/*========================================================================
 * Allocation (fonction reelle)
 *========================================================================*/

PRIVATE struct vmem_area* virtmem_buddy_alloc_area(u32_t size, u8_t flags)
{

  u32_t i,j;
  int ind;
  struct vmem_area* area;

  /* trouve la puissance de 2 superieure */
  size = size - 1;
  size = size | (size >> 1);
  size = size | (size >> 2);
  size = size | (size >> 4);
  size = size | (size >> 8);
  size = size | (size >> 16);
  size = size + 1;
  
  /* En deduit l indice */
  ind = msb(size) - PAGING_PAGE_SHIFT;
  
  /* Si ppage_free[ind] est NULL, on cherche un niveau superieur disponible */
  for(i=ind;LLIST_ISNULL(buddy_free[i])&&(i<VIRT_BUDDY_MAX);i++)
    {}

  if (i>=VIRT_BUDDY_MAX)
    {
      bochs_print("Can't allocate %d virtual bytes !\n",size);
      return NULL;
    }
  
  /* Scinde "recursivement" les niveaux superieurs */
  for(j=i;j>ind;j--)
    {
      struct vmem_area* ar1;
     
      area = LLIST_GETHEAD(buddy_free[j]);
      LLIST_REMOVE(buddy_free[j],area);

      /* Prend un vmem_area dans le cache */
      ar1 = (struct vmem_area*)virtmem_cache_alloc(area_cache, (flags&VIRT_BUDDY_NOMINCHECK?VIRT_CACHE_NOMINCHECK:VIRT_CACHE_DEFAULT));
      if (ar1==NULL)
	{
	  bochs_print("Cannot allocate %d virtual bytes !\n",size);
	  return NULL;
	}

      /* Scinde le noeud en 2 noeuds */

      ar1->base = area->base + (area->size >> 1);
      ar1->size = (area->size >> 1);
      ar1->index = area->index-1;

      area->size = (area->size >> 1);
      area->index = area->index-1;

      LLIST_ADD(buddy_free[j-1],ar1);
      LLIST_ADD(buddy_free[j-1],area);

    }

  /* Maintenant nous avons un noeud disponible au niveau voulu */
  area = LLIST_GETHEAD(buddy_free[ind]);
  /* Met a jour les listes */
  LLIST_REMOVE(buddy_free[ind],area);
  LLIST_ADD(buddy_used,area);
  
  return area;
}


/*========================================================================
 * Liberation (fonction reelle)
 *========================================================================*/

PRIVATE void virtmem_buddy_free_area(struct vmem_area* area)
{
  /* Enleve la zone de buddy_used */
  LLIST_REMOVE(buddy_used,area);

  /* Insere "recursivement" le noeud */
  while((area->index < VIRT_BUDDY_MAX-1)&&(!LLIST_ISNULL(buddy_free[area->index])))
    {
      struct vmem_area* buddy;
      
      /* Recherche d un buddy */
      buddy = LLIST_GETHEAD(buddy_free[area->index]);
      
      while ( (area->base+area->size != buddy->base)
	      && (buddy->base+buddy->size != area->base))
	{
	  buddy = LLIST_NEXT(buddy_free[area->index],buddy);
	  if (LLIST_ISHEAD(buddy_free[area->index],buddy))
	    {
	      /* Pas de buddy, on insere */
	      LLIST_ADD(buddy_free[area->index],area);
	      return;
	    }
	}
      
      /* Buddy trouve ici: fusion */
      area->base = (area->base<buddy->base?area->base:buddy->base);
      area->size <<= 1;
      area->index++;
      /* Enleve le buddy du buddy system */
      LLIST_REMOVE(buddy_free[buddy->index],buddy);
      /* Libere le buddy */
      virtmem_cache_free(area_cache,buddy);

    }
  
  /* Dernier niveau ou niveau vide */
  LLIST_ADD(buddy_free[area->index],area);
  
  return;
}


/*========================================================================
 * Initialise une zone en buddy
 *========================================================================*/

PRIVATE void virtmem_buddy_init_area(u32_t base, u32_t size)
{
  u32_t power;
  u8_t ind;
  struct vmem_area* area;

  base = PAGING_ALIGN_SUP(base);

  while (size >= PAGING_PAGE_SIZE)
    {
      /* Puissance de 2 inferieure */     
      power = size;
      power = power | (power >> 1);
      power = power | (power >> 2);
      power = power | (power >> 4);
      power = power | (power >> 8);
      power = power | (power >> 16);
      power = power - (power >> 1);

      /* Indice dans le buddy */
      ind = msb(power) - PHYS_PAGE_SHIFT;

      /* Prend un vmem_area dans le cache */
      area = (struct vmem_area*)virtmem_cache_alloc(area_cache, VIRT_CACHE_DEFAULT);
      if (area==NULL)
	{
	  bochs_print("Cannot initialize area\n");
	  return;
	}

      /* Remplit le vmem_area */
      area->base = base;
      area->size = power;
      area->index = ind;

      /* Insere dans le buddy */
      LLIST_ADD(buddy_free[ind],area);
      
      size -= power;
      base += power;
    }

  return;
}


/*========================================================================
 * Mapping physique d une area
 *========================================================================*/


PRIVATE void virtmem_buddy_map_area(struct vmem_area* area)
{
  u32_t n,base,sum;
  physaddr_t paddr,pa;
  virtaddr_t va;
  
  n = area->size;
  base = area->base;
  sum = 0;
  
  while( (sum < area->size)&&(n) )
    {
      while (sum < area->size)
	{
	  /* Essaie d allouer physiquement */
	  paddr = (physaddr_t)phys_alloc(n);
	  if (paddr)
	    {
	      /* Incremente la taille physique allouee */
	      sum += n;
	      
	      /* Mappe la memoire physique et virtuelle */
	      for(va=base,pa=paddr;
		  va<base+n;
		  va+=PAGING_PAGE_SIZE,pa+=PAGING_PAGE_SIZE)
		{
		  paging_map(va,pa,TRUE);
		}
	      
	      /* Deplace la base a mapper */
	      base += n;
	      
	    }
	  else
	    {
	      break;
	    }
	}
      
      /* Divise la taille par 2 */
      n >>= 1;
    }
  
  /* Si tout n est pas mappe, on demappe */
  if ( sum < area->size )
    {
      for(va=area->base;va<(area->base+area->size);va+=PAGING_PAGE_SIZE)
	{
	  paging_unmap(va);
	}
      /* On desalloue l area */
      virtmem_buddy_free_area(area);
    }
  
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

/*========================================================================
 * DEBUG: print buddy_free
 *========================================================================*/


PRIVATE void virtmem_print_buddy_free(void)
{
    struct vmem_area* area;
    u8_t i;
    for(i=VIRT_BUDDY_MAX;i;i--)
      {
	if (LLIST_ISNULL(buddy_free[i-1]))
	  {
	    bochs_print("~");
	  }
	else
	  {
	    area = LLIST_GETHEAD(buddy_free[i-1]);
	    do
	      {
		bochs_print("[0x%x (0x%x - %d)] ",area->base,area->size,area->index);
		area=LLIST_NEXT(buddy_free[i-1],area);
	      }while(!LLIST_ISHEAD(buddy_free[i-1],area));
	  }

	bochs_print("\n");
      }
    bochs_print("\n");
    return;
}
