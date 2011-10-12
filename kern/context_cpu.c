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
#include "virtmem_slab.h"
#include "context_cpu.h"


/*========================================================================
 * Declaration Private
 *========================================================================*/

PRIVATE void context_cpu_trampoline(cpu_ctx_func_t start_func, void* start_arg, cpu_ctx_func_t exit_func, void* exit_arg);
PRIVATE void print_context(struct context_cpu* ctx);

PRIVATE struct vmem_cache* ctx_cache;
PRIVATE struct context_cpu* next_ctx;


/*========================================================================
 * Initialisation
 *========================================================================*/


PUBLIC void context_cpu_init(void)
{

  /* Initialise un cache */
  ctx_cache = virtmem_cache_create("ctx_cpu_cache",sizeof(struct context_cpu),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  ASSERT_FATAL( ctx_cache!=NULL );

  /* Alloue le contexte noyau */
  kern_ctx = (struct context_cpu*)virtmem_cache_alloc(ctx_cache,VIRT_CACHE_DEFAULT);
  ASSERT_FATAL( kern_ctx != NULL );

  /* Le noyau devient le contexte courant */
  cur_ctx = kern_ctx;

  return;
}


/*========================================================================
 * Creation d un contexte
 *========================================================================*/


PUBLIC struct context_cpu* context_cpu_create(virtaddr_t start_entry, void* start_arg, virtaddr_t exit_entry, void* exit_arg, virtaddr_t stack_base, u32_t stack_size)
{
  struct context_cpu* ctx;
  virtaddr_t* esp;
  
  /* Petite verification */
  ASSERT_RETURN( (start_entry!=0)&&(exit_entry!=0)&&(stack_base!=0)&&(stack_size>CTX_CPU_MIN_STACK) , NULL);

  /* Alloue le contexte */
  ctx = (struct context_cpu*)virtmem_cache_alloc(ctx_cache,VIRT_CACHE_DEFAULT);
  ASSERT_RETURN( ctx != NULL , NULL );

  /* Nettoie la pile */
  klib_mem_set(0,stack_base,stack_size);
  /* Recupere l'adresse de pile pour empiler les arguments */
  esp = (virtaddr_t*)(stack_base+stack_size);
 
  /* Installe les registres de segments */
  ctx->cs = CONST_CS_SELECTOR;
  ctx->ds = CONST_DS_SELECTOR;
  ctx->es = CONST_ES_SELECTOR;
  ctx->ss = CONST_SS_SELECTOR;

  /* Positionne un faux code d erreur */
  ctx->error_code = CTX_CPU_FEC;

  /* Active les interruptions */
  ctx->eflags = (1<<CTX_CPU_INTFLAG_SHIFT);


  /* Pointe EIP sur la fonction trampoline */
  ctx->eip = (reg32_t)context_cpu_trampoline;

  /* Empile les arguments */
  *(--esp) = (virtaddr_t)exit_arg;
  *(--esp) = exit_entry;
  *(--esp) = (virtaddr_t)start_arg;
  *(--esp) = start_entry;
  /* Fausse adresse de retour pour la fonction de trampoline */
  *(--esp) = 0;

  /* Simule une pile interrompue (le switch passe par une interruption logicielle) */
  *(--esp) = ctx->eflags;
  *(--esp) = CONST_CS_SELECTOR;
  *(--esp) = ctx->eip;
  *(--esp) = ctx->error_code;
  *(--esp) = 0;

  /* Installe la pile */
  ctx->esp = (reg32_t)esp;


  return ctx;
}


/*========================================================================
 * Suppression d un contexte
 *========================================================================*/


PUBLIC u8_t context_cpu_destroy(struct context_cpu* ctx)
{
  /* Libere la structure */
  return virtmem_cache_free(ctx_cache,ctx);
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
      cur_ctx->esp = (reg32_t)(esp);
      cur_ctx->ss = CONST_SS_SELECTOR;
    }
  
  return;
}


/*========================================================================
 * Changement de contexte
 *========================================================================*/


PUBLIC void context_cpu_switch_to(struct context_cpu* ctx)
{
  /* Contexte surlequel switcher */
  next_ctx = ctx;
  /* Interruption pour changer le contexte */
  klib_int50();

  return;
}


/*========================================================================
 * Traite un changement de contexte
 *========================================================================*/


PUBLIC void context_cpu_handle_switch_to(struct context_cpu* ctx)
{
  /* Definit le nouveau contexte */
  cur_ctx = next_ctx;

  return;
}


/*========================================================================
 * Sortie d un contexte
 *========================================================================*/


PUBLIC void context_cpu_exit_to(struct context_cpu* ctx)
{
  /* Contexte vers lequel sortir */
  next_ctx = ctx;
  /* Interruption pour forcer le changement de contexte */
  klib_int51();
  return;
}


/*========================================================================
 * Trampoline
 *========================================================================*/


PRIVATE void context_cpu_trampoline(cpu_ctx_func_t start_func, void* start_arg, cpu_ctx_func_t exit_func, void* exit_arg)
{
  /* Trampline ! */
  start_func(start_arg);
  exit_func(exit_arg);

  /* Pas de retour */
  ASSERT_FATAL( 0 );

  return;
}



/*========================================================================
 * Affichage d un contexte
 *========================================================================*/

PRIVATE void print_context(struct context_cpu* ctx)
{
  klib_bochs_print(" gs: 0x%x \n fs: 0x%x \n es: 0x%x \n ds: 0x%x \n",ctx->gs,ctx->fs,ctx->es,ctx->ds);
  klib_bochs_print(" edi: 0x%x \n esi: 0x%x \n ebp: 0x%x \n esp2: 0x%x \n",ctx->edi,ctx->esi,ctx->ebp,ctx->orig_esp);
  klib_bochs_print(" ebx: 0x%x \n edx: 0x%x \n ecx: 0x%x \n eax: 0x%x \n",ctx->ebx,ctx->edx,ctx->ecx,ctx->eax);
  klib_bochs_print(" ret_addr: 0x%x \n error: 0x%x \n eip: 0x%x \n cs: 0x%x \n",ctx->ret_addr,ctx->error_code,ctx->eip,ctx->cs);
  klib_bochs_print(" eflags: 0x%x \n esp: 0x%x \n ss: 0x%x \n",ctx->eflags,ctx->esp,ctx->ss);
 
  return;
}
