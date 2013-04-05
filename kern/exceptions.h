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

   - types.h
   - const.h
   - thread.h      : struct thread needed
   
**/

#include <types.h>
#include "const.h"
#include "thread.h"


/**

   Prototypes
   ----------

   Declare the exceptions handler

**/

PUBLIC void excep_handle(u32_t num, struct thread* th);

#endif
