#ifndef START_H
#define START_H


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "const.h"


/*========================================================================
 * Constantes
 *========================================================================*/


#define START_MEM_LIMIT         4294967295UL
#define START_MEM_SIZE_0        1048576

/* Type pour boot_mmap_e820 */

#define START_E820_AVAILABLE    0x1
#define START_E820_RESERVED     0x2
#define START_E820_ACPI         0x3
#define START_E820_ACPI_NVS     0x4



/*======================================================================== 
 * Structure boot_info
 *========================================================================*/

PUBLIC struct boot_info
{
  physaddr_t kern_start;     /* Debut du noyau */
  physaddr_t kern_end;       /* Fin du noyau */
  u8_t drv_number;
  u16_t drv_cylinders;
  u8_t drv_heads;
  u8_t drv_sectors;
  u32_t mem_map_addr;
  u16_t mem_map_count;
  u16_t mem_upper;
  u16_t mem_lower;
  u16_t mem_0x0;
  u16_t mem_0x100000;
  u32_t mem_total;
}__attribute__((packed));


/*========================================================================
 * Structure boot_memmap_entry
 *========================================================================*/

PUBLIC struct boot_mmap_e820
{
  u64_t addr;           /* Adresse de la zone */
  u64_t size;           /* Taille de la zone */
  u32_t type;           /* Type de memoire */
}__attribute__((packed));


/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC struct boot_info* bootinfo;

#endif
