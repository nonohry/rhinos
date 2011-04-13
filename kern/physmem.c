/*
 * Gestion de la memoire physique
 *
 */


#include <types.h>
#include <llist.h>
#include "klib.h"
#include "start.h"
#include "physmem.h"


/***********************
 * Declaration Private
 ***********************/

PRIVATE u8_t phys_powerof2(u32_t n);
PRIVATE struct ppage_node* ppage_free[PPAGE_MAX_BUDDY];
PRIVATE struct ppage_node* ppage_used;
PRIVATE struct ppage_node* ppage_node_pool;

/*****************
 * Initialisation
 *****************/

PUBLIC void physmem_init(void)
{
  u32_t i;
  u64_t ram_size=0;

  /* Nullifie les structures de pages */  
  for(i=0; i<PPAGE_MAX_BUDDY; i++)
    {
      LLIST_NULLIFY(ppage_free[i]);
    }
  LLIST_NULLIFY(ppage_used);
  LLIST_NULLIFY(ppage_node_pool);

  /* Recupere la taille de la mémoire */
  if (bootinfo->mem_entry)
    {
      /* Taille selon int 15/AX=E820 */
      struct boot_mmap_e820* entry;
      for(entry=(struct boot_mmap_e820*)bootinfo->mem_addr,i=0;i<bootinfo->mem_entry;i++,entry++)
	{
	  ram_size += entry->size;
	}
    }
  else
    {
      /* Taille selon int 15/AX=E801 (limite a 4G) */
      ram_size = (bootinfo->mem_lower + (bootinfo->mem_upper << SHIFT64))<<SHIFT1024;
    }

  return;
}


/*****************************************
 *  La puissance de 2 superieure ou egale 
 *  et plancher a 12 (2^12 = 4096)
 *****************************************/

PRIVATE u8_t phys_powerof2(u32_t n)
{
  int i;
  u32_t m;

  i=-1;
  m=n;

  /* Shift a droite pour obtenir le msb */
  while(m!=0)
    {
      m>>=1;
      i++;
    }

  /* si n==2^msb alors on renvoie msb sinon msb+1 */
  i=i+(n==(1<<i)?0:1);
  
  /* Plancher a PPAGE_SHIFT */
  return (i<PPAGE_SHIFT?PPAGE_SHIFT:i);
}



/***************
 * Allocation 
 ***************/

PUBLIC void* phys_alloc(u32_t size)
{

  u32_t i,j,k,ind;
  struct ppage_node* node;
  
  /* Determine msb plafonne a PAGE_SHIFT */
  k = phys_powerof2(size);
  /* En deduit l indice dans ppage_free */
  ind = k - PPAGE_SHIFT;
  
  /* Si ppage_free[ind] est NULL, on cherche un niveau superieur disponible */
  for(i=ind;LLIST_ISNULL(ppage_free[i])&&(i<PPAGE_MAX_BUDDY);i++)
    {}

  if (i>=PPAGE_MAX_BUDDY)
    {
      bochs_print("Can't allocate %d bytes !\n",size);
      return NULL;
    }
  
  /* Scinde "recursivement" les niveaux superieurs */
  for(j=i;j>ind;j--)
    {
      struct ppage_node *n1;
     
      node = LLIST_GETHEAD(ppage_free[j]);
      LLIST_REMOVE(ppage_free[j],node);

      n1 = LLIST_GETHEAD(ppage_node_pool);
      LLIST_REMOVE(ppage_node_pool,n1);

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
  
  return (void*)node->start;
}



/***************
 * Liberation
 ***************/

PUBLIC void phys_free(void* addr)
{
  struct ppage_node* node;

  /* Recherche du node dans ppage_used */
  node = LLIST_GETHEAD(ppage_used);

  while(node->start != (u32_t)addr)
    {
      node = LLIST_NEXT(ppage_used, node);
      if(LLIST_ISHEAD(ppage_used, node))
	{
	  /* Adresse non trouvee */
	  return;
	}
    }

  /* Adresse trouvee ici, on supprime le noeud de la liste allouee */
  LLIST_REMOVE(ppage_used,node);

  /* Insere "recursivement le noeud */
  while((node->index < PPAGE_MAX_BUDDY)&&(!LLIST_ISNULL(ppage_free[node->index])))
    {
      struct ppage_node* buddy;
      
      /* Recherche d un budy */
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
  /* Si c est le dernier niveau, on reajuste les champs */
  if (node->index > PPAGE_MAX_BUDDY)
    {
      node->index--;
      node->size >>= 1;
    }
  LLIST_ADD(ppage_free[node->index],node);

  return;
}
