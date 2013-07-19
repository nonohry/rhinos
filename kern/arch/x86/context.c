/**

   context.c
   =========

   CPU context manipulation

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - context.h
   - x86_lib.h

**/


#include <define.h>
#include <types.h>
#include "context.h"
#include "x86_lib.h"


/** 
    Constants: INTFLAG_SHIFT 
    ------------------------

    Shifts to reach Interrupt flag in EFLAGS

**/


#define INTFLAG_SHIFT     9


/**

   Function: u8_t ctx_setup(struct x86_context* ctx, virtaddr_t base, virtaddr_t stack_base, size_t stack_size)
   ------------------------------------------------------------------------------------------------------------

   Set up a cpu context.

   Simply provide segment registers, stack registers , EFLAGS and EIP.

**/   



PUBLIC u8_t ctx_setup(struct x86_context* ctx, virtaddr_t base, virtaddr_t stack_base, size_t stack_size)
{
  /* Sanity checks */
  if (!((base)&&(stack_base)&&(stack_size)))
    {
      return EXIT_FAILURE;
    }
  
  /* Nullify stack */
  x86_mem_set(0,(addr_t)stack_base,stack_size);
  
  /* Set up segment registers */
  ctx->cs = X86_CONST_USER_CS_SELECTOR;
  ctx->ds = X86_CONST_USER_DS_SELECTOR;
  ctx->es = X86_CONST_USER_ES_SELECTOR;
  ctx->ss = X86_CONST_USER_SS_SELECTOR;

  /* Fake error code */
  ctx->error_code = 0xFEC;

  /* Enable interrupts in EFLAGS */
  ctx->eflags = (1<<INTFLAG_SHIFT);

  /* EIP points to entry point */
  ctx->eip = (reg32_t)base;

  /* Set up ESP */
  ctx->esp = (reg32_t)(stack_base+stack_size);

  return EXIT_SUCCESS;
}
