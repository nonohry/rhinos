/**

   vm_pool.h
   =========

   Virtual pages pool header

**/



#ifndef VM_POOL_H
#define VM_POOL_H


/**

   Includes
   --------

   - define.h
   - types.h
 
**/

#include <define.h>
#include <types.h>


/**

   Constant: VM_POOL_ERROR
   ------------------------

   Error value returned in case of allocation failure
   It is a non-aligned value

**/

#define VM_POOL_ERROR     1


/**

   Prototypes
   ----------

   Give access to pool setup

**/

PUBLIC u8_t vm_pool_setup(void);
PUBLIC virtaddr_t vm_pool_alloc(void);
PUBLIC u8_t vm_pool_free(u32_t addr);

#endif
