/**

   pager0.h
   ========

   Kernel page fault handler header

**/


#ifndef PAGER0_H
#define PAGER0_H


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
   ----------

   Give access to setup

**/

u8_t pager0_setup(void);



#endif
