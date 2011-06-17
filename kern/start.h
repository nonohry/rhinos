#ifndef START_H
#define START_H


/**************
 * Includes
 **************/

#include <types.h>


/**************
 * Constantes
 **************/

#define START_BOOTINFO_ADDR     0x803

/* Type pour boot_mmap_e820 */

#define START_E820_AVAILABLE    0x1
#define START_E820_RESERVED     0x2
#define START_E820_ACPI         0x3
#define START_E820_ACPI_NVS     0x4


/********************** 
 * Structure boot_info
 **********************/

PUBLIC struct boot_info
{
  physaddr_t kern_start;     /* Debut du noyau */
  physaddr_t kern_end;       /* Fin du noyau */
  physaddr_t mem_addr;       /* Adresse du memory map */
  u16_t mem_entry;      /* Nombre d entree dans le memory map */
  u16_t mem_lower;      /* Memoire basse en Ko */
  u16_t mem_upper;      /* Memoire haute en 64Ko */
  u32_t mem_ram_pages;  /* Nombre de pages memoire max (available01)*/
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
