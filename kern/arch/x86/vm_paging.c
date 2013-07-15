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

   Constants: paging relatives
   ---------------------------

**/


#define VM_PAGING_ENTRIES      1024
#define VM_PAGING_DIRSHIFT     22
#define VM_PAGING_TBLSHIFT     12
#define VM_PAGING_TBLMASK      0x3FF
#define VM_PAGING_OFFMASK      0xFFF
#define VM_PAGING_BASESHIFT    12

#define VM_PAGING_USER         0
#define VM_PAGING_SUPER        1
#define VM_PAGING_IDENTITY     2

#define VM_PAGING_SELFMAP      0x3FF




/**

   Structure: struct pde
   ---------------------

   Describe an 32 bits entry in page directory.
   Members are:

     - present   : page in memory or not
     - rw        : read/write access 
     - user      : user/supervisor right
     - pwt       : write through
     - pcd       : cache disabled
     - accessed  : page accessed
     - zero      : nil
     - pagesize  : page size
     - global    : global page
     - available : available bits
     - baseaddr  : page table physical address

**/

PUBLIC struct pde
{
  u32_t present   :1  ;
  u32_t rw        :1  ;
  u32_t user      :1  ;
  u32_t pwt       :1  ;
  u32_t pcd       :1  ;
  u32_t accessed  :1  ;
  u32_t zero      :1  ;
  u32_t pagesize  :1  ;
  u32_t global    :1  ;
  u32_t available :3  ;
  u32_t baseaddr  :20 ;
}__attribute__ ((packed));


/**

   Structure: struct pte
   ---------------------

   Describe an 32 bits entry in page table.
   Members are:

     - present   : page in memory or not
     - rw        : read/write access 
     - user      : user/supervisor right
     - pwt       : write through
     - pcd       : cache disabled
     - accessed  : page accessed
     - dirty     : page was written
     - zero      : nil
     - global    : global page
     - available : available bits
     - baseaddr  : physical page address

**/

PUBLIC struct pte
{
  u32_t present   :1  ;
  u32_t rw        :1  ;
  u32_t user      :1  ;
  u32_t pwt       :1  ;
  u32_t pcd       :1  ;
  u32_t accessed  :1  ;
  u32_t dirty     :1  ;
  u32_t zero      :1  ;
  u32_t global    :1  ;
  u32_t available :3  ;
  u32_t baseaddr  :20 ;
}__attribute__ ((packed));



/**

   Macro: VM_PAGING_GET_PDE(__addr)
   -----------------------------

   Extract the page directory entry number from a virtual address

**/

#define VM_PAGING_GET_PDE(__addr)				\
  ( (__addr) >> VM_PAGING_DIRSHIFT )



/**

   Macro: VM_PAGING_GET_PTE(__addr)
   -----------------------------
   
   Extract the page table entry number from a virtual address

**/

#define VM_PAGING_GET_PTE(__addr)				\
  ( ((__addr) >> VM_PAGING_TBLSHIFT)&VM_PAGING_TBLMASK )


/**

   Macro: VM_PAGING_GET_PD()
   ----------------------

   Return virtual address of current page directory thanks to self maping

**/
 
#define VM_PAGING_GET_PD()							\
  ( (virtaddr_t)(VM_PAGING_SELFMAP<<VM_PAGING_DIRSHIFT) + (virtaddr_t)(VM_PAGING_SELFMAP<<VM_PAGING_TBLSHIFT) )


/**

   Macro: VM_PAGING_GET_PT(__i)
   -------------------------

   Return virtual address of page table pointed by the entry `i` 
   in current page directory thanks to self maping

**/

#define VM_PAGING_GET_PT(__i)						\
  ( (virtaddr_t)(VM_PAGING_SELFMAP<<VM_PAGING_DIRSHIFT) + (virtaddr_t)((__i)<<VM_PAGING_TBLSHIFT) )




/**

   Global: kern_pd
   ---------------

   Kernel page directory


**/


PUBLIC struct pde* kern_pd;


/**

   Function: u8_t paging_setup(physaddr_t* base)
   ---------------------------------------------

   Setup paging, using `base` as the start of a memory pool to create objects required for initilization.

   Create the kernel page directory and physically allocate all the page tables corresponding to the kernel space.
   This trick makes the synchronization between an user space and the kernel space a lot easier.
   Then, all the kernel space in used is identity mapping before activating the pagination.

**/


PUBLIC u8_t vm_paging_setup(physaddr_t* base)
{
  u16_t i;
  physaddr_t p;
  struct pte* table;

  /* Allocate kernel page directory */
  kern_pd = (struct pde*)*base;

  /* Update `base` */
  *base += VM_PAGING_ENTRIES*sizeof(struct pde);

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
      table = (struct pte*)*base;
      /* Update `base` */
      *base += VM_PAGING_ENTRIES*sizeof(struct pte);
  
      /* Point page directory entry `i` to that new physical page */
      kern_pd[i].present = 1;
      kern_pd[i].rw = 1;
      kern_pd[i].user = 0;
      kern_pd[i].baseaddr = (((physaddr_t)table)>>VM_PAGING_BASESHIFT);

      /* Clean page table */
      x86_mem_set(0,(addr_t)table,VM_PAGING_ENTRIES*sizeof(struct pte));
    }



  /* Identity-map kernel space */
  for(p=X86_ALIGN_INF(X86_CONST_KERN_START);
      p<X86_ALIGN_SUP(*base);
      p+=X86_CONST_PAGE_SIZE)
    {
      if (vm_paging_map((virtaddr_t)p, p) == EXIT_FAILURE)
	{
	  return EXIT_FAILURE;
	}
    }

  /* Load kernel page directory */
  x86_load_pd((physaddr_t)kern_pd);

  return EXIT_SUCCESS;
}




/**

   Function: u8_t vm_paging_map(virtaddr_t vaddr, physaddr_t paddr)
   ----------------------------------------------------------------


   Associate `vaddr` and `paddr` in kernel space.


**/


PUBLIC u8_t vm_paging_map(virtaddr_t vaddr, physaddr_t paddr)
{

  struct pte* table;
  u16_t pde,pte;

  /* Get page directory and page table entries from `vaddr` */
  pde = VM_PAGING_GET_PDE(vaddr);
  pte = VM_PAGING_GET_PTE(vaddr);

  /* Cannot map self mapping area */
  if ( pde == VM_PAGING_SELFMAP )
    {
      return EXIT_FAILURE;
    }

  /* Kernel PDE must be pre-allocated */
  if (!(kern_pd[pde].present))
    {
      return EXIT_FAILURE;
    }

  /* Retrieve page table */
  table = (struct pte*)(kern_pd[pde].baseaddr<<VM_PAGING_BASESHIFT);
  
  /* If page table entry exists, mapping already exists (not good) */
  if (table[pte].present)
    {
      return EXIT_FAILURE;
    }

  /* Point page table entry to the desired physical page */
  table[pte].present = 1;
  table[pte].rw = 1;
  table[pte].user = 0;
  table[pte].baseaddr = paddr >> VM_PAGING_BASESHIFT;

  return EXIT_SUCCESS;
}



/**

   Function: u8_t vm_paging_unmap(virtaddr_t vaddr)
   ------------------------------------------------


   Remove the association virtual/physical corresponding to the virtaul address `vaddr` in kernel page directory

**/


PUBLIC u8_t vm_paging_unmap(virtaddr_t vaddr)
{
  struct pte* table;
  u16_t pde,pte;


  /* Get page directory and page table entries from `vaddr` */ 
  pde = VM_PAGING_GET_PDE(vaddr);
  pte = VM_PAGING_GET_PTE(vaddr);
 

  /* Check page table entry existence as well as validity in regards to self mapping */
  if ( (pde == VM_PAGING_SELFMAP)||(!kern_pd[pde].present) )
    {
      return EXIT_FAILURE;
    }

  /* Get page table */
  table = (struct pte*)(kern_pd[pde].baseaddr<<VM_PAGING_BASESHIFT);

  /* No page table ? Error */
  if (!table[pte].present)
    {
      return EXIT_FAILURE;
    }

  /* Nullify the corresponding page table entry */
  table[pte].present=0;
  table[pte].rw=0;
  table[pte].user=0;
  table[pte].baseaddr=0;

  return EXIT_SUCCESS;
	   
}
  
