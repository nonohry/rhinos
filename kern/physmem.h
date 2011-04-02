/*
 * Header de physmem.c
 *
 */

#ifndef PHYSMEM_C
#define PHYSMEM_C


/**************
 * Constantes 
 **************/

#define PPAGE_MAX_BUDDY  20  /* 32 - 12 = 20 */
#define PPAGE_SHIFT      12  /* 2^12=4096 */


/***************
 * Structures 
 ***************/

struct ppage_node 
{
  u32_t start;
  u32_t size;
  u8_t  index;
  struct ppage_node* prev;
  struct ppage_node* next;
};


/***************
 * Prototypes 
 ***************/

PUBLIC struct ppage_node* ppage_free[PPAGE_MAX_BUDDY];
PUBLIC struct ppage_node* ppage_allocated;

PUBLIC void physmem_init(void);
PUBLIC void* phys_alloc(u32_t size);

#endif
