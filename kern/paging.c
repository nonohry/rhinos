/**

   paging.c
   ========

   Pagination management

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - const.h
   - klib.h
   - start.h      : kernel boundaries needed
   - physmem.h    : physical memory needed
   - paging.h     : self header

**/


#include <define.h>
#include <types.h>
#include "const.h"
#include "klib.h"
#include "start.h"
#include "physmem.h"
#include "paging.h"


/**

   Funtion: u8_t paging_init(void)
   -------------------------------

   Pagination initialization.
   Create the kernel page directory and physically allocate all the page tables corresponding to the kernel space.
   This trick makes the synchronization between an user space and the kernel space a lot easier.
   Then, all the kernel space in used is identity mapping before activating the pagination.

**/

PUBLIC u8_t paging_init(void)
{
  physaddr_t p;
  u16_t i;
  struct pte* table;

  /* Kernel page directory */
  kern_PD = (struct pde*)phys_alloc(PAGING_ENTRIES*sizeof(struct pde));
  if ( kern_PD == NULL)
    {
      return EXIT_FAILURE;
    }

  
  /* Clean the page directory */
  klib_mem_set(0,(addr_t)kern_PD,PAGING_ENTRIES*sizeof(struct pde));

  /* Self Mapping */
  kern_PD[PAGING_SELFMAP].present = 1;
  kern_PD[PAGING_SELFMAP].rw = 1;
  kern_PD[PAGING_SELFMAP].user = 0;
  kern_PD[PAGING_SELFMAP].baseaddr = (physaddr_t)kern_PD >> PAGING_BASESHIFT;
  

  /* Pre allocate page tables */
  for(i=0;i<CONST_KERN_HIGHMEM/CONST_PAGE_SIZE/PAGING_ENTRIES;i++)
    {
      /* We never know ... */
      if ( (i == PAGING_SELFMAP)||(i>=PAGING_ENTRIES) )
	{
	  return EXIT_FAILURE;
	}
         
      /* Allocate a physical page */
      table = (struct pte*)phys_alloc(PAGING_ENTRIES*sizeof(struct pte));
      if( table == NULL)
	{
	  return EXIT_FAILURE;
	}
   
      /* Point PDE to that new physical page */
      kern_PD[i].present = 1;
      kern_PD[i].rw = 1;
      kern_PD[i].user = 0;
      kern_PD[i].baseaddr = (((physaddr_t)table)>>PAGING_BASESHIFT);

      /* Clean page table */
      klib_mem_set(0,(addr_t)table,PAGING_ENTRIES*sizeof(struct pte));
    }

  /* Identity map kernel code and data to activate pagination seamlessly */
  for(p=PAGING_ALIGN_INF(CONST_KERN_START);
      p<PAGING_ALIGN_SUP(CONST_KERN_END);
      p+=CONST_PAGE_SIZE)
    {
      if (paging_map((virtaddr_t)p, p, PAGING_SUPER|PAGING_IDENTITY) == EXIT_FAILURE)
	{
	  return EXIT_FAILURE;
	}
    }
 
  /* Identity map physical memory manager structures too */
  for(p=PAGING_ALIGN_INF(CONST_PAGE_NODE_POOL_ADDR);
      p<PAGING_ALIGN_SUP(CONST_PAGE_NODE_POOL_ADDR+((start_mem_total) >> CONST_PAGE_SHIFT)*sizeof(struct ppage_desc));
      p+=CONST_PAGE_SIZE)
    {
      if (paging_map((virtaddr_t)p, p, PAGING_SUPER|PAGING_IDENTITY) == EXIT_FAILURE)
	{
	  return EXIT_FAILURE;
	}
    }
  
  /* Load kernel page directory */
  klib_load_CR3((physaddr_t)kern_PD);

  /* Activate pagination */
  klib_set_pg_cr0();

  return EXIT_SUCCESS;
}



/**

   Function: u8_t paging_map(virtaddr_t vaddr, physaddr_t paddr, u8_t flags)
   -------------------------------------------------------------------------


   Associate `vaddr` and `paddr`.
   `flags` switches between an  user or kernel mode mapping.

**/


PUBLIC u8_t paging_map(virtaddr_t vaddr, physaddr_t paddr, u8_t flags)
{
  struct pde* pd;
  struct pte* table;
  u16_t pde,pte;


  /* Get right page directory, page directory entry and page table entry linked to `vaddr` */
  pde = PAGING_GET_PDE(vaddr);
  pte = PAGING_GET_PTE(vaddr);
  pd = (flags&PAGING_IDENTITY?kern_PD:(struct pde*)PAGING_GET_PD());

  /* Cannot map self ampping area */
  if ( pde == PAGING_SELFMAP )
    {
      return EXIT_FAILURE;
    }
      
      
  /* If there is no page directory entry, create it */
  if (!(pd[pde].present))
    {
      /* Allocate a physical page for page table */
      table = (struct pte*)phys_alloc(PAGING_ENTRIES*sizeof(struct pte));
      if( table == NULL)
	{
	  return EXIT_FAILURE;
	}

      /* Point the page directory entry to that page table */
      pd[pde].present = 1;
      pd[pde].rw = 1;
      pd[pde].user = (flags&PAGING_SUPER?0:1);
      pd[pde].baseaddr = (((physaddr_t)table)>>PAGING_BASESHIFT);

      /* Nullify page table */
      klib_mem_set(0,(flags&PAGING_IDENTITY?(addr_t)table:(addr_t)PAGING_GET_PT(pde)),PAGING_ENTRIES*sizeof(struct pte));
    }

  /* Here, the page table necessary exists, so get it */
  table = (struct pte*)(flags&PAGING_IDENTITY?pd[pde].baseaddr<<PAGING_BASESHIFT:PAGING_GET_PT(pde));

  /* If the page table entry exists, mapping already exists */
  if (table[pte].present)
    {
      return EXIT_FAILURE;
    }
 
  /* Point the page table entry to the desired physical page */
  table[pte].present = 1;
  table[pte].rw = 1;
  table[pte].user = (flags&PAGING_SUPER?0:1);
  table[pte].baseaddr = paddr >> PAGING_BASESHIFT;

  /* Indicate a mapping for the physical page */
  phys_map(paddr);

  /* Indicate a mapping for the page table.
     In fact, we use this indication as a counter to determine the number 
     of physical pages referenced by the page table */
  phys_map(pd[pde].baseaddr << PAGING_BASESHIFT);
  

  return EXIT_SUCCESS;
}


/**

   Function: u8_t paging_unmap(virtaddr_t vaddr)
   ---------------------------------------------


   Remove the association virtual/physical corresponding to the virtaul address `vaddr` in the current page directory
   If the mapping is the last in a user page table, the page table is also freed.

**/


PUBLIC u8_t paging_unmap(virtaddr_t vaddr)
{
  struct pde* pd;
  struct pte* table;
  u16_t pde,pte;


  /* Get current page directory, page directory entry and page table entry linked to `vaddr` */ 
  pde = PAGING_GET_PDE(vaddr);
  pte = PAGING_GET_PTE(vaddr);
  pd = (struct pde*)PAGING_GET_PD();

  /* Check page table entry existence as well as validity in regards to self mapping */
  if ( (pde == PAGING_SELFMAP)||(!pd[pde].present) )
    {
      return EXIT_FAILURE;
    }

  /* Get page table */
  table = (struct pte*)(PAGING_GET_PT(pde));

  /* No page table ? Error */
  if (!table[pte].present)
    {
      return EXIT_FAILURE;
    }
 
  /* Unmap and free physical page */
  phys_unmap(table[pte].baseaddr<<PAGING_BASESHIFT, PHYS_UNMAP_DEFAULT);

  /* Nullify the corresponding page table entry */
  table[pte].present=0;
  table[pte].rw=0;
  table[pte].user=0;
  table[pte].baseaddr=0;

  /* Decrement page table mapping counter and free user space page table if needed */
  if (phys_unmap(pd[pde].baseaddr << PAGING_BASESHIFT, 
		 (pde<CONST_KERN_HIGHMEM/CONST_PAGE_SIZE/PAGING_ENTRIES?PHYS_UNMAP_NOFREE:PHYS_UNMAP_DEFAULT)) == PHYS_UNMAP_FREE)
    {
      /* Nullify page directory entry if the page table is freed */
      pd[pde].present = 0;
      pd[pde].rw=0;
      pd[pde].user=0;
      pd[pde].baseaddr=0;

    }
  
  return EXIT_SUCCESS;
	   
}
  

/**
   
   Function: physaddr_t paging_virt2phys(virtaddr_t vaddr)
   -------------------------------------------------------

   Return the physical address mapped to `vaddr`. 
   If such a mapping does not exist, return 0.

**/

PUBLIC physaddr_t paging_virt2phys(virtaddr_t vaddr)
{
  struct pde* pd;
  struct pte* table;
  u16_t pde,pte;
  u32_t offset;

  /* Get current page directory, page directory entry and page table entry linked to `vaddr` */ 
  pde = PAGING_GET_PDE(vaddr);
  pte = PAGING_GET_PTE(vaddr);
  pd = (struct pde*)PAGING_GET_PD();

  /* Offset in physical page */
  offset = vaddr & PAGING_OFFMASK;

  /* No page directory entry ? Error */
  if ( !pd[pde].present )
    {
      return 0;
    }
 
  /* Get page table */
  table = (struct pte*)(PAGING_GET_PT(pde));

  /* No page table entry ? Error */
  if ( !table[pte].present )
    {
      return 0;
    }
 
  /* Return physical address */
  return (((table[pte].baseaddr)<<PAGING_BASESHIFT)+offset);

}
