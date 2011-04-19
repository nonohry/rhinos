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

PRIVATE u8_t phys_isInArea(struct ppage_node* node, u32_t start, u32_t size); 
PRIVATE void phys_print(struct ppage_node* l);

PRIVATE struct ppage_node* ppage_free[PPAGE_MAX_BUDDY];
PRIVATE struct ppage_node* ppage_used;
PRIVATE struct ppage_node* ppage_node_pool;


/*****************
 * Initialisation
 *****************/

PUBLIC void physmem_init(void)
{
  u32_t i;
  u32_t ram_size=0;
  u32_t ram_pages;
  struct ppage_node* node;

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
	  ram_size += (u32_t)entry->size;
	}
    }
  else
    {
      /* Taille selon int 15/AX=E801 (limite a 4G) */
      ram_size = ((bootinfo->mem_lower + (bootinfo->mem_upper << SHIFT64))<<SHIFT1024);
    }

  /* Nombre maximal de page */
  ram_pages = (ram_size >> PPAGE_SHIFT);

  /* Cree le pool de node et le buddy */
  for(i=0; i<ram_pages; i++)
    {
      node=(struct ppage_node*)(PPAGE_NODE_POOL_ADDR+i*sizeof(struct ppage_node));
      node->start=(i<<PPAGE_SHIFT);
      node->size=(1<<PPAGE_SHIFT);
      /* Enfile dans a liste ppage_used */
      LLIST_ADD(ppage_used,node);  
    
      /* Construit le buddy en liberant les pages libres */
      if ( !phys_isInArea(node,KERN_AREA_START,bootinfo->kern_end+1) &&
	   !phys_isInArea(node,ROM_AREA_START,ROM_AREA_SIZE)  &&
	   !phys_isInArea(node,POOL_AREA_START,ram_pages*sizeof(struct ppage_node)) &&
	   !phys_isInArea(node,ACPI_AREA_START,ACPI_AREA_SIZE) )
	{
	  /* Libere la page du node */
	  phys_free((void*)node->start);
	}

    }

  for(i=1;i<=PPAGE_MAX_BUDDY;i++)
    {
      phys_print(ppage_free[PPAGE_MAX_BUDDY-i]);
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
 
  /* Trouve le msb de la puissance */
  ind=-1;
  while(size!=0)
    {
      size>>=1;
      ind++;
    }
  
  /* En deduit l indice */
  ind -= PPAGE_SHIFT;
  
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
  
  return (void*)(node->start);
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
  while((node->index < PPAGE_MAX_BUDDY-1)&&(!LLIST_ISNULL(ppage_free[node->index])))
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
  LLIST_ADD(ppage_free[node->index],node);

  return;
}


/************************
 * Page dans une region 
 ************************/

PRIVATE u8_t phys_isInArea(struct ppage_node* node, u32_t start, u32_t size)
{
     u8_t case1,case2,case3,case4;
     
     case1 = ((node->start+node->size)<=(start+size))&&((node->start+node->size)>(start));
     case2 = ((node->start)<(start+size))&&((node->start)>=(start));
     case3 = ((node->start)>=(start))&&((node->start+node->size)<=(start+size));
     case4 = ((node->start)<=(start))&&((node->start+node->size)>=(start+size));

     return (case1 || case2 || case3 || case4);

}


PRIVATE void phys_print(struct ppage_node* l)
{
  if (LLIST_ISNULL(l))
    {
      bochs_print("~");
    }
  else
    {
      struct ppage_node* node;
      node = LLIST_GETHEAD(l);
      do
	{
	  bochs_print("[%d (%d)] ",node->start,node->size);
	  node = LLIST_NEXT(l,node);
	}while(!LLIST_ISHEAD(l,node));

    }

  bochs_print("\n");
  return;

}
