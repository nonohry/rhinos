#ifndef KLIB_H
#define KLIB_H

#include <types.h>

EXTERN void bochs_print(char*,...);
EXTERN void outb(u16_t,u8_t);
EXTERN void inb(u16_t,u8_t*);
EXTERN void load_CR3(physaddr_t);
EXTERN void set_pg_cr0(void);
EXTERN void flush_tlb(void);
EXTERN void mem_set(u32_t, physaddr_t, u32_t);
EXTERN void mem_copy(physaddr_t, physaddr_t, u32_t);
EXTERN u32_t random(void);
EXTERN void idle_task(void);

#endif
