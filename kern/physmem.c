/*
 * Gestion de la memoire physique
 *
 */


#include <types.h>
#include <llist.h>
#include "klib.h"
#include "start.h"
#include "physmem.h"


/*========================================================================			
 * Declarations Private
 *========================================================================*/

PRIVATE void phys_init_area(u32_t base, u32_t size);

PRIVATE struct ppage_desc* ppage_free[PHYS_PAGE_MAX_BUDDY];


/*========================================================================
 * Initialisation
 *========================================================================*/

PUBLIC void phys_init(void)
{
  int i;
  u32_t pool_size;
  struct boot_mmap_e820* entry;

  /* Nullifie les structures de pages */  
  for(i=0; i<PHYS_PAGE_MAX_BUDDY; i++)
    {
      LLIST_NULLIFY(ppage_free[i]);
    }

  /* Calcule la taille maximale du pool */
  pool_size = ((bootinfo->mem_size) >> PHYS_PAGE_SHIFT)*sizeof(struct ppage_desc);

  /* Entre les zones libres du memory map dans le buddy */
  for(entry=(struct boot_mmap_e820*)bootinfo->mem_addr,i=0;i<bootinfo->mem_entry;i++,entry++)
    {
      if (entry->type == START_E820_AVAILABLE)
	{
	  
	  physaddr_t base = (physaddr_t)entry->addr;
	  u32_t size = (u32_t)entry->size;

	  /* Si le noyau est dans la zone, alors sa fin devient le debut de la zone */
	  if ( (bootinfo->kern_end >= base)&&
	       (bootinfo->kern_end <= base+size) )
	    {
	      /* Reajuste la taille */
	      size -= (bootinfo->kern_end - base);
	      base = bootinfo->kern_end;
	    }
	  
	  /* Si le pool est dans la zone, alors sa fin devient le debut de la zone */
	  if ( (PHYS_PAGE_NODE_POOL_ADDR+pool_size >= base)&&
	       (PHYS_PAGE_NODE_POOL_ADDR+pool_size <= base+size) )
	    {
	      /* Reajuste la taille */
	      size -= (PHYS_PAGE_NODE_POOL_ADDR+pool_size - base);
	      base = PHYS_PAGE_NODE_POOL_ADDR+pool_size;
	      
	    }
	  
	  /* Initialise la zone dans le buddy */
	  phys_init_area(base,size);
	}
    }

  return;
}


/*========================================================================
 * Allocation 
 *========================================================================*/

PUBLIC void* phys_alloc(u32_t size)
{

  u32_t i,j;
  int ind;
  struct ppage_desc* pdesc;
  
  /* trouve la puissance de 2 superieure */
  size = size - 1;
  size = size | (size >> 1);
  size = size | (size >> 2);
  size = size | (size >> 4);
  size = size | (size >> 8);
  size = size | (size >> 16);
  size = size + 1;
  
  /* En deduit l indice */
  ind = msb(size) - PHYS_PAGE_SHIFT;
  
  /* Si ppage_free[ind] est NULL, on cherche un niveau superieur disponible */
  for(i=ind;LLIST_ISNULL(ppage_free[i])&&(i<PHYS_PAGE_MAX_BUDDY);i++)
    {}

  if (i>=PHYS_PAGE_MAX_BUDDY)
    {
      bochs_print("Can't allocate %d bytes !\n",size);
      return NULL;
    }
  
  /* Scinde "recursivement" les niveaux superieurs */
  for(j=i;j>ind;j--)
    {
      struct ppage_desc* pd1;
     
      pdesc = LLIST_GETHEAD(ppage_free[j]);
      LLIST_REMOVE(ppage_free[j],pdesc);

      /* Prend un pdesc dans le pool */
      pd1 = PHYS_GET_DESC(pdesc->start + (pdesc->size >> 1));
      PHYS_NULLIFY_DESC(pd1);

      /* Scinde le noeud en 2 noeuds */

      pd1->start = pdesc->start+ (pdesc->size >> 1);
      pd1->size = (pdesc->size >> 1);
      pd1->index = pdesc->index-1;

      pdesc->size = (pdesc->size >> 1);
      pdesc->index = pdesc->index-1;

      LLIST_ADD(ppage_free[j-1],pd1);
      LLIST_ADD(ppage_free[j-1],pdesc);

    }

  /* Maintenant nous avons un noeud disponible au niveau voulu */
  pdesc = LLIST_GETHEAD(ppage_free[ind]);
  /* Met a jour les listes */
  LLIST_REMOVE(ppage_free[ind],pdesc);
  
  return (void*)(pdesc->start);
}


/*========================================================================
 * Liberation
 *========================================================================*/

PUBLIC void phys_free(void* addr)
{
  struct ppage_desc* pdesc;

  /* Cherche la description associee a l adresse */
  pdesc = PHYS_GET_DESC((physaddr_t)addr);
  if (PHYS_PDESC_ISNULL(pdesc))
    {
      return;
    }

  /* Si la taille est nulle, on sort */
  if (!pdesc->size)
    {
      return;
    }

  /* Insere "recursivement" le noeud */
  while((pdesc->index < PHYS_PAGE_MAX_BUDDY-1)&&(!LLIST_ISNULL(ppage_free[pdesc->index])))
    {
      struct ppage_desc* buddy;
      
      /* Recherche d un buddy */
      buddy = LLIST_GETHEAD(ppage_free[pdesc->index]);
      
      while ( (pdesc->start+pdesc->size != buddy->start)
	      && (buddy->start+buddy->size != pdesc->start))
	{
	  buddy = LLIST_NEXT(ppage_free[pdesc->index],buddy);
	  if (LLIST_ISHEAD(ppage_free[pdesc->index],buddy))
	    {
	      /* Pas de buddy, on insere */
	      LLIST_ADD(ppage_free[pdesc->index],pdesc);
	      return;
	    }
	}
      
      /* Buddy trouve ici: fusion */
      pdesc->start = (pdesc->start<buddy->start?pdesc->start:buddy->start);
      pdesc->size <<= 1;
      pdesc->index++;
      /* Enleve le buddy du buddy */
      LLIST_REMOVE(ppage_free[buddy->index],buddy);
      /* Libere le buddy */
      PHYS_NULLIFY_DESC(buddy);

    }
  
  /* Dernier niveau ou niveau vide */
  LLIST_ADD(ppage_free[pdesc->index],pdesc);
  
  return;
}



/*========================================================================
 * Indique un mappage sur une ppage allouee
 *========================================================================*/

PUBLIC void phys_map(physaddr_t addr)
{
  struct ppage_desc* pdesc;
  
  /* Cherche la description associee a l adresse */
  pdesc = PHYS_GET_DESC(addr);
  if (PHYS_PDESC_ISNULL(pdesc))
    {
      return;
    }

  if (pdesc->size)
    {
      /* Incremente le nombre de mappages */
      pdesc->maps++;
    }

  return;
}


/*========================================================================
 * Supprime un mappage sur une ppage allouee
 *========================================================================*/

PUBLIC u8_t phys_unmap(physaddr_t addr)
{
  struct ppage_desc* pdesc;
  
  /* Cherche la description associee a l adresse */
  pdesc = PHYS_GET_DESC(addr);
  if (PHYS_PDESC_ISNULL(pdesc))
    {
      return PHYS_UNMAP_NONE;
    }

  if ((pdesc->size)&&(pdesc->maps))
    {
      /* Decremente le nombre de mappages */
      pdesc->maps--;
      /* Plus de mappage ? */
      if (!(pdesc->maps))
	{
	  /* On libere */
	  phys_free((void*)addr);
	  return PHYS_UNMAP_FREE;
	}
      else
	{
	  return PHYS_UNMAP_UNMAP;
	}
    }

  return PHYS_UNMAP_NONE;
}


/*========================================================================
 * Initialise une zone en buddy
 *========================================================================*/

PRIVATE void phys_init_area(u32_t base, u32_t size)
{
  u32_t power;
  u8_t ind;
  struct ppage_desc* pdesc;

  base = PHYS_ALIGN_SUP(base);

  while (size >= PHYS_PAGE_SIZE)
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

      /* Prend un pdesc dans le pool */
      pdesc = PHYS_GET_DESC(base);
 
      PHYS_NULLIFY_DESC(pdesc);
 
      /* Remplit le pdesc */
      pdesc->start = base;
      pdesc->size = power;
      pdesc->index = ind;

      /* Insere dans le buddy */
      LLIST_ADD(ppage_free[ind],pdesc);
      
      size -= power;
      base += power;
    }

  return;
}


/*========================================================================
 * DEBUG: Affiche le buddy
 *========================================================================*/

PUBLIC void phys_print_buddy(void)
{
  short i;
   bochs_print("-= Buddy =-\n");
  for(i=PHYS_PAGE_MAX_BUDDY-1;i>=0;i--)
    {
      if (LLIST_ISNULL(ppage_free[i]))
	{
	  bochs_print("   ~");
	}
      else
	{
	  struct ppage_desc* pdesc;
	  pdesc = LLIST_GETHEAD(ppage_free[i]);
	  bochs_print("   ");
	  do
	    {
	      bochs_print("[%d (%d)] ",pdesc->start,pdesc->size);
	      pdesc=LLIST_NEXT(ppage_free[i],pdesc);
	    }while(!LLIST_ISHEAD(ppage_free[i],pdesc));
	}

      bochs_print("\n");
    }

  return;
}
