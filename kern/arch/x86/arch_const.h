/**

   arch_const.h
   ============

   Glue header for architecture dependant constantes

 **/

#ifndef ARCH_CONST_H
#define ARCH_CONST_H



/**

   Includes
   --------

   - x86_const.h

**/


#include "x86_const.h"


/**

   Constants: Kernel boundaries
   -----------------------------

**/


#define ARCH_CONST_KERN_START             X86_CONST_KERN_START
#define ARCH_CONST_KERN_END               X86_CONST_KERN_END


/**

   Constants: Paging relatives
   ----------------------------

**/


#define ARCH_CONST_PAGE_SIZE              X86_CONST_PAGE_SIZE
#define ARCH_CONST_PAGE_SHIFT             X86_CONST_PAGE_SHIFT







#endif
