/**

   paging.c
   ========

   Virtual Memory Paging management

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - x86_lib.h
   - serial.h
   - vm_paging.h   : self header
 
**/


#include <define.h>
#include <types.h>
#include "x86_lib.h"
#include "serial.h"
#include "vm_paging.h"



/**

   Function: u8_t paging_setup(addr_t base)
   ----------------------------------------

   Setup paging, using `base` as the start of a memory pool to create objects required for initilization.

   Create the kernel page directory and physically allocate all the page tables corresponding to the kernel space.
   This trick makes the synchronization between an user space and the kernel space a lot easier.
   Then, all the kernel space in used is identity mapping before activating the pagination.

**/


PUBLIC u8_t paging_setup(physaddr_t base)
{
  u16_t i;
  struct pte* table;

  serial_printf("init base: 0x%x\n",base);

  /* Allocate kernel page directory */
  kern_pd = (struct pde*)base;

  /* Update `base` */
  base += VM_PAGING_ENTRIES*sizeof(struct pde);

  serial_printf("base after kern_pd: 0x%x\n",base);

  /* Clean page directory */
  x86_mem_set(0,(addr_t)kern_pd,VM_PAGING_ENTRIES*sizeof(struct pde));

  /* Self Mapping */
  kern_pd[VM_PAGING_SELFMAP].present = 1;
  kern_pd[VM_PAGING_SELFMAP].rw = 1;
  kern_pd[VM_PAGING_SELFMAP].user = 0;
  kern_pd[VM_PAGING_SELFMAP].baseaddr = (physaddr_t)kern_pd >> VM_PAGING_BASESHIFT;

  /* Pre allocate page tables in kernel space */
  for(i=0;i<X86_CONST_KERN_HIGHMEM/X86_CONST_PAGE_SIZE/VM_PAGING_ENTRIES;i++)
    {
      /* We never know ... */
      if ( (i == VM_PAGING_SELFMAP)||(i>=VM_PAGING_ENTRIES) )
	{
	  return EXIT_FAILURE;
	}
         
      /* Allocate a page table */
      table = (struct pte*)base;
      /* Update `base` */
      base += VM_PAGING_ENTRIES*sizeof(struct pte);
      serial_printf("base after page table n°%d: 0x%x\n",i,base);

      /* Point page directory entry `i` to that new physical page */
      kern_pd[i].present = 1;
      kern_pd[i].rw = 1;
      kern_pd[i].user = 0;
      kern_pd[i].baseaddr = (((physaddr_t)table)>>VM_PAGING_BASESHIFT);

      /* Clean page table */
      x86_mem_set(0,(addr_t)table,VM_PAGING_ENTRIES*sizeof(struct pte));
    }



  return EXIT_SUCCESS;
}
