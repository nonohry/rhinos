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



/** 

    Function Pointer
    ----------------

    Glue for printf

**/


PRIVATE void (*arch_printf)(const char* str,...) = &serial_printf;


#endif
