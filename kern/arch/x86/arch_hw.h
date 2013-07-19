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
   - pic.h    : x86 pit functions
 
**/


#include <define.h>
#include <types.h>
#include "pic.h"


/** 

    Function Pointers
    -----------------

    Glue for pit

**/


PRIVATE u8_t (*arch_enable_irq)(u8_t n)__attribute__((unused)) = &pic_enable_irq;
PRIVATE u8_t (*arch_disable_irq)(u8_t n)__attribute__((unused)) = &pic_disable_irq;


#endif
