/*
 * Virtmem_slab.c
 * Allocateur de memoire virtuelle (petits objets)
 *
 */


/***********
 * Includes
 ***********/

#include <types.h>
#include <llist.h>
#include "klib.h"
#include "paging.h"
#include "virtmem_buddy.h"
#include "virtmem_slab.h"


/***********************
 * Declarations PRIVATE
 ***********************/

PRIVATE void virtmem_cache_grow_little(struct vmem_cache* cache); 

PRIVATE void virtmem_print_cache(struct vmem_cache* cache);
PRIVATE void virtmem_print_slabs(struct vmem_slab* slab);
PRIVATE void virtmem_print_bufctls(struct vmem_bufctl* bufctl);


/*****************
 * Cache Statique
 *****************/

static struct vmem_cache cache_cache =
  {
  name: "vmem_cache",
  size: sizeof(struct vmem_cache),
  align: 0,
  constructor: NULL,
  destructor: NULL,
  slabs_free: NULL,
  slabs_partial: NULL,
  slabs_full: NULL,
  next: NULL,
  prev: NULL
  };


/*********************************
 * Initialisation de l allocateur
 *********************************/


PUBLIC void virtmem_cache_init(void)
{
  virtmem_print_cache(&cache_cache);
  virtmem_cache_grow_little(&cache_cache);
  virtmem_print_cache(&cache_cache);

  return;
}



/*****************************************
 * Croissance du cache pour petits objets 
 * (slab on page)
 *****************************************/

PRIVATE void virtmem_cache_grow_little(struct vmem_cache* cache)
{ 
  struct vmem_slab* slab;
  virtaddr_t buf;
  virtaddr_t page;
  u16_t buf_size;

  /* Obtention d'un page virtuelle mappee */
  page = (virtaddr_t)virtmem_buddy_alloc(PAGING_PAGE_SIZE, VIRT_BUDDY_MAP);

  /* Initialisation du slab en tete de page */
  slab = (struct vmem_slab*)page;
  slab->count = 0;
  slab->cache = cache;
  LLIST_NULLIFY(slab->free_buf);

  /* Taille du bufctl et du buffer associe */
  buf_size = sizeof(struct vmem_bufctl) + cache->size;

  /* Cree les bufctl et les buffers dans la page */
  for(buf = (page+sizeof(struct vmem_slab));
      buf < (page+PAGING_PAGE_SIZE-buf_size);
      buf += buf_size)
    {
      struct vmem_bufctl* bc = (struct vmem_bufctl*)buf;

      /* Initialise le bufctl */
      bc->base = buf+sizeof(struct vmem_bufctl);
      bc->slab = slab;
      /* Ajoute a la liste du slab */
      LLIST_ADD(slab->free_buf,bc);
    }

  /* Relie le nouveau slab au cache */
  LLIST_ADD(cache->slabs_free,slab);

  return;
}


/*******************************
 * DEBUG: Affichage des bufctls
 *******************************/

PRIVATE void virtmem_print_bufctls(struct vmem_bufctl* bufctl)
{
  struct vmem_bufctl* bc;
  
  if (LLIST_ISNULL(bufctl))
    {
      bochs_print("~ ");
    }
  else
    {
      bc = LLIST_GETHEAD(bufctl);
      do
	{
	  bochs_print(" %d ",bc->base);
	  bc = LLIST_NEXT(bufctl,bc);
	  
	}while(!LLIST_ISHEAD(bufctl,bc));
      
    }

  return;
}

/*****************************
 * DEBUG: Affichage des slabs
 *****************************/

PRIVATE void virtmem_print_slabs(struct vmem_slab* slab)
{
  struct vmem_slab* sl;

  if (LLIST_ISNULL(slab))
    {
      bochs_print("~ ");
    }
  else
    {
      sl = LLIST_GETHEAD(slab);
      do
	{
	  bochs_print(" slab [ ");
	  virtmem_print_bufctls(sl->free_buf);
	  bochs_print(" ] ");
	  sl = LLIST_NEXT(slab,sl);
	  
	}while(!LLIST_ISHEAD(slab,sl));
      
    }

  return;
}

/******************************
 * DEBUG: Affichage des caches
 ******************************/

PRIVATE void virtmem_print_cache(struct vmem_cache* cache)
{

  bochs_print(" name: %s\n",cache->name);
  bochs_print("   size: %d\n",cache->size);
  
  bochs_print("   slabs_free: ");
  virtmem_print_slabs(cache->slabs_free);
  bochs_print("\n");
  
  bochs_print("   slabs_partial: ");
  virtmem_print_slabs(cache->slabs_partial);
  bochs_print("\n");

  bochs_print("   slabs_full: ");
  virtmem_print_slabs(cache->slabs_full);
  bochs_print("\n");

  return;
};
