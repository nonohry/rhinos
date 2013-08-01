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
   - arch_ctx.h   : arch_ctx_t needed
   - context.h    : struct context
   - x86_lib.h    : x86_get_pf_addr
   - vm_paging.h  : x86 virtual memory functions
 
**/


#include <define.h>
#include <types.h>
#include "arch_ctx.h"
#include "context.h"
#include "x86_lib.h"
#include "vm_paging.h"



/**

   Typedef: addrspace_t
   --------------------

   address space

**/


typedef virtaddr_t addrspace_t;


/**

   Constants: page fault types
   ---------------------------

**/


#define ARCH_PF_UNRESOLVABLE     VM_PF_UNRESOLVABLE   
#define ARCH_PF_INTERNAL         VM_PF_INTERNAL      
#define ARCH_PF_EXTERNAL         VM_PF_EXTERNAL         



/** 

    Function Pointers
    -----------------

    Glue for map, unmap, address space switch and page fault handling

**/


#endif
