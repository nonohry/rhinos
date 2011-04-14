#ifndef START_H
#define START_H


/**************
 * Includes
 **************/

#include <types.h>


/**************
 * Constantes
 **************/

#define BOOTINFO_ADDR     0x803

/* Type pour boot_mmap_e820 */

#define E820_AVAILABLE    0x1
#define E820_RESERVED     0x2
#define E820_ACPI         0x3
#define E820_ACPI_NVS     0x4


/********************** 
 * Structure boot_info
 **********************/

PUBLIC struct boot_info
{
  u32_t kern_start;     /* Debut du noyau */
  u32_t kern_end;       /* Fin du noyau */
  u32_t mem_addr;       /* Adresse du memory map */
  u16_t mem_entry;      /* Nombre d entree dans le memory map */
  u16_t mem_lower;      /* Memoire basse en Ko */
  u16_t mem_upper;      /* Memoire haute en 64Ko */
}__attribute__((packed));

/******************************
 * Structure boot_memmap_entry
 ******************************/

PUBLIC struct boot_mmap_e820
{
  u64_t addr;           /* Adresse de la zone */
  u64_t size;           /* Taille de la zone */
  u32_t type;           /* Type de memoire */
}__attribute__((packed));


/*************
 * Prototypes
 *************/

PUBLIC struct boot_info* bootinfo;

#endif
