/**

   arch_vm.h
   =========


   Define "glue" functions for using architecture dependant virtual
   memory mechanism in kernel

**/


#ifndef ARCH_VM_H
#define ARCH_VM_H


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

   Typedef: addrspace_t
   --------------------

   address space

**/


typedef virtaddr_t addrspace_t;



/** 

    Function Pointers
    -----------------

    Glue for map, unmap and address space switch

**/


PRIVATE u8_t (*arch_vm_map)(virtaddr_t vaddr, physaddr_t paddr)__attribute__((unused)) = &vm_paging_map;
PRIVATE u8_t (*arch_vm_unmap)(virtaddr_t vaddr)__attribute__((unused)) = &vm_paging_unmap;
PRIVATE u8_t (*arch_vm_switch_to)(addrspace_t sp)__attribute__((unused)) = &vm_switch_to;


#endif
