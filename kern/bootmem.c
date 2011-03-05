/*  
 * Gestion de la memoire physique 
 * 
 */

#include <types.h>
#include "klib.h"
#include "start.h"
#include "bootmem.h"

/* Bitmap & Co */

PRIVATE u8_t phys_bitmap[BOOTMEM_PAGES/sizeof(u8_t)];
PRIVATE u32_t phys_bitmap_last_page;
PRIVATE u32_t phys_bitmap_last_offset;
PRIVATE int phys_region(u32_t size);

/* Macros */

#define SET_PAGE_USED(n)      phys_bitmap[n/sizeof(u8_t)] |= (1 << (n/PAGE_SIZE) % sizeof(u8_t))
#define SET_PAGE_FREE(n)      phys_bitmap[n/sizeof(u8_t)] &= ~(1 << (n/PAGE_SIZE) % sizeof(u8_t))
#define GET_PAGE_STATUS(n)    phys_bitmap[n/sizeof(u8_t)] & (1 << (n/PAGE_SIZE) % sizeof(u8_t))


/************** 
 * Allocation 
 **************/

PUBLIC void* bootmem_alloc(u32_t size)
{
  u32_t addr;
  
  /* Regarde si la taille nécessite plusieurs pages */
  if ( (PAGE_SIZE - phys_bitmap_last_offset) >= size )
    {
      /* On se contente de mettre à jour les variables globales */
      addr = (phys_bitmap_last_page*PAGE_SIZE) + phys_bitmap_last_offset;
      phys_bitmap_last_offset += size;
      /* Marque la page comme allouée (cas de la premiere allocation) */
      SET_PAGE_USED(phys_bitmap_last_page);
      /* Retourne l'adresse */
      return (void*)addr;
    }
  else
    {
      u32_t n,j;
      int i;
      
      /* Nombre de page, a calculer differemment selon les multiples */
      n = (size%PAGE_SIZE==0?size/PAGE_SIZE:size/PAGE_SIZE+1);
      
      /* Recupere la premiere page d une region libre */
      i = phys_region(n);

      /* Adresse de la region libre */
      if(i>=0)
	{
	  /* Marque la region comme occupee */
	  for(j=i;j<i+n;j++)
	    {
	      SET_PAGE_USED(j);
	    }
	  
	  /* Stocke l'adresse de base */
	  addr = i*PAGE_SIZE;
	  /* Met à jour les variables globales */
	  phys_bitmap_last_page = i+n-1;
	  /* Offset: 0 signifie PAGE_SIZE */
	  phys_bitmap_last_offset = (size%PAGE_SIZE==0?PAGE_SIZE:size%PAGE_SIZE);
	  /* Renvoie l'adresse de base */
	  return (void*)addr;
	  
	}	
      else
	{
	  /* Si on arrive la, il n y a pas assez de memoire */
	  bochs_print("Could not allocate %d bytes\n",size);
	  return (void*)0;
	}
    }
}


/************** 
 * Liberation 
 **************/

PUBLIC void bootmem_free(void* addr, u32_t size)
{
  u32_t pfn_start, pfn_end;
  u32_t i;

  /* Calcule les alignements des pages */
  pfn_start = ((u32_t)addr + PAGE_SIZE - 1)/PAGE_SIZE;
  pfn_end = ((u32_t)addr + size)/PAGE_SIZE;

  /* Libere uniquement les pages entieres */
  if (pfn_start < pfn_end)
    {
      for(i=pfn_start;i<pfn_end;i++)
	{
	  SET_PAGE_FREE(i);
	}
    }

  return;
}


/****************************************
 * Trouve une region libre (brute force)
 ****************************************/

PRIVATE int phys_region(u32_t size)
{
  int i,j=0;
  
  for(i=0; (i<BOOTMEM_PAGES-size); i++)
    {
      for(j=0;j<size && ~GET_PAGE_STATUS(i+j);j++);
      if (j>=size) break;
    }

  return ((j>=size) ?  i : -1);
}

/*****************
 * Initialisation
 *****************/

PUBLIC void bootmem_init()
{
  u32_t i;

  /* Marque les adresses basses comme allouees */
  for(i=0; i<bootinfo->kern_end/PAGE_SIZE+1; i++)
    {
      SET_PAGE_USED(i);
    }
  
  /* Ajuste les variables globales */
  phys_bitmap_last_page = i;
  phys_bitmap_last_offset = PAGE_SIZE;

  /* Marque les adresses hautes comme allouees */
  for(i=BIOS_ROM_START/PAGE_SIZE; i<BOOTMEM_END/PAGE_SIZE+1; i++)
    {
      SET_PAGE_USED(i);
    }

  return;
}
