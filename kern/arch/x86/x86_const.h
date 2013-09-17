/**

   x86_const.h
   =======

   x86 constantes

 **/

#ifndef X86_CONST_H
#define X86_CONST_H



/**

   Includes
   --------

   - define.h
   - types.h

**/


#include <define.h>
#include <types.h>
#include "context.h"


/**

   Constants: Kernel boundaries
   -----------------------------

   extern ones come from linker.

**/

extern u8_t __CONST_KERN_START[];
extern u8_t __CONST_KERN_END[];

#define X86_CONST_KERN_START             (u32_t)__CONST_KERN_START
#define X86_CONST_KERN_END               (u32_t)__CONST_KERN_END


/**

   Constants: Paging relatives
   ----------------------------

**/

#define X86_CONST_PAGE_SIZE              4096
#define X86_CONST_PAGE_SHIFT             12


/**

   Constants: Memory layout relatives
   -----------------------------------

**/


#define X86_CONST_ROM_AREA_START        0x9F000
#define X86_CONST_ROM_AREA_SIZE         0x60FFF
#define X86_CONST_ACPI_AREA_START       0xFEC00000
#define X86_CONST_ACPI_AREA_SIZE        0x13FFFFF
#define X86_CONST_KERN_HIGHMEM          (1<<28)

/**

   Constant: X86_CONST_BOOT_MODULES
   ----------------------------

   Number of modules loaded by bootloader

**/

#define X86_CONST_BOOT_MODULES           3


/**

   Constants: Segmentation selectors
   ----------------------------------

**/

#define X86_CONST_KERN_CS_SELECTOR       8    /*  CS = 0000000000001  0  00   =  8  */
#define	X86_CONST_KERN_DS_SELECTOR       16   /*  DS = 0000000000010  0  00   =  16 */
#define	X86_CONST_KERN_ES_SELECTOR	     16   /*  ES = 0000000000010  0  00   =  16 */
#define	X86_CONST_KERN_SS_SELECTOR	     16   /*  SS = 0000000000010  0  00   =  16 */

#define X86_CONST_USER_CS_SELECTOR       27   /*  CS = 0000000000011  0  11   =  8  */
#define	X86_CONST_USER_DS_SELECTOR       35   /*  DS = 0000000000100  0  11   =  16 */
#define	X86_CONST_USER_ES_SELECTOR	     35   /*  ES = 0000000000100  0  11   =  16 */
#define	X86_CONST_USER_SS_SELECTOR	     35   /*  SS = 0000000000100  0  11   =  16 */

#define X86_CONST_TSS_SELECTOR           40   /* TSS = 0000000000101  0  00   =  40 */


/**
   
   Constants: Processor rings
   ---------------------------

**/

#define X86_CONST_RING0                  0
#define X86_CONST_RING1                  1
#define X86_CONST_RING2                  2
#define X86_CONST_RING3                  3




/**

   Constants: Registers involved in message passing
   -----------------------------------------------

   ESI = Source (and message type)
   EDI = Destination
   EAX = Return code
   EBX = Message 1/3
   ECX = Message 2/3
   EDX = Message 3/3

**/


#define X86_CONST_SOURCE                  CTX_ESI
#define X86_CONST_DEST                    CTX_EDI
#define X86_CONST_RETURN                  CTX_EAX
#define X86_CONST_MSG1                    CTX_EBX
#define X86_CONST_MSG2                    CTX_ECX
#define X86_CONST_MSG3                    CTX_EDX


#endif
