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


/**************************
 * DEBUG: Affiche un cache
 **************************/

PRIVATE void virtmem_print_cache(struct vmem_cache* cache)
{
  struct vmem_slab* slab;
  struct vmem_bufctl* bufctl;

  bochs_print(" name: %s\n",cache->name);
  bochs_print("   size: %d\n",cache->size);
  
  bochs_print("   slabs_free: ");
  if (LLIST_ISNULL(cache->slabs_free))
    {
      bochs_print("~");
    }
  else
    {
      slab = LLIST_GETHEAD(cache->slabs_free);
      do
	{
	  bochs_print("slab [");
	  
	  if (LLIST_ISNULL(slab->free_buf))
	    {
	      bochs_print("~ ");
	    }
	  else
	    {
	      bufctl = LLIST_GETHEAD(slab->free_buf);
	      do
		{
		  bochs_print(" %d ",bufctl->base);
		  bufctl = LLIST_NEXT(slab->free_buf,bufctl);

		}while(!LLIST_ISHEAD(slab->free_buf,bufctl));

	    }

	  bochs_print("]");
	  slab = LLIST_NEXT(cache->slabs_free,slab);
	}while(!LLIST_ISHEAD(cache->slabs_free,slab));
    }
  bochs_print("\n");
  
  bochs_print("   slabs_partial: ");
  if (LLIST_ISNULL(cache->slabs_partial))
    {
      bochs_print("~");
    }
  else
    {
      slab = LLIST_GETHEAD(cache->slabs_partial);
      do
	{
	  bochs_print("slab [");
	  bochs_print("]");
	  slab = LLIST_NEXT(cache->slabs_partial,slab);
	}while(!LLIST_ISHEAD(cache->slabs_partial,slab));
    }
  bochs_print("\n");

  bochs_print("   slabs_full: ");
  if (LLIST_ISNULL(cache->slabs_full))
    {
      bochs_print("~");
    }
  else
    {
      slab = LLIST_GETHEAD(cache->slabs_full);
      do
	{
	  bochs_print("slab [");
	  bochs_print("]");
	  slab = LLIST_NEXT(cache->slabs_full,slab);
	}while(!LLIST_ISHEAD(cache->slabs_full,slab));
    }
  bochs_print("\n");

  return;
};



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
