/**

   e820.h
   ======

   E820 Memory Map header

**/



#ifndef E820_H
#define E820_H


/**

   Includes
   --------

   - define.h
   - types.h
   - setup.h    : Multiboot information structure

**/


#include <define.h>
#include <types.h>
#include "setup.h"



/**
   
   Constants: types of memory in a E820 memory map
   -----------------------------------------------

**/

#define E820_AVAILABLE    0x1
#define E820_RESERVED     0x2
#define E820_ACPI         0x3
#define E820_ACPI_NVS     0x4



/**
   
   Prototype
   ---------

   Give access to memory map creation

**/


PUBLIC u8_t e820_setup(struct multiboot_info* bootinfo);

#endif
