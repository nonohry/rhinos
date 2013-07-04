/**

   paging.h
   ========

   Pagination header. 
   Define useful constants and macros

**/

#ifndef PAGING_H
#define PAGING_H

/**

   Includes
   --------

   - types.h
   - const.h

**/

#include <define.h>
#include <types.h>
#include "const.h"


/**

   Constants: paging relatives
   ---------------------------

**/

#define PAGING_ENTRIES      1024
#define PAGING_DIRSHIFT     22
#define PAGING_TBLSHIFT     12
#define PAGING_TBLMASK      0x3FF
#define PAGING_OFFMASK      0xFFF
#define PAGING_BASESHIFT    12

#define PAGING_USER         0
#define PAGING_SUPER        1
#define PAGING_IDENTITY     2

#define PAGING_SELFMAP      0x3FF


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

   Macro: PAGING_GET_PDE(__addr)
   -----------------------------

   Extract the page directory entry number from a virtual address

**/

#define PAGING_GET_PDE(__addr)				\
  ( (__addr) >> PAGING_DIRSHIFT )



/**

   Macro: PAGING_GET_PTE(__addr)
   -----------------------------
   
   Extract the page table entry number from a virtual address

**/

#define PAGING_GET_PTE(__addr)				\
  ( ((__addr) >> PAGING_TBLSHIFT)&PAGING_TBLMASK )


/**

   Macro: PAGING_ALIGN_INF(__addr)
   -------------------------------

   Align `__addr` on  lower page size boundary

**/

#define PAGING_ALIGN_INF(__addr)			\
  ( ((__addr) >> CONST_PAGE_SHIFT) << CONST_PAGE_SHIFT )


/**

   Macro: PAGING_ALIGN_INF(__addr)
   -------------------------------

   Align `__addr` on  upper page size boundary

**/

#define PAGING_ALIGN_SUP(__addr)						\
  ( (((__addr)&0xFFFFF000) == (__addr))?((__addr) >> CONST_PAGE_SHIFT) << CONST_PAGE_SHIFT:(((__addr) >> CONST_PAGE_SHIFT)+1) << CONST_PAGE_SHIFT )


/**

   Macro: PAGING_GET_PD()
   ----------------------

   Return virtual address of current page directory thanks to self maping

**/
 
#define PAGING_GET_PD()							\
  ( (virtaddr_t)(PAGING_SELFMAP<<PAGING_DIRSHIFT) + (virtaddr_t)(PAGING_SELFMAP<<PAGING_TBLSHIFT) )


/**

   Macro: PAGING_GET_PT(__i)
   -------------------------

   Return virtual address of page table pointed by the entry `i` 
   in current page directory thanks to self maping

**/

#define PAGING_GET_PT(__i)						\
  ( (virtaddr_t)(PAGING_SELFMAP<<PAGING_DIRSHIFT) + (virtaddr_t)((__i)<<PAGING_TBLSHIFT) )



/**

   Global: struct pde* kern_PD
   ---------------------------

   Kernel Page Directory

**/

PUBLIC struct pde* kern_PD;


/**
   
   Prototypes
   ----------

   Give access to mapping and unmapping as well as initialization and conversion utility

**/

PUBLIC u8_t paging_init(void);
PUBLIC u8_t paging_map(virtaddr_t vaddr, physaddr_t paddr, u8_t flags);
PUBLIC u8_t paging_unmap(virtaddr_t vaddr);
PUBLIC physaddr_t paging_virt2phys(virtaddr_t vaddr);

#endif
