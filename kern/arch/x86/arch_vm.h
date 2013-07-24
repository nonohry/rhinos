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


PRIVATE u8_t (*arch_vm_map)(virtaddr_t vaddr, physaddr_t paddr)__attribute__((unused)) = &vm_paging_map;
PRIVATE u8_t (*arch_vm_unmap)(virtaddr_t vaddr)__attribute__((unused)) = &vm_paging_unmap;
PRIVATE u8_t (*arch_vm_switch_to)(addrspace_t sp)__attribute__((unused)) = &vm_switch_to;
PRIVATE virtaddr_t (*arch_pf_addr)(void)__attribute__((unused)) = &x86_get_pf_addr;
PRIVATE u8_t (*arch_pf_resolvable)(arch_ctx_t* ctx)__attribute__((unused)) = &vm_pf_resolvable;
PRIVATE u8_t (*arch_pf_fix)(virtaddr_t vaddr, physaddr_t paddr, u8_t flag, u8_t rw, u8_t super)__attribute__((unused)) = &vm_pf_fix;


#endif
