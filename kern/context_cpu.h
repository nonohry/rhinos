/*
 * context_cpu.h
 * Header de context_cpu.c
 *
 */


#ifndef CONTEXT_CPU_H
#define CONTEXT_CPU_H


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "const.h"


/*========================================================================
 * Constantes
 *========================================================================*/

#define CTX_CPU_MIN_STACK         32
#define CTX_CPU_INTFLAG_SHIFT     9
#define CTX_CPU_FEC               0xFEC

/*========================================================================
 * Structures
 *========================================================================*/


PUBLIC struct context_cpu
{
  reg16_t gs;
  reg16_t fs;
  reg16_t es;
  reg16_t ds;
  reg32_t edi;
  reg32_t esi;
  reg32_t ebp;
  reg32_t orig_esp;
  reg32_t ebx;
  reg32_t edx;
  reg32_t ecx;
  reg32_t eax;
  reg32_t ret_addr;   /* Adresse de retour empilee par les appels de fonctions */
  reg32_t error_code;
  reg32_t eip;
  reg32_t cs;
  reg32_t eflags;
  reg32_t esp;
  reg32_t ss;
} __attribute__ ((packed));


/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC	struct context_cpu* kern_ctx;
PUBLIC	struct context_cpu* cur_ctx;

PUBLIC void context_cpu_init(void);
PUBLIC struct context_cpu* context_cpu_create(virtaddr_t entry, void* arg, virtaddr_t stack_base, u32_t stack_size);
PUBLIC u8_t context_cpu_destroy(struct context_cpu* ctx);
PUBLIC void context_cpu_postsave(reg32_t ss, reg32_t* esp);
PUBLIC void context_cpu_switch_to(struct context_cpu* ctx);
PUBLIC void context_cpu_handle_switch_to(struct context_cpu* ctx);
PUBLIC void context_cpu_exit_to(struct context_cpu* ctx);
PUBLIC void context_cpu_handle_exit_to(struct context_cpu* ctx);

#endif
