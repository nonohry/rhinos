/**

   vm_pool.c
   =========

   Virtual pages pool, implemented as a stack
   

**/



/**

   Includes
   --------

   - define.h
   - types.h
   - arch_const.h  : Kernel boudaries needed
   - boot.h        : boot info
   - vm_pool.h     : self header

**/


#include <define.h>
#include <types.h>
#include <arch_const.h>
#include "boot.h"
#include "vm_pool.h"
