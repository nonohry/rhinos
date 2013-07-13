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

    Function Pointers
    -----------------

    Glue for map & unmap

**/


PUBLIC void (*arch_printf)(const char* str,...);



/**

   Function Pointers Assignement
   -----------------------------

   Define arch_map & arch_unmap

**/


arch_printf = &serial_printf;


#endif
