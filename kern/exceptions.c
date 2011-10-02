/*
 * Gestion des exceptions
 *
 */


/*========================================================================
 * Interrupt
 *========================================================================*/

#include <types.h>
#include "const.h"
#include "klib.h"
#include "exceptions.h"


/*========================================================================
 * Gestion des exceptions (generique)
 *========================================================================*/

PUBLIC void excep_handle(u32_t num, struct context_cpu* ctx)
{
  bochs_print("Exception %d !\n",num);
  bochs_print(" gs: 0x%x \n fs: 0x%x \n es: 0x%x \n ds: 0x%x \n",ctx->gs,ctx->fs,ctx->es,ctx->ds);
  bochs_print(" edi: 0x%x \n esi: 0x%x \n ebp: 0x%x \n esp2: 0x%x \n",ctx->edi,ctx->esi,ctx->ebp,ctx->orig_esp);
  bochs_print(" ebx: 0x%x \n edx: 0x%x \n ecx: 0x%x \n eax: 0x%x \n",ctx->ebx,ctx->edx,ctx->ecx,ctx->eax);
  bochs_print(" ret_addr: 0x%x \n error: 0x%x \n eip: 0x%x \n cs: 0x%x \n",ctx->ret_addr,ctx->error_code,ctx->eip,ctx->cs);
  bochs_print(" eflags: 0x%x \n esp: 0x%x \n ss: 0x%x \n",ctx->eflags,ctx->esp,ctx->ss);
  while(1){}

  return;
}
