/*
 * Gestion de la memoire physique
 *
 */


#include <types.h>
#include <llist.h>
#include <wmalloc.h>
#include "klib.h"
#include "start.h"
#include "physmem.h"


/***********************
 * Declarations Private
 ***********************/

PRIVATE short phys_msb(u32_t n);
PRIVATE void phys_init_area(u32_t base, u32_t size);
PRIVATE void phys_free_buddy(struct ppage_node* node);
PRIVATE struct ppage_node* phys_find_used(physaddr_t paddr);

PRIVATE struct phys_wm_alloc phys_wm;
PRIVATE struct ppage_node* ppage_free[PHYS_PAGE_MAX_BUDDY];
PRIVATE struct ppage_node* ppage_used;
PRIVATE struct ppage_node* ppage_node_pool;


/*****************
 * Initialisation
 *****************/

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
  LLIST_NULLIFY(ppage_used);
  LLIST_NULLIFY(ppage_node_pool);

  /* Calcule la taille maximale du pool */
  pool_size = ((bootinfo->mem_size) >> PHYS_PAGE_SHIFT)*sizeof(struct ppage_node);

  /* Initialise le WaterMark Allocator */
  WMALLOC_INIT(phys_wm,PHYS_PAGE_NODE_POOL_ADDR,pool_size);

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


/***************
 * Allocation 
 ***************/

PUBLIC void* phys_alloc(u32_t size)
{

  u32_t i,j;
  int ind;
  struct ppage_node* node;
  
  /* trouve la puissance de 2 superieure */
  size = size - 1;
  size = size | (size >> 1);
  size = size | (size >> 2);
  size = size | (size >> 4);
  size = size | (size >> 8);
  size = size | (size >> 16);
  size = size + 1;
  
  /* En deduit l indice */
  ind = phys_msb(size) - PHYS_PAGE_SHIFT;
  
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
      struct ppage_node* n1;
     
      node = LLIST_GETHEAD(ppage_free[j]);
      LLIST_REMOVE(ppage_free[j],node);

      /* Alloue a la volee un ppage_node dans le pool si besoins */
      if (LLIST_ISNULL(ppage_node_pool))
	{
	  struct ppage_node* pnode;
	  pnode = (struct ppage_node*)WMALLOC_ALLOC(phys_wm,sizeof(struct ppage_node));
	  if (pnode==NULL)
	    {
	      bochs_print("Unable to water mark allocate ! \n");
	      return NULL;
	    }
	  LLIST_ADD(ppage_node_pool,pnode);
	}

      /* Prend un node dans le pool */
      n1 = LLIST_GETHEAD(ppage_node_pool);
      LLIST_REMOVE(ppage_node_pool,n1);

      /* Scinde le noeud en 2 noeuds */

      n1->start = node->start;
      n1->size = (node->size >> 1);
      n1->index = node->index-1;

      node->start = node->start + (node->size >> 1);
      node->size = (node->size >> 1);
      node->index = node->index-1;

      LLIST_ADD(ppage_free[j-1],n1);
      LLIST_ADD(ppage_free[j-1],node);

    }

  /* Maintenant nous avons un noeud disponible au niveau voulu */
  node = LLIST_GETHEAD(ppage_free[ind]);
  /* Met a jour les listes */
  LLIST_REMOVE(ppage_free[ind],node);
  LLIST_ADD(ppage_used,node);
  
  return (void*)(node->start);
}


/***************
 * Liberation
 ***************/

PUBLIC void phys_free(void* addr)
{
  struct ppage_node* node;

  /* Cherche le noeud associe a l adresse */
  node = phys_find_used((physaddr_t)addr);

  /* Si un noeud est trouve, on libere */
  if (node != NULL) 
    {
      phys_free_buddy(node);
    }
  return;
}



/*******************************************
 * Indique un mappage sur une ppage allouee
 *******************************************/

PUBLIC void phys_map(physaddr_t addr)
{
  struct ppage_node* node;
  
  /* Cherche le noeud associe a l adresse */
  node = phys_find_used(addr);

  if (node != NULL)
    {
      /* Incremente le nombre de mappages */
      node->maps++;
    }

  return;
}


/*********************************************
 * Supprime un mappage sur une ppage allouee
 *********************************************/

PUBLIC u8_t phys_unmap(physaddr_t addr)
{
  struct ppage_node* node;
  
  /* Cherche le noeud associe a l adresse */
  node = phys_find_used(addr);

  if ((node!=NULL)&&(node->maps))
    {
      /* Decremente le nombre de mappages */
      node->maps--;
      /* Plus de mappage ? */
      if (!(node->maps))
	{
	  /* On libere */
	  phys_free_buddy(node);
	  return PHYS_UNMAP_FREE;
	}
      else
	{
	  return PHYS_UNMAP_UNMAP;
	}
    }

  return PHYS_UNMAP_NONE;
}


/***********************************
 * La fonction reelle de liberation
 ***********************************/

PRIVATE void phys_free_buddy(struct ppage_node* node)
{
  /* Enleve le noeud de la liste des noeuds alloues */
  LLIST_REMOVE(ppage_used,node);
  
  /* Insere "recursivement" le noeud */
  while((node->index < PHYS_PAGE_MAX_BUDDY-1)&&(!LLIST_ISNULL(ppage_free[node->index])))
    {
      struct ppage_node* buddy;
      
      /* Recherche d un buddy */
      buddy = LLIST_GETHEAD(ppage_free[node->index]);
      
      while ( (node->start+node->size != buddy->start)
	      && (buddy->start+buddy->size != node->start))
	{
	  buddy = LLIST_NEXT(ppage_free[node->index],buddy);
	  if (LLIST_ISHEAD(ppage_free[node->index],buddy))
	    {
	      /* Pas de buddy, on insere */
	      LLIST_ADD(ppage_free[node->index],node);
	      return;
	    }
	}
      
      /* Buddy trouve ici */
      node->start = (node->start<buddy->start?node->start:buddy->start);
      node->size <<= 1;
      node->index++;
      LLIST_REMOVE(ppage_free[buddy->index],buddy);
      LLIST_ADD(ppage_node_pool, buddy);
      
    }
  
  /* Dernier niveau ou niveau vide */
  LLIST_ADD(ppage_free[node->index],node);
  
  
  return;
}



/********************************************
 * Trouve le noeud correspondant a l adresse
 ********************************************/

PRIVATE struct ppage_node* phys_find_used(physaddr_t paddr)
{
  struct ppage_node* node;
  
  /* Recherche du node dans ppage_used */
  node = LLIST_GETHEAD(ppage_used);

  while(node->start != paddr)
    {
      node = LLIST_NEXT(ppage_used, node);
      if(LLIST_ISHEAD(ppage_used, node))
	{
	  /* Adresse non trouvee */
	  return NULL;
	}
    }

  /* Adresse trouvee */
  return node;

}


/********************************
 * Initialise une zone en buddy
 ********************************/

PRIVATE void phys_init_area(u32_t base, u32_t size)
{
  u32_t power;
  u8_t ind;
  struct ppage_node* node;

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
      ind = phys_msb(power) - PHYS_PAGE_SHIFT;

      /* Alloue un node dans le pool a la volee si besoins */
      if (LLIST_ISNULL(ppage_node_pool))
	{
	  node = (struct ppage_node*)WMALLOC_ALLOC(phys_wm,sizeof(struct ppage_node));
	  if (node == NULL)
	    {
	      bochs_print("Unable to water mark allocate !\n");
	      return;
	    }
	  LLIST_ADD(ppage_node_pool,node);
	}

      /* Prend un node dans le pool */
      node = LLIST_GETHEAD(ppage_node_pool);
      LLIST_REMOVE(ppage_node_pool,node);

      /* Remplit le node */
      node->start = base;
      node->size = power;
      node->index = ind;

      /* Insere dans le buddy */
      LLIST_ADD(ppage_free[ind],node);
      
      size -= power;
      base += power;
    }

  return;
}

/***********************
 * Most Significant Bit
 ***********************/

PRIVATE short phys_msb(u32_t n)
{
  short  msb=-1;
  
  /* Compte les bits */
  while(n!=0)
    {
      n>>=1;
      msb++;
    }

  return msb;
}
