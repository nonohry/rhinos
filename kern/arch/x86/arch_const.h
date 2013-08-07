/**

   arch_const.h
   ============

   Glue header for architecture dependant constantes

 **/

#ifndef ARCH_CONST_H
#define ARCH_CONST_H



/**

   Includes
   --------

   - x86_const.h
   - vm_paging.h

**/


#include "x86_const.h"
#include "vm_paging.h"


/**

   Constants: Kernel boundaries
   -----------------------------

**/


#define ARCH_CONST_KERN_START             X86_CONST_KERN_START
#define ARCH_CONST_KERN_END               X86_CONST_KERN_END


/**

   Constants: Paging relatives
   ----------------------------

**/


#define ARCH_CONST_PAGE_SIZE              X86_CONST_PAGE_SIZE
#define ARCH_CONST_PAGE_SHIFT             X86_CONST_PAGE_SHIFT



/**

   Constants: Page Fault flags
   ---------------------------

**/


#define ARCH_PF_UNRESOLVABLE              VM_PF_UNRESOLVABLE
#define ARCH_PF_INTERNAL                  VM_PF_INTERNAL
#define ARCH_PF_EXTERNAL                  VM_PF_EXTERNAL
#define ARCH_PF_RW                        VM_PF_RW
#define ARCH_PF_SUPER                     VM_PF_SUPER
#define ARCH_PF_ELF                       VM_PF_ELF


/**

   Constant: ARCH_CONST_KERN_HIGHMEM
   ----------------------------------

   Last byte of kernel virtual memory

**/

#define ARCH_CONST_KERN_HIGHMEM            X86_CONST_KERN_HIGHMEM

#endif
