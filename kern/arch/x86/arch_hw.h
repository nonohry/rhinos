/**

   arch_hw.h
   =========


   Define "glue" functions for using architecture dependant 
   harware features in kernel

**/


#ifndef ARCH_HW_H
#define ARCH_HW_H


/**

   Includes
   --------

   - define.h
   - types.h
   - pic.h     : x86 pit functions
   - x86_lib.h : sti
 
**/


#include <define.h>
#include <types.h>
#include "pic.h"
#include "x86_lib.h"


/** 

    Function Pointers
    -----------------

    Glue for pit, sti

**/


PRIVATE u8_t (*arch_enable_irq)(u8_t n)__attribute__((unused)) = &pic_enable_irq;
PRIVATE u8_t (*arch_disable_irq)(u8_t n)__attribute__((unused)) = &pic_disable_irq;
PRIVATE void (*arch_sti)(void)__attribute__((unused)) = &x86_sti;


#endif
