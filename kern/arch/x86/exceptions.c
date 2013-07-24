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
   - vm_paging.h   : page fault error codes
   - x86_lib.h
   - exceptions.h  : self header

**/
   

#include <define.h>
#include <types.h>
#include "serial.h"
#include "context.h"
#include "vm_paging.h"
#include "x86_lib.h"
#include "exceptions.h"


/**
   
   Function: void excep_handle(u32_t num, struct x86_context* ctx)
   ---------------------------------------------------------------

   Handle or dispatch processor exceptions.
   For the moment, it only prints the thread cpu context

**/

PUBLIC void excep_handle(u32_t num, struct x86_context* ctx)
{
  if (num == 14)
    {
      serial_printf("Page fault is %s resolvable:\n",vm_pf_resolvable(ctx)==TRUE?"":"not");

      switch(ctx->error_code)
	{
	case VM_PF_SUPER_READ_NONPRESENT:
	  serial_printf("Super tried to read non present page\n");
	  break;
	case VM_PF_SUPER_READ_PROTECTION:
	  serial_printf("Super tried to read and caused protection fault\n");
	  break;
	case VM_PF_SUPER_WRITE_NONPRESENT:
	  serial_printf("Super tried to write to non present page\n");
	  break;
	case VM_PF_SUPER_WRITE_PROTECTION:
	  serial_printf("Super tried to write and caused protection fault\n");
	  break;
	case VM_PF_USER_READ_NONPRESENT:
	  serial_printf("User tried to read non present page\n");
	  break;
	case VM_PF_USER_READ_PROTECTION:
	  serial_printf("User tried to read and caused protection fault\n");
	  break;
	case VM_PF_USER_WRITE_NONPRESENT:
	  serial_printf("User tried to write to non present page\n");
	  break;
	case VM_PF_USER_WRITE_PROTECTION:
	  serial_printf("User tried to write and caused protection fault\n");
	  break;
	default:
	  serial_printf("Page fault with strange error code 0x%x !\n", ctx->error_code);
	  break;
	}
      
      vm_pf_fix(x86_get_pf_addr(), 0x1000, 1, 1);

    }
  else
    {
      serial_printf("Exception %d with error code 0x%x !\n",num, ctx->error_code);
      serial_printf(" gs: 0x%x \n fs: 0x%x \n es: 0x%x \n ds: 0x%x \n",ctx->gs,ctx->fs,ctx->es,ctx->ds);
      serial_printf(" edi: 0x%x \n esi: 0x%x \n ebp: 0x%x \n esp2: 0x%x \n",ctx->edi,ctx->esi,ctx->ebp,ctx->orig_esp);
      serial_printf(" ebx: 0x%x \n edx: 0x%x \n ecx: 0x%x \n eax: 0x%x \n",ctx->ebx,ctx->edx,ctx->ecx,ctx->eax);
      serial_printf(" ret_addr: 0x%x \n error: 0x%x \n eip: 0x%x \n cs: 0x%x \n",ctx->ret_addr,ctx->error_code,ctx->eip,ctx->cs);
      serial_printf(" eflags: 0x%x \n esp: 0x%x \n ss: 0x%x \n",ctx->eflags,ctx->esp,ctx->ss);
      
      while(1){}
    }

  return;
}
