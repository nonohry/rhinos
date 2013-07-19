/**

   exceptions.c
   ===========

   Exceptions Handling

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - serial.h      : output
   - context.h     : CPU context
   - exceptions.h  : self header

**/
   

#include <define.h>
#include <types.h>
#include "serial.h"
#include "context.h"
#include "exceptions.h"


/**
   
   Function: void excep_handle(u32_t num, struct x86_context* ctx)
   ---------------------------------------------------------------

   Handle or dispatch processor exceptions.
   For the moment, it only prints the thread cpu context

**/

PUBLIC void excep_handle(u32_t num, struct x86_context* ctx)
{
  serial_printf("Exception %d !\n",num);
  serial_printf(" gs: 0x%x \n fs: 0x%x \n es: 0x%x \n ds: 0x%x \n",ctx->gs,ctx->fs,ctx->es,ctx->ds);
  serial_printf(" edi: 0x%x \n esi: 0x%x \n ebp: 0x%x \n esp2: 0x%x \n",ctx->edi,ctx->esi,ctx->ebp,ctx->orig_esp);
  serial_printf(" ebx: 0x%x \n edx: 0x%x \n ecx: 0x%x \n eax: 0x%x \n",ctx->ebx,ctx->edx,ctx->ecx,ctx->eax);
  serial_printf(" ret_addr: 0x%x \n error: 0x%x \n eip: 0x%x \n cs: 0x%x \n",ctx->ret_addr,ctx->error_code,ctx->eip,ctx->cs);
  serial_printf(" eflags: 0x%x \n esp: 0x%x \n ss: 0x%x \n",ctx->eflags,ctx->esp,ctx->ss);
  while(1){}

  return;
}