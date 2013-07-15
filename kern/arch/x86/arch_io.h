/**

   arch_io.h
   =========


   Define "glue" functions for using architecture dependant 
   output functions in kernel

**/


#ifndef ARCH_IO_H
#define ARCH_IO_H


/**

   Includes
   --------

   - define.h
   - types.h
   - serial.h  : x86 output functions
 
**/


#include <define.h>
#include <types.h>
#include "serial.h"
#include "x86_lib.h"


/** 

    Function Pointers
    -----------------

    Glue for printf, memset and memcopy

**/


PRIVATE void (*arch_printf)(const char* str,...)__attribute__((unused)) = &serial_printf;
PRIVATE void (*arch_memset)(u32_t val, addr_t dest, u32_t len)__attribute__((unused)) = &x86_mem_set;
PRIVATE void (*arch_memcopy)(addr_t src, addr_t dest, u32_t len)__attribute__((unused)) = &x86_mem_copy;

#endif
