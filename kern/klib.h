#ifndef KLIB_H
#define KLIB_H

#include <types.h>

EXTERN void bochs_print(char*,...);
EXTERN void outb(u16_t,u8_t);
EXTERN void inb(u16_t,u8_t*);
EXTERN void phys_copy(u32_t, u32_t, u32_t);
EXTERN u32_t random(void);
EXTERN void idle_task(void);

#endif
