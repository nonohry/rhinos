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
   
   Prototype
   ---------

   Give access to memory map creation

**/


PUBLIC u8_t e820_setup(struct multiboot_info* bootinfo);

#endif
