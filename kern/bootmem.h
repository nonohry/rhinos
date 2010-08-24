#ifndef BOOTMEM_H
#define BOOTMEM_H


/*************
 * Includes 
 *************/

#include <types.h>

/***************
 * Constantes
 ***************/

#define    BOOTMEM_START     0x0
#define    BOOTMEM_END       0xFFFFF

#define    PAGE_SIZE         4096
#define    BOOTMEM_PAGES     (BOOTMEM_END-BOOTMEM_START)/PAGE_SIZE

/*************
 * Prototypes
 *************/

PUBLIC void* bootmem_alloc(u32_t size);
PUBLIC void bootmem_free(void* addr, u32_t size);

#endif
