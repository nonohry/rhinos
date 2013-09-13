/**

   arch_vm.h
   =========


   Define "glue" functions for using architecture dependant 
   virtual memory mechanisms in kernel

**/


#ifndef ARCH_VM_H
#define ARCH_VM_H


/**

   Includes
   --------

   - define.h
   - types.h
   - vm_paging.h   : x86 paging

**/


#include <define.h>
#include <types.h>
#include "vm_paging.h"


/**

   Typedef: arch_addrspace_t
   -------------------------

   Glue for address space type

**/

typedef virtaddr_t arch_addrspace_t;


/** 

    Function Pointers
    -----------------

    Glue for vm_sync

**/


PRIVATE u8_t (*arch_sync_addrspace)(virtaddr_t addrspace)__attribute__((unused)) = &vm_sync;

#endif
