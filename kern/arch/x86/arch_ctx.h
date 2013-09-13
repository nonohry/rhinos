/**

   arch_ctx.h
   ==========


   Define "glue" functions for using architecture dependant 
   cpu context in kernel

**/


#ifndef ARCH_CTX_H
#define ARCH_CTX_H


/**

   Includes
   --------

   - define.h
   - types.h
   - context.h    : x86 cpu context
 
**/


#include <define.h>
#include <types.h>
#include "context.h"



/**

   Typedef: arch_ctx_t
   --------------------

   Glue for CPU context structure

**/

typedef struct x86_context arch_ctx_t;


/** 

    Function Pointers
    -----------------

    Glue for arch_ctx_setup

**/


PRIVATE u8_t (*arch_ctx_setup)(arch_ctx_t* ctx, virtaddr_t base, virtaddr_t stack_base, size_t stack_size)__attribute__((unused)) = &ctx_setup;

#endif
