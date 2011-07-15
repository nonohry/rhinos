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
};


/* Slab */
struct vmem_slab
{
  u32_t count;
  struct vmem_bufctl* free_buf;
  struct vmem_cache* cache;
  struct vmem_slab* next;
  struct vmem_slab* prev;
};


/* Cache */
struct vmem_cache
{
  char name[VIRT_CACHE_NAMELEN];
  u32_t size;
  u32_t align;
  void (*constructor)(void*,u32_t);
  void (*destructor)(void*,u32_t);
  struct vmem_slab* slabs_free;
  struct vmem_slab* slabs_partial;
  struct vmem_slab* slabs_full;
  struct vmem_cache* next;
  struct vmem_cache* prev;
};


#endif
