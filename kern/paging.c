/*
 * Mise en place de la pagination
 *
 */

#include <types.h>
#include "const.h"
#include "klib.h"
#include "start.h"
#include "physmem.h"
#include "paging.h"


/*========================================================================
 * Initialisation
 *========================================================================*/

PUBLIC u8_t paging_init(void)
{
  physaddr_t p;
  u16_t i;
  struct pte* table;

  /* PD Noyau */
  kern_PD = (struct pde*)phys_alloc(PAGING_ENTRIES*sizeof(struct pde));
  /* Validite de l allocation */
  if ( kern_PD == NULL)
    {
      return EXIT_FAILURE;
    }

  /* Nullifie la page */
  klib_mem_set(0,(addr_t)kern_PD,PAGING_ENTRIES*sizeof(struct pde));

  /* Self Mapping */
  kern_PD[PAGING_SELFMAP].present = 1;
  kern_PD[PAGING_SELFMAP].rw = 1;
  kern_PD[PAGING_SELFMAP].user = 0;
  kern_PD[PAGING_SELFMAP].baseaddr = (physaddr_t)kern_PD >> PAGING_BASESHIFT;
  

  /* Pre allocation des pages tables pour l espace noyau afin de faciliter la sync avec les threads */
  for(i=0;i<CONST_KERN_HIGHMEM/CONST_PAGE_SIZE/PAGING_ENTRIES;i++)
    {
      /* Au cas ou ... */
      if ( (i == PAGING_SELFMAP)||(i>=PAGING_ENTRIES) )
	{
	  return EXIT_FAILURE;
	}

      /* Alloue une page physique */
      table = (struct pte*)phys_alloc(PAGING_ENTRIES*sizeof(struct pte));
      if( table == NULL)
	{
	  return EXIT_FAILURE;
	}

      /* Fait pointer le pde sur la nouvelle page */
      kern_PD[i].present = 1;
      kern_PD[i].rw = 1;
      kern_PD[i].user = 0;
      kern_PD[i].baseaddr = (((physaddr_t)table)>>PAGING_BASESHIFT);

      /* Nullifie le page table */
      klib_mem_set(0,(addr_t)table,PAGING_ENTRIES*sizeof(struct pte));
    }

  
  for(p=PAGING_ALIGN_INF(0);
      p<PAGING_ALIGN_SUP(bootinfo->kern_end);
      p+=CONST_PAGE_SIZE)
    {
      if (paging_map((virtaddr_t)p, p, PAGING_SUPER|PAGING_IDENTITY) == EXIT_FAILURE)
	{
	  return EXIT_FAILURE;
	}
    }

  for(p=PAGING_ALIGN_INF(CONST_PAGE_NODE_POOL_ADDR);
      p<PAGING_ALIGN_SUP(CONST_PAGE_NODE_POOL_ADDR+((bootinfo->mem_total) >> CONST_PAGE_SHIFT)*sizeof(struct ppage_desc));
      p+=CONST_PAGE_SIZE)
    {
      if (paging_map((virtaddr_t)p, p, PAGING_SUPER|PAGING_IDENTITY) == EXIT_FAILURE)
	{
	  return EXIT_FAILURE;
	}
    }




  /* Charge le Kernel  Page Directory */
  klib_load_CR3((physaddr_t)kern_PD);

  /* Activation de la pagination */
  klib_set_pg_cr0();

  return EXIT_SUCCESS;
}


/*========================================================================
 * Mapping
 *========================================================================*/

PUBLIC u8_t paging_map(virtaddr_t vaddr, physaddr_t paddr, u8_t flags)
{
  struct pde* pd;
  struct pte* table;
  u16_t pde,pte;


  /* Recupere le pd, pde et pte associe */
  pde = PAGING_GET_PDE(vaddr);
  pte = PAGING_GET_PTE(vaddr);
  pd = (flags&PAGING_IDENTITY?kern_PD:(struct pde*)PAGING_GET_PD());

  /* Interdit le pde du self map */
  if ( pde == PAGING_SELFMAP )
    {
      return EXIT_FAILURE;
    }
      
      
  /* Si le pde n'existe pas, on le cree */
  if (!(pd[pde].present))
    {
      /* Alloue une page physique */
      table = (struct pte*)phys_alloc(PAGING_ENTRIES*sizeof(struct pte));
      if( table == NULL)
	{
	  return EXIT_FAILURE;
	}

      /* Fait pointer le pde sur la nouvelle page */
      pd[pde].present = 1;
      pd[pde].rw = 1;
      pd[pde].user = (flags&PAGING_SUPER?0:1);
      pd[pde].baseaddr = (((physaddr_t)table)>>PAGING_BASESHIFT);

      /* Nullifie le page table */
      klib_mem_set(0,(flags&PAGING_IDENTITY?(addr_t)table:(addr_t)PAGING_GET_PT(pde)),PAGING_ENTRIES*sizeof(struct pte));
    }

  /* Ici, la table existe forcement */
  table = (struct pte*)(flags&PAGING_IDENTITY?pd[pde].baseaddr<<PAGING_BASESHIFT:PAGING_GET_PT(pde));

  /* Si le pte est present, l adresse est deja mappee */
  if (table[pte].present)
    {
      return EXIT_FAILURE;
    }
 
  /* Fait pointer le pte sur la page physique */
  table[pte].present = 1;
  table[pte].rw = 1;
  table[pte].user = (flags&PAGING_SUPER?0:1);
  table[pte].baseaddr = paddr >> PAGING_BASESHIFT;

  /* Indique un mappage de notre adresse physique */
  phys_map(paddr);

  /* Indique un mappage dans la page table */
  phys_map(pd[pde].baseaddr << PAGING_BASESHIFT);
  

  return EXIT_SUCCESS;
}


/*========================================================================
 * Unmapping
 *========================================================================*/

PUBLIC u8_t paging_unmap(virtaddr_t vaddr)
{
  struct pde* pd;
  struct pte* table;
  u16_t pde,pte;


  /* Recupere le pd, pde et pte associe */
  pde = PAGING_GET_PDE(vaddr);
  pte = PAGING_GET_PTE(vaddr);
  pd = (struct pde*)PAGING_GET_PD();

  /* Interdit le pde du self map et verifie l'existence du pde */
  if ( (pde == PAGING_SELFMAP)||(!pd[pde].present) )
    {
      return EXIT_FAILURE;
    }

  /* Recupere la table */
  table = (struct pte*)(PAGING_GET_PT(pde));

  /* Si le pte n'existe pas, on retourne */
  if (!table[pte].present)
    {
      return EXIT_FAILURE;
    }
 
  /* Demap/Libere la page physique */
  phys_unmap(table[pte].baseaddr<<PAGING_BASESHIFT, PHYS_UNMAP_DEFAULT);

  /* Nullifie la structure */
  table[pte].present=0;
  table[pte].rw=0;
  table[pte].user=0;
  table[pte].baseaddr=0;

  /* Decremente le compteur de maps de la table et libere uniquement les pages tables hors espace noyau */
  if (phys_unmap(pd[pde].baseaddr << PAGING_BASESHIFT, 
		 (pde<CONST_KERN_HIGHMEM/CONST_PAGE_SIZE/PAGING_ENTRIES?PHYS_UNMAP_NOFREE:PHYS_UNMAP_DEFAULT)) == PHYS_UNMAP_FREE)
    {
      /* Si la page de la table est liberee, on nullifie le pd[pde] */
      pd[pde].present = 0;
      pd[pde].rw=0;
      pd[pde].user=0;
      pd[pde].baseaddr=0;

    }
  
  return EXIT_SUCCESS;
	   
}
  

/*========================================================================
 * Conversion Virtuelle vers Physique
 *========================================================================*/

PUBLIC physaddr_t paging_virt2phys(virtaddr_t vaddr)
{
  struct pde* pd;
  struct pte* table;
  u16_t pde,pte;
  u32_t offset;

  /* Recupere le pd, pde et pte associe */
  pde = PAGING_GET_PDE(vaddr);
  pte = PAGING_GET_PTE(vaddr);
  pd = (struct pde*)PAGING_GET_PD();

  /* Offset dans le page physique */
  offset = vaddr & PAGING_OFFMASK;

  /* Si le pde n'existe pas, on retourne */
  if ( !pd[pde].present )
    {
      return 0;
    }
 
  /* Recupere la table */
  table = (struct pte*)(PAGING_GET_PT(pde));

  /* Si le pte n'existe pas, on retourne */
  if ( !table[pte].present )
    {
      return 0;
    }
 
  /* Retourne l'adresse physique */
  return (((table[pte].baseaddr)<<PAGING_BASESHIFT)+offset);

}
