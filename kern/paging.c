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
  /* Validite de l allocation */
  if (kern_PD == NULL)
    {
      bochs_print("Cannot allocate Kernel PD\n");
      return;
    }

  /* Nullifie la page */
  mem_set(0,(u32_t)kern_PD,PAGING_ENTRIES*sizeof(struct pde));

  /* Self Mapping */
  kern_PD[PAGING_SELFMAP].present = 1;
  kern_PD[PAGING_SELFMAP].rw = 1;
  kern_PD[PAGING_SELFMAP].user = 0;
  kern_PD[PAGING_SELFMAP].baseaddr = (physaddr_t)kern_PD >> PAGING_BASESHIFT;
  
  /* Identity Mapping */
  paging_identityMapping(0,bootinfo->kern_end);
  paging_identityMapping(0x9FC00,PPAGE_NODE_POOL_ADDR+bootinfo->mem_ram_pages*sizeof(struct ppage_node));

  /* Charge le Kernel  Page Directory */
  load_CR3((physaddr_t)kern_PD);

  /* Activation de la pagination */
  set_pg_cr0();

  return;
}


/***********
 * Mapping
 **********/

PUBLIC u8_t paging_map(struct pde* pd, virtaddr_t vaddr, physaddr_t paddr, u8_t super)
{
  struct pte* table;
  u16_t pde,pte;

  // DEBUG
  bochs_print("pd: 0x%x\n",(u32_t)pd);

  /* Recupere le pde et pte associe */
  pde = PAGING_GET_PDE(vaddr);
  pte = PAGING_GET_PTE(vaddr);

  // DEBUG
  bochs_print("pde: %d, pte=%d\n",pde,pte);

  /* Interdit le pde du self map */
  if (pde == PAGING_SELFMAP)
    {
      bochs_print("Cannot map virtual address (self map)\n");
      return EXIT_FAILURE;
    }

  /* Si le pde n'existe pas, on le cree */
  if (!(pd[pde].present))
    {
      // DEBUG
      bochs_print("pd[pde].present=0\n");

      /* Alloue une page physique */
      table = (struct pte*)phys_alloc(PAGING_ENTRIES*sizeof(struct pte));
      if (table == NULL)
	{
	  bochs_print("Unable to allocate %d bytes\n",PAGING_ENTRIES*sizeof(struct pte));
	  return EXIT_FAILURE;
	}
      

      // DEBUG
      bochs_print("new table:0x%x\n",(u32_t)table);


      /* Fait pointer le pde sur la nouvelle page */
      pd[pde].present = 1;
      pd[pde].rw = 1;
      pd[pde].user = (super?0:1);
      pd[pde].baseaddr = (((physaddr_t)table)>>PAGING_BASESHIFT);

      // DEBUG
      bochs_print("pd[pde].baseaddr=0x%x\n",((physaddr_t)table) >> PAGING_BASESHIFT);


    }

  /* Ici, la table existe forcement */
  table = (struct pte*)(pd[pde].baseaddr << PAGING_BASESHIFT);

  // DEBUG
  bochs_print("table:0x%x\n",(u32_t)table);

  /* Si le pte est present, l adresse est deja mappee */
  if (table[pte].present)
    {

      // DEBUG
      bochs_print("table[pte].present=1\n");

      /* Libere la page precedemment allouee */
      phys_free( (physaddr_t*)(table[pte].baseaddr<<PAGING_BASESHIFT) );
    }

  /* Fait pointer le pte sur la page physique */
  table[pte].present = 1;
  table[pte].rw = 1;
  table[pte].user = (super?0:1);
  table[pte].baseaddr = paddr >> PAGING_BASESHIFT;

  // DEBUG
  bochs_print("table[pte].baseaddr=0x%x\n",paddr >> PAGING_BASESHIFT);


  return EXIT_SUCCESS;
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
	  /* Validite de l allocation */
	  if (table == NULL)
	    {
	      bochs_print("Unable to allocate Page Table\n");
	      return;
	    }

	  /* Nullifie la page */
	  mem_set(0,(u32_t)table,PAGING_ENTRIES*sizeof(struct pte));

	  /* Attributs du PDE */
	  kern_PD[pde].baseaddr = (((physaddr_t)table)>>PAGING_BASESHIFT);
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
