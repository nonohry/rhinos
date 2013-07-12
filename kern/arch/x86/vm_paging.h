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

   Prototypes
   ----------

   Give access to paging setup and un/mapping

**/

PUBLIC u8_t vm_paging_setup(physaddr_t base);
PUBLIC u8_t vm_paging_map(virtaddr_t vaddr, physaddr_t paddr);
PUBLIC u8_t vm_paging_unmap(virtaddr_t vaddr);

#endif
