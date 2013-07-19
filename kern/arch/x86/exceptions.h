/**

   exceptions.h
   ============

   Exception handling header file

**/


#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H


/**

   Includes
   --------

   - define.h
   - types.h
   - context.h      : CPU context needed
   
**/


#include <define.h>
#include <types.h>
#include "context.h"


/**

   Prototypes
   ----------

   Declare the exceptions handler

**/

PUBLIC void excep_handle(u32_t num, struct x86_context* ctx);

#endif
