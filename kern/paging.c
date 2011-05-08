/*
 * Mise en place de la pagination
 *
 */

#include <types.h>
#include "physmem.h"
#include "paging.h"


/**********************
 * Definitions PRIVATE
 **********************/

PRIVATE void paging_identityMapping(physaddr_t start, physaddr_t end);


/*****************
 * Initialisation
 *****************/

PUBLIC void paging_init(void)
{
  /* PD Noyau */
  kern_PD = (pagedir_t*)phys_alloc(sizeof(pagedir_t));

  /* Self Mapping */
  kern_PD[PAGING_SELFMAP]->present = 1;
  kern_PD[PAGING_SELFMAP]->rw = 1;
  kern_PD[PAGING_SELFMAP]->user = 0;
  
  /* Identity Mapping */
  paging_identityMapping(0,9000);

  return;
}



/******************************
 * Identity Mapping d une zone
 ******************************/

PRIVATE void paging_identityMapping(physaddr_t start, physaddr_t end)
{
  physaddr_t p;

  /* Parcours les adresses de PAGE_SIZE en PAGE_SIZE */
  for(p=PAGING_ALIGN_INF(start);p<PAGING_ALIGN_SUP(end);p+=PAGE_SIZE)
    {
      tabledir_t* table;

      /* Recupere les supposes PDE et PTE */
      u16_t pde = PAGING_GET_PDE(p);
      u16_t pte = PAGING_GET_PTE(p);
     
      /* Cree le PDE s il n existe pas */
      if (!kern_PD[pde]->present)
	{
	  /* La table vers laquelle pointer */
	  table = (tabledir_t*)phys_alloc(sizeof(tabledir_t));
	  /* Attributs du PDE */
	  kern_PD[pde]->baseaddr = (u32_t)table;
	  kern_PD[pde]->present = 1;
	  kern_PD[pde]->rw = 1;
	  kern_PD[pde]->user = 0;
	}

      /* Recupere la table du PDE */
      table = (tabledir_t*)(kern_PD[pde]->baseaddr);
      /* Ajuste les attributs et l'adresse pointee par le PTE */
      table[pte]->present = 1;
      table[pte]->rw = 1;
      table[pte]->user = 0;
      table[pte]->baseaddr = p;    

    }

  return;
}
