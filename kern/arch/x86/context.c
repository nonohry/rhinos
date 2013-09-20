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
   - x86_const.h
   - x86_lib.h
   - vm_segment.h   : TSS needed
   - context.h      : self header

**/


#include <define.h>
#include <types.h>
#include "x86_const.h"
#include "x86_lib.h"
#include "vm_segment.h"
#include "context.h"


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
   These registers will be popped from stack when the context switch will occur,
   just like a normal return from interrupt.

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
  ctx->esp = (reg32_t)(stack_base+stack_size-2);

  return EXIT_SUCCESS;
}


/**

   Function: void ctx_postsave(struct x86_context* ctx, reg32_t* esp)
   ------------------------------------------------------------------

   Finalize cpu context save in case of a in-kernel-space switch.
   
   Simply retrieve registers saved on stack by processor 
   and store them in the context structure.

**/


PUBLIC void ctx_postsave(struct x86_context* ctx, reg32_t* esp)
{


  /* Check 16 bits SS to determine ring */ 
  if ((ctx->ss & 0xFF) == X86_CONST_KERN_SS_SELECTOR)
    {
      /* Retrieve remaining registers */
      ctx->ret_addr = *(esp);
      ctx->error_code = *(esp+1);
      ctx->eip = *(esp+2);
      ctx->cs = *(esp+3);
      ctx->eflags = *(esp+4);
      ctx->esp = (reg32_t)(esp);
      ctx->ss = X86_CONST_KERN_SS_SELECTOR;
    }
   
  return;
}



/**

   Function: void ctx_prepare_switch(struct x86_context* ctx)
   ----------------------------------------------------------

   Prepare future switch for thread with context `ctx` by setting 
   `tss.esp0` to end of `ctx`.

**/


PUBLIC void ctx_prepare_switch(struct x86_context* ctx)
{

  /* Set TSS */
  tss.esp0 = (virtaddr_t)ctx+sizeof(struct x86_context);
  
  return;
}



/**

   Function: void ctx_set_register(struct x86_context* ctx, u8_t r, u32_t value)
   -----------------------------------------------------------------------------

   Set register identified by `r` value to `value` into contxt `ctx`.
   Register can only be a register involved in message passing.

**/


PUBLIC void ctx_set_register(struct x86_context* ctx, u8_t r, u32_t value)
{
  switch(r)
    {
    case CTX_ESI:
      {
	ctx->esi=value;
	break;
      }
    case CTX_EDI:
      {
	ctx->edi=value;
	break;
      }
    case CTX_EAX:
      {
	ctx->eax=value;
	break;
      }
    case CTX_EBX:
      {
	ctx->ebx=value;
	break;
      }
    case CTX_ECX:
      {
	ctx->ecx=value;
	break;
      }
    case CTX_EDX:
      {
	ctx->edx=value;
	break;
      }
    }

  return;
}


/**

   Function: u32_t ctx_get_register(struct x86_context* ctx, u8_t r)
   -----------------------------------------------------------------

   Get register, identified by `r`, value from context `ctx`.
   Register can only be a register involved in message passing.

**/


PUBLIC u32_t ctx_get_register(struct x86_context* ctx, u8_t r)
{
  switch(r)
    {
    case CTX_ESI:
      {
	return ctx->esi;
	break;
      }
    case CTX_EDI:
      {
	return ctx->edi;
	break;
      }
    case CTX_EAX:
      {
	return ctx->eax;
	break;
      }
    case CTX_EBX:
      {
	return ctx->ebx;
	break;
      }
    case CTX_ECX:
      {
	return ctx->ecx;
	break;
      }
    case CTX_EDX:
      {
	return ctx->edx;
	break;
      }
    }

  return 0;
  
}
