/*
 * Gestion du contexte cpu
 *
 */


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "const.h"
#include "klib.h"
#include "assert.h"
#include "virtmem.h"
#include "context_cpu.h"


/*========================================================================
 * Declaration Private
 *========================================================================*/

PRIVATE void print_context(struct context_cpu* ctx);


/*========================================================================
 * Initialisation
 *========================================================================*/


PUBLIC void context_cpu_init(void)
{
  /* Alloue le contexte noyau */
  kern_ctx = (struct context_cpu*)virt_alloc(sizeof(struct context_cpu));
  ASSERT_FATAL( kern_ctx != NULL );

  /* Le noyau devient le contexte courant */
  cur_ctx = kern_ctx;

  return;
}


/*========================================================================
 * Post traitement de la sauvegarde du contexte
 *========================================================================*/


PUBLIC void context_cpu_postsave(reg32_t ss, reg32_t* esp)
{
  /* Traitement si pas changement de privileges */
  if (ss == CONST_SS_SELECTOR)
    {
      /* Recupere les registres oublies */
      cur_ctx->ret_addr = *(esp);
      cur_ctx->error_code = *(esp+1);
      cur_ctx->eip = *(esp+2);
      cur_ctx->cs = *(esp+3);
      cur_ctx->eflags = *(esp+4);
      cur_ctx->esp = (reg32_t)(esp+5);
      cur_ctx->ss = ss;
    }

  return;
}


/*========================================================================
 * Affichage d un context
 *========================================================================*/

PRIVATE void print_context(struct context_cpu* ctx)
{
  bochs_print(" gs: 0x%x \n fs: 0x%x \n es: 0x%x \n ds: 0x%x \n",ctx->gs,ctx->fs,ctx->es,ctx->ds);
  bochs_print(" edi: 0x%x \n esi: 0x%x \n ebp: 0x%x \n esp2: 0x%x \n",ctx->edi,ctx->esi,ctx->ebp,ctx->orig_esp);
  bochs_print(" ebx: 0x%x \n edx: 0x%x \n ecx: 0x%x \n eax: 0x%x \n",ctx->ebx,ctx->edx,ctx->ecx,ctx->eax);
  bochs_print(" ret_addr: 0x%x \n error: 0x%x \n eip: 0x%x \n cs: 0x%x \n",ctx->ret_addr,ctx->error_code,ctx->eip,ctx->cs);
  bochs_print(" eflags: 0x%x \n esp: 0x%x \n ss: 0x%x \n",ctx->eflags,ctx->esp,ctx->ss);
 
  return;
}
