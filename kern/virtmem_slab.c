/*
 * Virtmem_slab.c
 * Allocateur de memoire virtuelle (petits objets)
 *
 */


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include <llist.h>
#include "klib.h"
#include "assert.h"
#include "start.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem_buddy.h"
#include "virtmem_slab.h"


/*========================================================================
 * Declarations PRIVATE
 *========================================================================*/

PRIVATE u8_t virtmem_cache_grow_big(struct vmem_cache* cache, virtaddr_t addr);
PRIVATE u8_t virtmem_cache_grow_little(struct vmem_cache* cache, virtaddr_t addr); 
PRIVATE u8_t virtmem_slab_destroy(struct vmem_cache* cache,struct vmem_slab* slab);

PRIVATE struct vmem_cache* slab_cache;
PRIVATE struct vmem_cache* bufctl_cache;
PRIVATE struct vmem_cache* cache_list;

/*========================================================================
 * Caches Statiques
 *========================================================================*/

static struct vmem_cache cache_cache =
  {
  name: "cache_cache",
  size: sizeof(struct vmem_cache),
  align: 0,
  min_slab_free: 0,
  flags: VIRT_CACHE_NOREAP,
  constructor: NULL,
  destructor: NULL,
  slabs_free: NULL,
  slabs_partial: NULL,
  slabs_full: NULL,
  next: NULL,
  prev: NULL
  };



/*========================================================================
 * Initialisation de l allocateur
 *========================================================================*/


PUBLIC u8_t virtmem_cache_init(void)
{
  virtaddr_t vaddr_init;
  physaddr_t paddr_init;
  u32_t i;

  /* Initialise la liste des caches */
  LLIST_NULLIFY(cache_list);
  LLIST_SETHEAD(&cache_cache);
      
  /* Initialisation manuelle de cache_cache */
  for(i=0;i<VIRT_CACHE_STARTSLABS;i++)
    {
      /* Cree une adresse virtuelle mappee pour les initialisations */
      vaddr_init = i*PAGING_PAGE_SIZE + VIRT_BUDDY_POOLLIMIT;
      paddr_init = (physaddr_t)phys_alloc(PAGING_PAGE_SIZE);
      paging_map(vaddr_init, paddr_init, TRUE);
      /* Fait grossir cache_cache dans cette page */
      ASSERT_RETURN( virtmem_cache_grow(&cache_cache,vaddr_init)==EXIT_SUCCESS , EXIT_FAILURE );
    }

  /* Les caches des structures de base */
  slab_cache = virtmem_cache_create("slab_cache",sizeof(struct vmem_slab),0,0,VIRT_CACHE_NOREAP,NULL,NULL);
  ASSERT_RETURN( slab_cache!=NULL , EXIT_FAILURE);

  bufctl_cache = virtmem_cache_create("bufctl_cache",sizeof(struct vmem_bufctl),0,0,VIRT_CACHE_NOREAP,NULL,NULL);
  ASSERT_RETURN( bufctl_cache!=NULL , EXIT_FAILURE);

  return EXIT_SUCCESS;
}


/*========================================================================
 * Creation d un cache
 *========================================================================*/


PUBLIC struct vmem_cache* virtmem_cache_create(const char* name, u16_t size, u16_t align, u16_t min_slab_free, u8_t flags, void (*ctor)(void*,u32_t), void (*dtor)(void*,u32_t))
{
  u8_t i;
  struct vmem_cache* cache;

  /* Allocation du cache */
  cache = (struct vmem_cache*)virtmem_cache_alloc(&cache_cache, VIRT_CACHE_DEFAULT);
  ASSERT_RETURN( cache!=NULL , NULL);

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
  cache->align_offset = 0;
  cache->min_slab_free = min_slab_free;
  cache->flags = flags;
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


/*========================================================================
 * Liberation dans un cache
 *========================================================================*/


PUBLIC u8_t virtmem_cache_free(struct vmem_cache* cache, void* buf)
{
  struct ppage_desc* pdesc;
  struct vmem_bufctl* bc;
  struct vmem_slab* slab;

  /* Recupere la page physique */
  pdesc = PHYS_GET_DESC( paging_virt2phys((virtaddr_t)buf) );
  ASSERT_RETURN( !PHYS_PDESC_ISNULL(pdesc) , EXIT_FAILURE);

  /* Parcours de la liste des bufctl */
  ASSERT_RETURN( !LLIST_ISNULL(pdesc->bufctl) , EXIT_FAILURE);
  bc = LLIST_GETHEAD(pdesc->bufctl);
  while( bc->base != (virtaddr_t)buf )
    {
      bc = LLIST_NEXT(pdesc->bufctl,bc);
      /* Retour si on ne trouve pas le bufctl */
      ASSERT_RETURN( !LLIST_ISHEAD(pdesc->bufctl,bc) , EXIT_FAILURE);
    }


  /* Recupere le slab */
  slab = bc->slab;

  /* Verifie que le cache est bien celui attendu */
  if (slab->cache != cache)
    {
      return EXIT_FAILURE;
    }

  /* Enleve le bufctl de la page physique */
  LLIST_REMOVE(pdesc->bufctl,bc);


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

  return EXIT_SUCCESS;
}


/*========================================================================
 * Allocation dans un cache
 *========================================================================*/


PUBLIC void* virtmem_cache_alloc(struct vmem_cache* cache, u8_t flags)
{
  struct vmem_slab* slabs_list;
  struct vmem_slab* slab;
  struct vmem_bufctl* bufctl;
  struct ppage_desc* pdesc;
  u32_t count;

  /* Agrandit le cache s il faut */
  if ( (LLIST_ISNULL(cache->slabs_free)) && (LLIST_ISNULL(cache->slabs_partial)) )
    {
      /* Appel de la fonction  */
      ASSERT_RETURN( virtmem_cache_grow(cache, VIRT_CACHE_NOADDR)==EXIT_SUCCESS , NULL);
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
  ASSERT_RETURN( !PHYS_PDESC_ISNULL(pdesc) , NULL);

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


  /* Controle du min_slab_free */
  if ( (cache->min_slab_free) && !(flags & VIRT_CACHE_NOMINCHECK) )
    {
      /* Compte les slabs free */
      count = 0;
      if (!LLIST_ISNULL(cache->slabs_free))
	{
	  slab = LLIST_GETHEAD(cache->slabs_free);
	  do
	    {
	      count++;
	      slab = LLIST_NEXT(cache->slabs_free,slab);
	    }while(!LLIST_ISHEAD(cache->slabs_free,slab));
	}
      
      /* Grossit le cache autant que necessaire */
      while (count < cache->min_slab_free )
	{
	  virtmem_cache_grow(cache, VIRT_CACHE_NOADDR);
	  count++;
	}
    }
  

  /* Retourne l'adresse du buffer */
  return (void*)(bufctl->base);

}


/*========================================================================
 * Destruction d un cache
 *========================================================================*/


PUBLIC u8_t virtmem_cache_destroy(struct vmem_cache* cache)
{
  u8_t i;

  /* Petit controle */
  ASSERT_RETURN( LLIST_ISNULL(cache->slabs_partial)&&LLIST_ISNULL(cache->slabs_full) , EXIT_FAILURE);

  /* Parcourt la liste des slabs */
  while(!LLIST_ISNULL(cache->slabs_free))
    {
      struct vmem_slab* slab = LLIST_GETHEAD(cache->slabs_free);
      /* Detruit le slab */
      virtmem_slab_destroy(cache,slab);
      /* Detruit le chainage */
      LLIST_REMOVE(cache->slabs_free,slab);
      /* Libere le slab au besoin */
       if (cache->size > (PAGING_PAGE_SIZE >> VIRT_CACHE_GROWSHIFT))
	{
	  ASSERT_RETURN( virtmem_cache_free(slab_cache,slab)==EXIT_SUCCESS, EXIT_FAILURE) ;
	}
    }
  
  /* Remise a zero */
  for(i=0;i<VIRT_CACHE_NAMELEN;i++)
    {
      cache->name[i] = 0;
    }
  cache->size = 0;
  cache->align = 0;
  cache->constructor = NULL;
  cache->destructor = NULL;
  LLIST_REMOVE(cache_list,cache);

  /* Liberation */
  ASSERT_RETURN( virtmem_cache_free(&cache_cache,cache)==EXIT_SUCCESS, EXIT_FAILURE);

  return EXIT_SUCCESS;
}



/*========================================================================
 * Reaping de l allocateur
 *========================================================================*/


PUBLIC u32_t virtmem_cache_reap(u8_t flags)
{
  struct vmem_cache* cache;
  struct vmem_cache* cache_reap;
  struct vmem_slab* slab;
  u32_t pages, max_pages,i;

  /* Controle */
  ASSERT_RETURN( !LLIST_ISNULL(cache_list) , 0);

  /* Initialise le parcours des caches */
  cache = LLIST_GETHEAD(cache_list);
  cache_reap = NULL;
  i = 1;
  max_pages = 0;

  /* Parcours des caches */
  do
    {
      pages = 0;

      /* Controle des flags et des slabs libres */
      if ( ( !(cache->flags & VIRT_CACHE_NOREAP)  || (flags & VIRT_CACHE_BRUTALREAP) ) &&
	   ( !(cache->flags & VIRT_CACHE_JUSTGROWN) || (flags & VIRT_CACHE_FORCEREAP) || (flags & VIRT_CACHE_BRUTALREAP) ) &&
	   !(LLIST_ISNULL(cache->slabs_free)) )
	{
	  /* Parcourt les slabs de slabs_free */
	  slab = LLIST_GETHEAD(cache->slabs_free);
	  do
	    {
	      /* Compte les pages susceptibles d etre liberee */
	      pages += slab->n_pages;
	      slab = LLIST_NEXT(cache->slabs_free,slab);
	      
	    }while(!LLIST_ISHEAD(cache->slabs_free,slab));

	  /* Met a jour le maximum de page et le cache associe */
	  if (pages > max_pages)
	    {
	      max_pages = pages;
	      cache_reap = cache;
	    }
	}
      
      /* Met a jour le flag dans tous les cas */
      cache->flags &= ~VIRT_CACHE_JUSTGROWN;
      cache = LLIST_NEXT(cache_list,cache);
      i++;

    }while( (!LLIST_ISHEAD(cache_list,cache))&&(i<VIRT_CACHE_REAPLEN)  );

  /* Deplace la tete de liste (cas VIRT_CACHE_REAPLEN, aucun effet sinon) */
  cache_list = cache;

  /* S'assure qu on libere des pages */
  ASSERT_RETURN( max_pages , 0);

  /* Detruit les slabs du cache selectionne*/
  while(!LLIST_ISNULL(cache_reap->slabs_free))
    {
      slab = LLIST_GETHEAD(cache_reap->slabs_free);

      /* Detruit le slab */
      ASSERT_RETURN( virtmem_slab_destroy(cache_reap,slab)==EXIT_SUCCESS, 0);

      /* Detruit le chainage */
      LLIST_REMOVE(cache_reap->slabs_free,slab);

      /* Libere le slab au besoin */
       if (cache_reap->size > (PAGING_PAGE_SIZE >> VIRT_CACHE_GROWSHIFT))
	{
	  ASSERT_RETURN( virtmem_cache_free(slab_cache,slab)==EXIT_SUCCESS, 0);
	}

    }

  /* Retourne le nombre de pages liberees */
  return max_pages;
  
}


/*========================================================================
 * Croissance du cache
 * (fonction principale)
 *========================================================================*/


PUBLIC u8_t virtmem_cache_grow(struct vmem_cache* cache, virtaddr_t addr)
{
  u8_t res;

  /* Redirige sur les 2 fonction de croissance en fonction de la taille */
  if ( cache->size > (PAGING_PAGE_SIZE >> VIRT_CACHE_GROWSHIFT) )
    {
      /* Gros objets */
      res = virtmem_cache_grow_big(cache, addr);
    }
  else
    {
      /* Petits objets */
      res = virtmem_cache_grow_little(cache, addr);
    }

  /* Indique la croissance */
  if (res == EXIT_SUCCESS)
    {
      cache->flags |= VIRT_CACHE_JUSTGROWN;
    }

  return res;
}



/*========================================================================
 * Destruction d un slab
 *========================================================================*/


PRIVATE u8_t virtmem_slab_destroy(struct vmem_cache* cache,struct vmem_slab* slab)
{
  virtaddr_t page;

  /* Petit controle */
  ASSERT_RETURN( !slab->count , EXIT_FAILURE);

  /* Destruction des objets et des bufctl */
  while(!LLIST_ISNULL(slab->free_buf))
    {
      struct vmem_bufctl* bc = LLIST_GETHEAD(slab->free_buf);
      
      /* Detruit l'objet si possible */
      if (cache->destructor != NULL)
	{
	  cache->destructor((void*)(bc->base),cache->size);
	}

      /* Remise a zero */
      bc->base = 0;
      bc->slab = NULL;
      LLIST_REMOVE(slab->free_buf,bc);

      /* Libere le bufctl si off slab */
      if (cache->size > (PAGING_PAGE_SIZE >> VIRT_CACHE_GROWSHIFT))
	{
	  ASSERT_RETURN( virtmem_cache_free(bufctl_cache,bc)==EXIT_SUCCESS, EXIT_FAILURE);
	}

    }

  /* Libere les pages virtuelles */
  page = PAGING_ALIGN_INF((virtaddr_t)slab->start);
  ASSERT_RETURN( virtmem_buddy_free((void*)page)==EXIT_SUCCESS , EXIT_FAILURE);

  /* Remise a zero */
  slab->max_objects = 0;
  slab->n_pages = 0;
  slab->cache = NULL;
  slab->start = 0;

  return EXIT_SUCCESS;
}



/*========================================================================
 * Croissance du cache pour petits objets 
 * (slab on page)
 *========================================================================*/


PRIVATE u8_t virtmem_cache_grow_little(struct vmem_cache* cache, virtaddr_t addr)
{ 
  struct vmem_slab* slab;
  virtaddr_t buf;
  virtaddr_t page;
  u16_t buf_size, wasted;
  u8_t np;

  /* Calcul du nombre de page */
  np = (PAGING_ALIGN_SUP(cache->size)) >> PAGING_PAGE_SHIFT;

  /* Obtention d'un page virtuelle mappee */
  if (addr == VIRT_CACHE_NOADDR)
    {
      page = (virtaddr_t)virtmem_buddy_alloc(np*PAGING_PAGE_SIZE, VIRT_BUDDY_MAP | VIRT_BUDDY_NOMINCHECK);
      ASSERT_RETURN( ((void*)page) != NULL , EXIT_FAILURE);
    }
  else
    {
      /* Verifie que l adresse est bien mappee */
      struct ppage_desc* pdesc = PHYS_GET_DESC( paging_virt2phys(addr)  );
      ASSERT_RETURN( !PHYS_PDESC_ISNULL(pdesc) , EXIT_FAILURE);
      page = addr;
    }
  
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

  /* Calcul l alignement */
  if (cache->align)
    {
      wasted = (np*PAGING_PAGE_SIZE)-(slab->max_objects*cache->size);
      if (wasted)
	{
	  slab->start += cache->align_offset;
	  cache->align_offset = (cache->align_offset+cache->align)%wasted;
	}
    }

  /* Relie le nouveau slab au cache */
  LLIST_ADD(cache->slabs_free,slab);

  /* Cree les bufctl et les buffers dans la page */
  for(buf = slab->start;
      buf < (page+PAGING_PAGE_SIZE-buf_size);
      buf += buf_size)
    {
      struct vmem_bufctl* bc = (struct vmem_bufctl*)buf;

      /* Initialise le bufctl */
      bc->base = buf+sizeof(struct vmem_bufctl);
      bc->slab = slab;
      /* Applique le constructeur */
      if ( cache->constructor != NULL)
	{
	  cache->constructor((void*)(bc->base),cache->size);
	}
      /* Ajoute a la liste du slab */
      LLIST_ADD(slab->free_buf,bc);
    }

  return EXIT_SUCCESS;
}



/*========================================================================
 * Croissance du cache pour les gros objets
 * (slab off page)
 *========================================================================*/


PRIVATE u8_t virtmem_cache_grow_big(struct vmem_cache* cache, virtaddr_t addr)
{
  struct vmem_slab* slab;
  struct vmem_bufctl* bc;
  virtaddr_t page;
  u16_t i, wasted;
  u8_t np;

  /* Calcul du nombre de pages */
  np =  (PAGING_ALIGN_SUP(cache->size)) >> PAGING_PAGE_SHIFT;

  /* Obtention de pages virtuelle mappee */
  if (addr == VIRT_CACHE_NOADDR)
    {
      page = (virtaddr_t)virtmem_buddy_alloc(np*PAGING_PAGE_SIZE, VIRT_BUDDY_MAP | VIRT_BUDDY_NOMINCHECK);
      ASSERT_RETURN( ((void*)page)!=NULL , EXIT_FAILURE);
    }
  else
    {
      /* Verifie que l adresse est bien mappee */
      struct ppage_desc* pdesc = PHYS_GET_DESC( paging_virt2phys(addr)  );
      ASSERT_RETURN( !PHYS_PDESC_ISNULL(pdesc) , EXIT_FAILURE);
      page = addr;

    }
  
  
  /* Obtention d un slab */
  slab = (struct vmem_slab*)virtmem_cache_alloc(slab_cache, VIRT_CACHE_DEFAULT);
  ASSERT_RETURN( slab!=NULL , EXIT_FAILURE);

  /* Initialisation du slab */
  slab->count = 0;
  slab->max_objects = (np*PAGING_PAGE_SIZE)/cache->size;
  slab->cache = cache;
  slab->n_pages = np;
  slab->start = page;
  LLIST_NULLIFY(slab->free_buf);

  /* Calcul l alignement */
  if (cache->align)
    {
      wasted = (np*PAGING_PAGE_SIZE)-(slab->max_objects*cache->size);
      if (wasted)
	{
	  slab->start += cache->align_offset;
	  cache->align_offset = (cache->align_offset+cache->align)%wasted;
	}
    }

  /* Relie le nouveau slab au cache */
  LLIST_ADD(cache->slabs_free,slab);

  /* Cree les bufctls et les fait pointer sur la page */
  for(i=0; i<slab->max_objects; i++)
    {
      bc = (struct vmem_bufctl*)virtmem_cache_alloc(bufctl_cache, VIRT_CACHE_DEFAULT);
      ASSERT_RETURN( bc!=NULL , EXIT_FAILURE);

      /* Initialise le bufctl */
      bc->base = slab->start + i*cache->size;
      bc->slab = slab;
     /* Applique le constructeur */
      if ( cache->constructor != NULL)
	{
	  cache->constructor((void*)(bc->base),cache->size);
	}
      /* Ajout a la liste du slab */
      LLIST_ADD(slab->free_buf,bc);

    }

  return EXIT_SUCCESS;
}
