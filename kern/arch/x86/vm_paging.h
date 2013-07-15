/**

   vm_paging.h
   ===========

   Virtual Memory Paging header. 
 
**/

#ifndef VM_PAGING_H
#define VM_PAGING_H


/**

   Includes
   --------

   - define.h
   - types.h
 
**/

#include <define.h>
#include <types.h>




/**

   Prototypes
   ----------

   Give access to paging setup and un/mapping

**/

PUBLIC u8_t vm_paging_setup(physaddr_t* base);
PUBLIC u8_t vm_paging_map(virtaddr_t vaddr, physaddr_t paddr);
PUBLIC u8_t vm_paging_unmap(virtaddr_t vaddr);

#endif
