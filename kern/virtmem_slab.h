/*
 * Virtmem_slab.h
 * Header de virtmem_slab.c
 *
 */

#ifndef VIRTMEM_SLAB_H
#define VIRTMEM_SLAB_H


/************
 * Includes
 ***********/

#include <types.h>


/**************
 * Constantes
 **************/

#define VIRT_CACHE_NAMELEN     32

#define VIRT_CACHE_GROWSHIFT   3   /* 2^3 = 8 */

/**************
 * Structures
 **************/


/* Bufctl */
struct vmem_bufctl
{
  virtaddr_t base;
  struct vmem_slab* slab;
  struct vmem_bufctl* next;
  struct vmem_bufctl* prev;
} __attribute__ ((packed));


/* Slab */
struct vmem_slab
{
  u16_t count;
  u16_t max_objects;
  struct vmem_bufctl* free_buf;
  struct vmem_cache* cache;
  struct vmem_slab* next;
  struct vmem_slab* prev;
} __attribute__ ((packed));


/* Cache */
struct vmem_cache
{
  char name[VIRT_CACHE_NAMELEN];
  u16_t size;
  u16_t align;
  void (*constructor)(void*,u32_t);
  void (*destructor)(void*,u32_t);
  struct vmem_slab* slabs_free;
  struct vmem_slab* slabs_partial;
  struct vmem_slab* slabs_full;
  struct vmem_cache* next;
  struct vmem_cache* prev;
} __attribute__ ((packed));



/**************
 * Prototypes
 **************/

PUBLIC void virtmem_cache_init(void);
PUBLIC void* virtmem_cache_alloc(struct vmem_cache* cache);
PUBLIC void virtmem_cache_free(struct vmem_cache* cache, void* buf);
PUBLIC struct vmem_cache* virtmem_cache_create(const char* name, u16_t size, u16_t align, void (*ctor)(void*,u32_t), void (*dtor)(void*,u32_t));

#endif
