/*
 * Gestion de la memoire physique
 *
 */


#include <types.h>
#include <llist.h>
#include "klib.h"
#include "bootmem.h"
#include "physmem.h"


/***********************
 * Declaration Private
 ***********************/

PRIVATE u8_t phys_powerof2(u32_t n);


/*****************
 * Initialisation
 *****************/

PUBLIC void physmem_init(void)
{
  u8_t i;

  /* Nullifie les structures de pages */  
  for(i=0; i<PPAGE_MAX_BUDDY; i++)
    {
      LLIST_NULLIFY(ppage_free[i]);
    }
  LLIST_NULLIFY(ppage_allocated);

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
  
  k = phys_powerof2(size);
  ind = k - PPAGE_SHIFT;
  
  for(i=ind;LLIST_ISNULL(ppage_free[i])&&(i<PPAGE_MAX_BUDDY);i++)
    {}

  if (i>=PPAGE_MAX_BUDDY)
    {
      bochs_print("Can't allocate %d bytes !\n",size);
      return NULL;
    }
  
  for(j=i;j>ind;j--)
    {
      struct ppage_node *n1;
     
      node = LLIST_GETHEAD(ppage_free[j]);
      LLIST_REMOVE(ppage_free[j],node);

      n1 = (struct ppage_node*)boot_alloc(sizeof(struct ppage_node));

      n1->start = node->start;
      n1->size = node->size/2;
      n1->index = node->index/2;

      node->start = node->start + node->size/2;
      node->size = node->size/2;
      node->index = node->index/2;

      LLIST_ADD(ppage_free[j-1],n1);
      LLIST_ADD(ppage_free[j-1],node);

    }

  node = LLIST_GETHEAD(ppage_free[ind]);
  LLIST_REMOVE(ppage_free[ind],node);
  LLIST_ADD(ppage_allocated,node);
  
  return (void*)node->start;
}

