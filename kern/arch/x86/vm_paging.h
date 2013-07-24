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

   Constants: Page fault error codes
   ---------------------------------

**/


#define VM_PF_SUPER_READ_NONPRESENT       0
#define VM_PF_SUPER_READ_PROTECTION       1
#define VM_PF_SUPER_WRITE_NONPRESENT      2
#define VM_PF_SUPER_WRITE_PROTECTION      3

#define VM_PF_USER_READ_NONPRESENT        4
#define VM_PF_USER_READ_PROTECTION        5
#define VM_PF_USER_WRITE_NONPRESENT       6
#define VM_PF_USER_WRITE_PROTECTION       7



/**

   Prototypes
   ----------

   Give access to paging setup and un/mapping

**/

PUBLIC u8_t vm_paging_setup(physaddr_t* base);
PUBLIC u8_t vm_paging_map(virtaddr_t vaddr, physaddr_t paddr);
PUBLIC u8_t vm_paging_unmap(virtaddr_t vaddr);
PUBLIC u8_t vm_switch_to(virtaddr_t pd_addr);
PUBLIC u8_t vm_pf_resolvable(struct x86_context* ctx);
PUBLIC u8_t vm_pf_fix(virtaddr_t vaddr, physaddr_t paddr, u8_t rw, u8_t super);

#endif
