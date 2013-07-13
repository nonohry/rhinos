/**

   vm_arch.h
   =========


   Define "glue" functions for using architecture dependant virtual
   memory mechanism in kernel

**/


#ifndef VM_ARCH_H
#define VM_ARCH_H


/**

   Includes
   --------

   - define.h
   - types.h
   - vm_paging.h  : x86 virtual memory functions
 
**/


#include <define.h>
#include <types.h>
#include "vm_paging.h"



/** 

    Function Pointers
    -----------------

    Glue for map & unmap

**/


PUBLIC u8_t (*arch_vm_map)(virtaddr_t vaddr, physaddr_t paddr);
PUBLIC u8_t (*arch_vm_unmap)(virtaddr_t vaddr);



/**

   Function Pointers Assignement
   -----------------------------

   Define arch_map & arch_unmap

**/


arch_vm_map = &vm_paging_map;
arch_vm_unmap = &vm_paging_unmap;


#endif
