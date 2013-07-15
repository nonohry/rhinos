/**

   mem.h
   =====

   Kernel memory manager header file

**/


#ifndef MEM_H
#define MEM_H


/**

   Includes
   --------

   - define.h
   - types.h
 
**/


#include <define.h>
#include <types.h>


/**

   Prototypes
   ==========

   Give access to memory manager initialization as well as allocation/release primitives

**/



PUBLIC u8_t mem_init(void);


#endif
