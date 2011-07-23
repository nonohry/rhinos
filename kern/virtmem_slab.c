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


PRIVATE void virtmem_print_caches(struct vmem_cache* cache);
PRIVATE void virtmem_print_slabs(struct vmem_slab* slab);
PRIVATE void virtmem_print_bufctls(struct vmem_bufctl* bufctl);

PRIVATE struct vmem_cache* slab_cache;
PRIVATE struct vmem_cache* bufctl_cache;
PRIVATE struct vmem_cache* cache_list;


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


/*******
 * TEST
 *******/

struct test 
{
  int a;
  int b;
  int c;
  char name[9000];
};

void test_ctor(void* buf, u32_t size)
{
  struct test* test = (struct test*)buf;
  test->a=9;
  test->b=8;
  test->c=89;
  return;
  
}

void test_dtor(void* buf, u32_t size)
{
  struct test* test = (struct test*)buf;
  test->a=0;
  test->b=0;
  test->c=100;
  return;
  
}

/*********************************
 * Initialisation de l allocateur
 *********************************/


PUBLIC void virtmem_cache_init(void)
{
  /* Initilise la liste des caches */
  LLIST_NULLIFY(cache_list);

  /* Les caches des structures de base */
  slab_cache = virtmem_cache_create("slab_cache",sizeof(struct vmem_slab),0,NULL,NULL);
  bufctl_cache = virtmem_cache_create("bufctl_cache",sizeof(struct vmem_bufctl),0,NULL,NULL);

  struct vmem_cache* test_cache = virtmem_cache_create("test_cache",sizeof(struct test),0,test_ctor,test_dtor);

  struct test* toto;

  toto = (struct test*)virtmem_cache_alloc(test_cache);
  /*
  tata = (u32_t)virtmem_cache_alloc(test_cache);
  tutu = (u32_t)virtmem_cache_alloc(test_cache);
  titi = (u32_t)virtmem_cache_alloc(test_cache);
  toto = (u32_t)virtmem_cache_alloc(test_cache);
  */

  bochs_print("a:%d b:%d c:%d\n",toto->a,toto->b,toto->c);

  virtmem_cache_free(test_cache,(void*)toto);

  bochs_print("a:%d b:%d c:%d\n",toto->a,toto->b,toto->c);


  virtmem_print_caches(cache_list);

  return;
}


/**********************
 * Creation d un cache
 **********************/

PUBLIC struct vmem_cache* virtmem_cache_create(const char* name, u16_t size, u16_t align, void (*ctor)(void*,u32_t), void (*dtor)(void*,u32_t))
{
  u8_t i;
  struct vmem_cache* cache;

  /* Allocation du cache */
  cache = (struct vmem_cache*)virtmem_cache_alloc(&cache_cache);
  if (cache == NULL)
    {
      return NULL;
    }

  /* Copie du nom */
  i=0;
  while( (name[i]!=0)&&(i<VIRT_CACHE_NAMELEN-1) )
    {
      cache->name[i] = name[i];
      i++;
    }
  cache->name[i]=0;

  /* Remplissage des champs */
  cache->size = size;
  cache->align = align;
  cache->constructor = ctor;
  cache->destructor = dtor;
  cache->slabs_free = NULL;
  cache->slabs_partial = NULL;
  cache->slabs_full = NULL;

  /* Link a cache_cache */
  LLIST_ADD(cache_list,cache);

  /* Retourne le cache */
  return cache;

}


/***************************
 * Liberation dans un cache
 ***************************/

PUBLIC u8_t virtmem_cache_free(struct vmem_cache* cache, void* buf)
{
  struct ppage_desc* pdesc;
  struct vmem_bufctl* bc;
  struct vmem_slab* slab;

  /* Recupere la page physique */
  pdesc = PHYS_GET_DESC( paging_virt2phys((virtaddr_t)buf) );
  if (pdesc == NULL)
    {
      return EXIT_FAILURE;
    }

  /* Cherche le bufctl dans la page */
  if (LLIST_ISNULL(pdesc->bufctl))
    {
      return EXIT_FAILURE;
    }
  else
    {
      /* Parcours de la liste des bufctl */
      bc = LLIST_GETHEAD(pdesc->bufctl);
      while( bc->base != (virtaddr_t)buf )
	{
	  bc = LLIST_NEXT(pdesc->bufctl,bc);
	  /* Retour si on ne trouve pas le bufctl */
	  if (LLIST_ISHEAD(pdesc->bufctl,bc))
	    {
	      return EXIT_FAILURE;
	    }
	}
    }

  /* Enleve le bufctl de la page physique */
  LLIST_REMOVE(pdesc->bufctl,bc);

  /* Recupere le slab */
  slab = bc->slab;
  /* Modifie le slab en consequence */
  slab->count--;
  LLIST_ADD(slab->free_buf,bc);

  /* Determine les listes du cache a modifier */
  if (slab->count+1 == slab->max_objects)
    {
      /* Passage de full vers partial */
      LLIST_REMOVE(cache->slabs_full,slab);
      if (slab->count)
	{
	  LLIST_ADD(cache->slabs_partial,slab);
	}
      else
	{
	  LLIST_ADD(cache->slabs_free,slab);
	}
    }
  else
    {
      /* Passage de partial a free si possible */
      if (!slab->count)
	{
	  LLIST_REMOVE(cache->slabs_partial,slab);
	  LLIST_ADD(cache->slabs_free,slab);
	}
    }

  /* Applique le destructeur */
  if ( cache->destructor != NULL)
    {
      cache->destructor((void*)buf,cache->size);
    }

  return EXIT_SUCCESS;
}


/*****************************
 * Allocation dans un cache
 *****************************/

PUBLIC void* virtmem_cache_alloc(struct vmem_cache* cache)
{
  struct vmem_slab* slabs_list;
  struct vmem_slab* slab;
  struct vmem_bufctl* bufctl;
  struct ppage_desc* pdesc;

  /* Agrandit le cache s il faut */
  if ( (LLIST_ISNULL(cache->slabs_free)) && (LLIST_ISNULL(cache->slabs_partial)) )
    {
      /* Appel de la fonction (non recursive) */
      virtmem_cache_grow(cache);
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
  /* Lie la page physique au cache */
  pdesc->cache = cache;
  
  /* Actualise le compte du slab */
  slab->count++;

  /* Change le slab de liste au besoins */
  if (slabs_list == cache->slabs_free)
    {
      LLIST_REMOVE(cache->slabs_free,slab);
      if (slab->count == slab->max_objects)
	{
	  LLIST_ADD(cache->slabs_full,slab);
	}
      else
	{
	  LLIST_ADD(cache->slabs_partial,slab);
	}
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

  /* Applique le constructeur */
  if ( cache->constructor != NULL)
    {
      cache->constructor((void*)(bufctl->base),cache->size);
    }

  /* Retourne l'adresse du buffer */
  return (void*)(bufctl->base);

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
  u8_t np;

  /* Calcul du nombre de page */
  np = (PAGING_ALIGN_SUP(cache->size)) >> PAGING_PAGE_SHIFT;

  /* Obtention d'un page virtuelle mappee */
  page = (virtaddr_t)virtmem_buddy_alloc(np*PAGING_PAGE_SIZE, VIRT_BUDDY_MAP);

  /* Initialisation du slab en tete de page */
  slab = (struct vmem_slab*)page;
  slab->count = 0;
  slab->cache = cache;
  slab->n_pages = np;
  slab->start = page+sizeof(struct vmem_slab);
  LLIST_NULLIFY(slab->free_buf);

  /* Taille du bufctl et du buffer associe */
  buf_size = sizeof(struct vmem_bufctl) + cache->size;

  /* Calcul le nombre maximal d objets */
  slab->max_objects = (np*PAGING_PAGE_SIZE - sizeof(struct vmem_slab)) / buf_size;

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
 * (slab off page)
 *******************************************/

PRIVATE void virtmem_cache_grow_big(struct vmem_cache* cache)
{
  struct vmem_slab* slab;
  struct vmem_bufctl* bc;
  virtaddr_t page;
  u16_t i;
  u8_t np;

  /* Calcul du nombre de pages */
  np =  (PAGING_ALIGN_SUP(cache->size)) >> PAGING_PAGE_SHIFT;

  /* Obtention de pages virtuelle mappee */
  page = (virtaddr_t)virtmem_buddy_alloc(np*PAGING_PAGE_SIZE, VIRT_BUDDY_MAP);

  /* Obtention d un slab */
  slab = (struct vmem_slab*)virtmem_cache_alloc(slab_cache);

  /* Initialisation du slab */
  slab->count = 0;
  slab->max_objects = (np*PAGING_PAGE_SIZE)/cache->size;
  slab->cache = cache;
  slab->n_pages = np;
  slab->start = page;
  LLIST_NULLIFY(slab->free_buf);

  /* Cree les bufctls et les fait pointer sur la page */
  for(i=0; i<slab->max_objects; i++)
    {
      bc = (struct vmem_bufctl*)virtmem_cache_alloc(bufctl_cache);
      
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
	  bochs_print(" slab (%d pages, start: 0x%x) [ %d objects:  %d used / ",slab->n_pages,slab->start,slab->max_objects, slab->count);
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
