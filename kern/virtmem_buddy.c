/*
 * Virtmem_buddy.c
 * Allocateur de memoire virtuelle (gros objets)
 *
 */


#include <types.h>
#include "const.h"
#include <llist.h>
#include "klib.h"
#include "assert.h"
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
PRIVATE u8_t virtmem_buddy_free_area(struct vmem_area* area);
PRIVATE u8_t virtmem_buddy_init_area(u32_t base, u32_t size);
PRIVATE u8_t virtmem_buddy_map_area(struct vmem_area* area);


/*========================================================================
 * Initialisation de l'allocateur
 *========================================================================*/

PUBLIC u8_t virtmem_buddy_init()
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
  ASSERT_RETURN( area_cache!=NULL , EXIT_FAILURE);

  /* Initialisation manuelle du cache */
  for(i=0;i<VIRT_BUDDY_STARTSLABS;i++)
    {
       /* Cree une adresse virtuelle mappee pour les initialisations */
      vaddr_init = (i+VIRT_CACHE_STARTSLABS)*PAGING_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT;
      paddr_init = (physaddr_t)phys_alloc(PAGING_PAGE_SIZE);
      paging_map(vaddr_init, paddr_init, TRUE);
      /* Fait grossir cache_cache dans cette page */
      ASSERT_RETURN( virtmem_cache_grow(area_cache,vaddr_init)==EXIT_SUCCESS , EXIT_FAILURE);
    }

  /* Entre les pages des initialisations manuelles dans buddy_used */
  for(i=0;i<VIRT_CACHE_STARTSLABS+VIRT_BUDDY_STARTSLABS;i++)
    {
      area=(struct vmem_area*)virtmem_cache_alloc(area_cache, VIRT_CACHE_DEFAULT);
      ASSERT_RETURN( area!=NULL , EXIT_FAILURE);
      area->base = i*PAGING_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT;
      area->size = PAGING_PAGE_SIZE;
      area->index = 0;
      LLIST_ADD(buddy_used,area);
    }

  /* Initialise la memoire virtuelle disponible pour le noyau */
  ASSERT_RETURN( virtmem_buddy_init_area( (VIRT_CACHE_STARTSLABS+VIRT_BUDDY_STARTSLABS)*PAGING_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT, 
					  VIRT_BUDDY_HIGHTMEM - ((VIRT_CACHE_STARTSLABS+VIRT_BUDDY_STARTSLABS)*PAGING_PAGE_SIZE+VIRT_BUDDY_POOLLIMIT) )==EXIT_SUCCESS , EXIT_FAILURE);

  return EXIT_SUCCESS;
}


/*========================================================================
 * Allocation
 *========================================================================*/

PUBLIC void* virtmem_buddy_alloc(u32_t size, u8_t flags)
{
  struct vmem_area* area;
  struct ppage_desc* pdesc;

  /* Taille minimale */
  if (size < PAGING_PAGE_SIZE )
    {
      size = PAGING_PAGE_SIZE;
    }

  /* Allocation dans le buddy */
  area = virtmem_buddy_alloc_area(size, flags);
  ASSERT_RETURN( area!=NULL , NULL);

  /* Mapping physique selon flags */
  if ( flags & VIRT_BUDDY_MAP )
    {
      if ( virtmem_buddy_map_area(area)==EXIT_FAILURE )
	{
	  /* On libere area si le mapping echoue */
	  virtmem_buddy_free_area(area);
	  return NULL;
	}
      else
	{
	  /* Si le mapping reussi, tentative de lier area au descripteur physique */
	  pdesc = PHYS_GET_DESC( paging_virt2phys((virtaddr_t)area->base)  );
	    if (!PHYS_PDESC_ISNULL(pdesc))
	      {
		LLIST_REMOVE(buddy_used, area);
		LLIST_ADD(pdesc->area, area);
	      }
	}
    }

  /* Retourne l adresse de base */
  return (void*)area->base;

}


/*========================================================================
 * Liberation
 *========================================================================*/

PUBLIC u8_t  virtmem_buddy_free(void* addr)
{
  
  struct vmem_area* area;
  struct ppage_desc* pdesc;
  virtaddr_t va;

  /* Cherche la vmem_area associee a l adresse via le descripteur physique */
  area = NULL;
  pdesc = PHYS_GET_DESC( paging_virt2phys((virtaddr_t)addr)  );
  if ( (!PHYS_PDESC_ISNULL(pdesc)) && (!LLIST_ISNULL(pdesc->area)) )
    {
      /* Parcourt la liste des area du descripteur */
      area = LLIST_GETHEAD(pdesc->area);
      do
	{
	  /* Sort si l adresse est reouvee */
	  if (area->base == (virtaddr_t)addr)
	    {
	      break;
	    }
	  area = LLIST_NEXT(pdesc->area,area);
	}while(!LLIST_ISHEAD(pdesc->area,area));
    }

  /* Cherche la vmem_area associe dans la liste buddy_used sinon */
  if ( (area == NULL) || (area->base != (virtaddr_t)addr) )
    {
      if (!LLIST_ISNULL(buddy_used))
	{
	  area = LLIST_GETHEAD(buddy_used);
	  do
	    {
	      /* Sort si l adresse est trouvee */
	      if (area->base == (virtaddr_t)addr)
		{
		  /* Enleve la zone de buddy_used */
		  LLIST_REMOVE(buddy_used,area);
		  break;
		}
	      /* Suivant */
	      area = LLIST_NEXT(buddy_used, area);
	      
	    }while(!LLIST_ISHEAD(buddy_used, area));
	}
    }
  
  /* Libere si on a l area */
  if (  (area != NULL) && (area->base == (virtaddr_t)addr) )
    {
      /* Demappe physiquement (aucun effet si non mappee) */
      for(va=area->base;va<(area->base+area->size);va+=PAGING_PAGE_SIZE)
	{
	  paging_unmap(va);
	}
      
      /* Reintegre l area dans le buddy */
      ASSERT_RETURN( virtmem_buddy_free_area(area)==EXIT_SUCCESS, EXIT_FAILURE);

      /* Retourne */
      return EXIT_SUCCESS;
      
    }

  return EXIT_FAILURE;
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

  ASSERT_RETURN( i<VIRT_BUDDY_MAX , NULL);
  
  /* Scinde "recursivement" les niveaux superieurs */
  for(j=i;j>ind;j--)
    {
      struct vmem_area* ar1;
     
      area = LLIST_GETHEAD(buddy_free[j]);
      LLIST_REMOVE(buddy_free[j],area);

      /* Prend un vmem_area dans le cache */
      ar1 = (struct vmem_area*)virtmem_cache_alloc(area_cache, (flags&VIRT_BUDDY_NOMINCHECK?VIRT_CACHE_NOMINCHECK:VIRT_CACHE_DEFAULT));
      ASSERT_RETURN( ar1 != NULL , NULL);

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


PRIVATE u8_t virtmem_buddy_free_area(struct vmem_area* area)
{

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
	      return EXIT_SUCCESS;
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
  
  return EXIT_SUCCESS;
}


/*========================================================================
 * Initialise une zone en buddy
 *========================================================================*/

PRIVATE u8_t virtmem_buddy_init_area(u32_t base, u32_t size)
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
      ind = msb(power) - CONST_PAGE_SHIFT;

      /* Prend un vmem_area dans le cache */
      area = (struct vmem_area*)virtmem_cache_alloc(area_cache, VIRT_CACHE_DEFAULT);
      ASSERT_RETURN( area!=NULL, EXIT_FAILURE);

      /* Remplit le vmem_area */
      area->base = base;
      area->size = power;
      area->index = ind;

      /* Insere dans le buddy */
      LLIST_ADD(buddy_free[ind],area);
      
      size -= power;
      base += power;
    }

  return EXIT_SUCCESS;
}


/*========================================================================
 * Mapping physique d une area
 *========================================================================*/


PRIVATE u8_t virtmem_buddy_map_area(struct vmem_area* area)
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
      return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}
