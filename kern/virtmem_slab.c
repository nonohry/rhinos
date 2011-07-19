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
#include "physmem.h"
#include "paging.h"
#include "virtmem_buddy.h"
#include "virtmem_slab.h"


/***********************
 * Declarations PRIVATE
 ***********************/

PRIVATE void virtmem_cache_grow(struct vmem_cache* cache);
PRIVATE void virtmem_cache_grow_big(struct vmem_cache* cache);
PRIVATE void virtmem_cache_grow_little(struct vmem_cache* cache); 
PRIVATE void* virtmem_cache_special_alloc(struct vmem_cache* cache);


PRIVATE void virtmem_print_caches(struct vmem_cache* cache);
PRIVATE void virtmem_print_slabs(struct vmem_slab* slab);
PRIVATE void virtmem_print_bufctls(struct vmem_bufctl* bufctl);


/*******************
 * Caches Statiques
 *******************/

static struct vmem_cache cache_cache =
  {
  name: "cache_cache",
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

static struct vmem_cache slab_cache =
  {
  name: "slab_cache",
  size: sizeof(struct vmem_slab),
  align: 0,
  constructor: NULL,
  destructor: NULL,
  slabs_free: NULL,
  slabs_partial: NULL,
  slabs_full: NULL,
  next: NULL,
  prev: NULL
  };

static struct vmem_cache bufctl_cache =
  {
  name: "bufctl_cache",
  size: sizeof(struct vmem_bufctl),
  align: 0,
  constructor: NULL,
  destructor: NULL,
  slabs_free: NULL,
  slabs_partial: NULL,
  slabs_full: NULL,
  next: NULL,
  prev: NULL
  };


static struct vmem_cache test_cache =
  {
  name: "test_cache",
  size: 1024,
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
  /* Initilise la liste des caches */
  LLIST_SETHEAD(&cache_cache);
  LLIST_SETHEAD(&slab_cache);
  LLIST_SETHEAD(&bufctl_cache);
  LLIST_SETHEAD(&test_cache);

  virtmem_cache_grow(&test_cache);

  virtmem_print_caches(&cache_cache);
  virtmem_print_caches(&slab_cache);
  virtmem_print_caches(&bufctl_cache);
  virtmem_print_caches(&test_cache);

  return;
}



/*************************
 * Croissance du cache
 * (fonction principale)
 *************************/


PRIVATE void virtmem_cache_grow(struct vmem_cache* cache)
{
  /* Redirige sur les 2 fonction de croissance en fonction de la taille */
  if ( cache->size > (PAGING_PAGE_SIZE >> VIRT_CACHE_GROWSHIFT) )
    {
      /* Gros objets */
      virtmem_cache_grow_big(cache);
    }
  else
    {
      /* Petits objets */
      virtmem_cache_grow_little(cache);
    }

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

  /* Calcul le nombre maximal d objets */
  slab->max_objects = (PAGING_PAGE_SIZE - sizeof(struct vmem_slab)) / buf_size;

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


/*******************************************
 * Croissance du cache pour les gros objets
 * (slabs off page)
 *******************************************/

PRIVATE void virtmem_cache_grow_big(struct vmem_cache* cache)
{
  struct vmem_slab* slab;
  struct vmem_bufctl* bc;
  virtaddr_t page;
  u16_t i;

  /* Obtention d une page virtuelle mappee */
  page = (virtaddr_t)virtmem_buddy_alloc(PAGING_PAGE_SIZE, VIRT_BUDDY_MAP);

  /* Obtention d un slab */
  slab = (struct vmem_slab*)virtmem_cache_special_alloc(&slab_cache);

  /* Initialisation du slab */
  slab->count = 0;
  slab->max_objects = PAGING_PAGE_SIZE/cache->size;
  slab->cache = cache;
  LLIST_NULLIFY(slab->free_buf);

  /* Cree les bufctls et les fait pointer sur la page */
  for(i=0; i<slab->max_objects; i++)
    {
      bc = (struct vmem_bufctl*)virtmem_cache_special_alloc(&bufctl_cache);
      
      /* Initialise le bufctl */
      bc->base = page + i*cache->size;
      bc->slab = slab;
      /* Ajout a la liste du slab */
      LLIST_ADD(slab->free_buf,bc);

    }

  /* Relie le nouveau slab au cache */
  LLIST_ADD(cache->slabs_free,slab);

  return;
}



/***********************************
 * Allocation dans un cache special
 ***********************************/

PRIVATE void* virtmem_cache_special_alloc(struct vmem_cache* cache)
{
  struct vmem_slab* slabs_list;
  struct vmem_slab* slab;
  struct vmem_bufctl* bufctl;
  struct ppage_desc* pdesc;

  /* Seulement les caches de base */
  if ( (cache != &cache_cache) &&
       (cache != &slab_cache)  &&
       (cache != &bufctl_cache) )
    {
      return NULL;
    }

  /* Agrandit le cache s il faut */
  if ( (LLIST_ISNULL(cache->slabs_free)) && (LLIST_ISNULL(cache->slabs_partial)) )
    {
      /* Appel de la fonction (non recursive) */
      virtmem_cache_grow_little(cache);
    }

  /* Trouve la liste de slabs de travail */
  slabs_list = (LLIST_ISNULL(cache->slabs_partial)?cache->slabs_free:cache->slabs_partial);

  /* Prend l element de tete */
  slab = LLIST_GETHEAD(slabs_list);

  /* Recupere un bufctl */
  bufctl = LLIST_GETHEAD(slab->free_buf);
  LLIST_REMOVE(slab->free_buf,bufctl);

  /* Lie le bufctl a sa page physique */
  pdesc = PHYS_GET_DESC( paging_virt2phys(bufctl->base)  );
  LLIST_ADD(pdesc->bufctl,bufctl);
  
  /* Actualise le compte du slab */
  slab->count++;

  /* Change le slab de liste au besoins */
  if (slabs_list == cache->slabs_free)
    {
      LLIST_REMOVE(cache->slabs_free,slab);
      LLIST_ADD(cache->slabs_partial,slab);
    }
  else
    {
      /* Change de partial vers full si besoins */
      if (slab->count == slab->max_objects)
	{
	  LLIST_REMOVE(cache->slabs_partial,slab);
	  LLIST_ADD(cache->slabs_full,slab);
	}
    }
  
  /* Retourne l'adresse du buffer */
  return (void*)(bufctl->base);

}


/*========================================================================*/



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
      u16_t n=0;
      bc = LLIST_GETHEAD(bufctl);
      do
	{
	  n++;
	  //bochs_print(" %d ",bc->base);
	  bc = LLIST_NEXT(bufctl,bc);
	  
	}while(!LLIST_ISHEAD(bufctl,bc));

      bochs_print(" %d free (Ox%x-0x%x) ",n,(u32_t)(bufctl->base),(u32_t)(bufctl->prev->base));
      
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
	  bochs_print(" slab [ %d objects:  %d used / ",slab->max_objects, slab->count);
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

PRIVATE void virtmem_print_caches(struct vmem_cache* cache)
{
  struct vmem_cache* c;

  if (LLIST_ISNULL(cache))
    {
      bochs_print(" ~ ");
    }
  else
    {
      c = LLIST_GETHEAD(cache);
      do
	{

	  bochs_print(" name: ");
	  bochs_print(c->name);
	  bochs_print("\n");
	  bochs_print("   size: %d\n",c->size);
	  
	  bochs_print("   slabs_free: ");
	  virtmem_print_slabs(c->slabs_free);
	  bochs_print("\n");
	  
	  bochs_print("   slabs_partial: ");
	  virtmem_print_slabs(c->slabs_partial);
	  bochs_print("\n");

	  bochs_print("   slabs_full: ");
	  virtmem_print_slabs(c->slabs_full);
	  bochs_print("\n");

	  c = LLIST_NEXT(cache,c);

	}while(!LLIST_ISHEAD(cache,c));
    }

  return;
};
