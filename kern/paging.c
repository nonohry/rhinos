/*
 * Mise en place de la pagination
 *
 */

#include <types.h>
#include "klib.h"
#include "start.h"
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
  kern_PD = (struct pde*)phys_alloc(PAGING_ENTRIES*sizeof(struct pde));

  /* Self Mapping */
  kern_PD[PAGING_SELFMAP].present = 1;
  kern_PD[PAGING_SELFMAP].rw = 1;
  kern_PD[PAGING_SELFMAP].user = 0;
  kern_PD[PAGING_SELFMAP].baseaddr = (physaddr_t)kern_PD;
  
  /* Identity Mapping */
  paging_identityMapping(0,bootinfo->kern_end);
  paging_identityMapping(0x9FC00,PPAGE_NODE_POOL_ADDR+bootinfo->mem_ram_pages*sizeof(struct ppage_node));

  /* Charge le Kernel  Page Directory */
  load_CR3((physaddr_t)kern_PD);

  /* Activation de la pagination */
  set_pg_cr0();

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
      struct pte* table;

      /* Recupere les supposes PDE et PTE */
      u16_t pde = PAGING_GET_PDE(p);
      u16_t pte = PAGING_GET_PTE(p);
     
      /* Cree le PDE s il n existe pas */
      if (!kern_PD[pde].present)
	{
	  /* La table vers laquelle pointer */
	  table = (struct pte*)phys_alloc(PAGING_ENTRIES*sizeof(struct pte));
	  /* Attributs du PDE */
	  kern_PD[pde].baseaddr = (((u32_t)table)>>PAGING_BASESHIFT);
	  kern_PD[pde].present = 1;
	  kern_PD[pde].rw = 1;
	  kern_PD[pde].user = 0;
	}

      /* Recupere la table du PDE */
      table = (struct pte*)(kern_PD[pde].baseaddr<<PAGING_BASESHIFT);
      /* Ajuste les attributs et l'adresse pointee par le PTE */
      table[pte].present = 1;
      table[pte].rw = 1;
      table[pte].user = 0;
      table[pte].baseaddr = (p>>PAGING_BASESHIFT);    

    }

  return;
}
