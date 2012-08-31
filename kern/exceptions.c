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
#include "thread.h"
#include "exceptions.h"


/*========================================================================
 * Gestion des exceptions (generique)
 *========================================================================*/

PUBLIC void excep_handle(u32_t num, struct thread* th)
{
  klib_printf("Exception %d !\n",num);
  klib_printf(" gs: 0x%x \n fs: 0x%x \n es: 0x%x \n ds: 0x%x \n",th->cpu.gs,th->cpu.fs,th->cpu.es,th->cpu.ds);
  klib_printf(" edi: 0x%x \n esi: 0x%x \n ebp: 0x%x \n esp2: 0x%x \n",th->cpu.edi,th->cpu.esi,th->cpu.ebp,th->cpu.orig_esp);
  klib_printf(" ebx: 0x%x \n edx: 0x%x \n ecx: 0x%x \n eax: 0x%x \n",th->cpu.ebx,th->cpu.edx,th->cpu.ecx,th->cpu.eax);
  klib_printf(" ret_addr: 0x%x \n error: 0x%x \n eip: 0x%x \n cs: 0x%x \n",th->cpu.ret_addr,th->cpu.error_code,th->cpu.eip,th->cpu.cs);
  klib_printf(" eflags: 0x%x \n esp: 0x%x \n ss: 0x%x \n",th->cpu.eflags,th->cpu.esp,th->cpu.ss);
  while(1){}

  return;
}
