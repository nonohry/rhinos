/**

   main.c
   ======

    Kernel main function. All initializations occur here

**/



/**
   
   Includes
   --------
   
   - define.h
   - types.h
   - arch_io   : architecture dependent io library
   - boot.h    : structure boot_info

**/


#include <define.h>
#include <types.h>
#include <arch_io.h>
#include "mem.h"
#include "thread.h"
#include "boot.h"


/**

   Function: int main(void)
   ------------------------

   Kernel main function

   It is responsible for initializing kernel services and launching servers

**/


PUBLIC int main(void)
{

  arch_printf("Hello World (from main) !\n");
  if (mem_setup() != EXIT_SUCCESS)
    {
      arch_printf("Unable to intialize kernel memory manager\n");
      goto err;
    }

  
  virtaddr_t s = (virtaddr_t)mem_alloc(64);
  struct thread* th = thread_create("toto",
				    0xBADBEEF,
				    s,
				    64);

  if (th != NULL)
    {
      arch_printf("Hello my name is %s\n",th->name);
      arch_printf("My entry point is at 0x%x\n",th->ctx.eip);
      arch_printf("My stack is at 0x%x and is 0x%x bytes long\n",
		  th->stack_base,
		  th->stack_size);
      arch_printf("My context is:\n");
      arch_printf(" gs: 0x%x \n fs: 0x%x \n es: 0x%x \n ds: 0x%x \n",th->ctx.gs,th->ctx.fs,th->ctx.es,th->ctx.ds);
      arch_printf(" edi: 0x%x \n esi: 0x%x \n ebp: 0x%x \n esp2: 0x%x \n",th->ctx.edi,th->ctx.esi,th->ctx.ebp,th->ctx.orig_esp);
      arch_printf(" ebx: 0x%x \n edx: 0x%x \n ecx: 0x%x \n eax: 0x%x \n",th->ctx.ebx,th->ctx.edx,th->ctx.ecx,th->ctx.eax);
      arch_printf(" ret_addr: 0x%x \n error: 0x%x \n eip: 0x%x \n cs: 0x%x \n",th->ctx.ret_addr,th->ctx.error_code,th->ctx.eip,th->ctx.cs);
      arch_printf(" eflags: 0x%x \n esp: 0x%x \n ss: 0x%x \n",th->ctx.eflags,th->ctx.esp,th->ctx.ss);
    }

  err:
 
  while(1)
    {
    }

  return EXIT_SUCCESS;
}

