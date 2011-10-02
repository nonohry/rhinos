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
 * Structures
 *========================================================================*/

PUBLIC	struct context_cpu* kern_ctx;
PUBLIC	struct context_cpu* cur_ctx;
PUBLIC  struct context_cpu* next_ctx;

PUBLIC void context_cpu_init(void);
PUBLIC void context_cpu_postsave(reg32_t ss, reg32_t* esp);

#endif
