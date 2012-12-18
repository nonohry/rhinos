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


/* Multiboot */

#define START_MULTIBOOT_MAGIC   0x2BADB002

#define START_MULTIBOOT_FLAG_MEMORY    0x1
#define START_MULTIBOOT_FLAG_DEVICE    0x2
#define START_MULTIBOOT_FLAG_CMDLINE   0x4
#define START_MULTIBOOT_FLAG_MODS      0x8
#define START_MULTIBOOT_FLAG_AOUT      0x10
#define START_MULTIBOOT_FLAG_ELF       0x20
#define START_MULTIBOOT_FLAG_MMAP      0x40
#define START_MULTIBOOT_FLAG_DRIVE     0x80
#define START_MULTIBOOT_FLAG_CONFIG    0x100
#define START_MULTIBOOT_FLAG_BLNAME    0x200
#define START_MULTIBOOT_FLAG_APM       0x400
#define START_MULTIBOOT_FLAG_VIDEO     0x800


#define START_MEM_SIZE_0        1048576

/* Type pour boot_mmap_e820 */

#define START_E820_AVAILABLE    0x1
#define START_E820_RESERVED     0x2
#define START_E820_ACPI         0x3
#define START_E820_ACPI_NVS     0x4


/* Constantes sanitize */

#define START_E820_MAX              50
#define START_FRAME_MAX             4096
#define START_SHIFT                 12


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


PUBLIC struct multiboot_info
{
  u32_t flags;
  u32_t mem_lower;
  u32_t mem_upper;
  u32_t boot_device;
  u32_t cmdline;
  u32_t mods_count;
  u32_t mods_addr;
  u32_t elf_num;
  u32_t elf_size;
  u32_t elf_addr;
  u32_t elf_shndx;
  u32_t mmap_length;
  u32_t mmap_addr;
  u32_t drives_length;
  u32_t drives_addr;
  u32_t config_table;
  u32_t boot_loader_name;
  u32_t apm_table;
  u32_t vbe_control_info;
  u32_t vbe_mode_info;
  u16_t vbe_mode;
  u16_t vbe_interface_seg;
  u16_t vbe_interface_off;
  u16_t vbe_interface_len;
}__attribute__((packed));


PUBLIC struct multiboot_mmap_entry
{
  u32_t size;
  u64_t addr;
  u64_t len;
  u32_t type;
}__attribute__((packed));


PUBLIC struct multiboot_mod_entry
{
  u32_t start;
  u32_t end;
  u32_t cmdline;
  u32_t pad;
}__attribute__((packed));



/*========================================================================
 * Structure boot_mmap_entry
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
