/**

   context.h
   =========

   thread context definition

**/


#ifndef CONTEXT_H
#define CONTEXT_H


/**

   includes
   --------

   - types.h

**/


#include <types.h>


/**

   Structure: struct context
   -------------------------

   CPU Context.
   Registers order match push order during assembly context save

**/

PUBLIC struct context
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
  reg32_t ret_addr;
  reg32_t error_code;
  reg32_t eip;
  reg32_t cs;
  reg32_t eflags;
  reg32_t esp;
  reg32_t ss;
} __attribute__ ((packed));


#endif
