/**

   vm_segment.h
   =============

   Virtual Memory Segmentation Setup Header 

**/


#ifndef VM_SEGMENT_H
#define VM_SEGMENT_H


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

    Give access to segmentation setup

**/

PUBLIC u8_t vm_segment_setup(void);

#endif
