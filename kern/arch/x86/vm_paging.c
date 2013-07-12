/**

   paging.c
   ========

   x86 Paging management

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - paging.h   : self header
 
**/


#include <define.h>
#include <types.h>
#include "paging.h"



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

  /* Allocate kernel page directory */
  kern_pd = (struct pde*)base;

  /* Update `base` */
  base += VM_PAGING_ENTRIES*sizeof(struct pde);


  return EXIT_SUCCESS;
}
