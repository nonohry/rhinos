/**

   setup.h
   =======

   Environnement setup header

**/



#ifndef SETUP_H
#define SETUP_H


/**

   Includes
   --------

   - define.h
   - types.h
 
**/

#include <define.h>
#include <types.h>



/**
   
   Constants: Multiboot header relatives
   -------------------------------------

**/


#define MULTIBOOT_MAGIC   0x2BADB002

#define MULTIBOOT_FLAG_MEMORY    0x1
#define MULTIBOOT_FLAG_DEVICE    0x2
#define MULTIBOOT_FLAG_CMDLINE   0x4
#define MULTIBOOT_FLAG_MODS      0x8
#define MULTIBOOT_FLAG_AOUT      0x10
#define MULTIBOOT_FLAG_ELF       0x20
#define MULTIBOOT_FLAG_MMAP      0x40
#define MULTIBOOT_FLAG_DRIVE     0x80
#define MULTIBOOT_FLAG_CONFIG    0x100
#define MULTIBOOT_FLAG_BLNAME    0x200
#define MULTIBOOT_FLAG_APM       0x400
#define MULTIBOOT_FLAG_VIDEO     0x800

#define MULTIBOOT_MMAP_MAX       128



/**

   Structure: struct multiboot_info
   --------------------------------

   Describe the multiboot structure provided by bootloader
   Description follows GNU Multiboot 0.96 specifications

**/


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


/**

   Structure: struct multiboot_mmap_entry
   --------------------------------------

   Item in a memory map.
   Members are self explanatory.

**/

PUBLIC struct multiboot_mmap_entry
{
  u32_t size;
  u64_t addr;
  u64_t len;
  u32_t type;
}__attribute__((packed));



/**

   Structure: struct multiboot_mod_entry
   -------------------------------------

   Item in the modules list provided by bootloader
   Members are:
     - start   : Address of the first byte
     - end     : Ending address
     - cmdline : Command line (module arguments or parameters)
     - pad     : Padding to keep things aligned
   
**/

PUBLIC struct multiboot_mod_entry
{
  u32_t start;
  u32_t end;
  u32_t cmdline;
  u32_t pad;
}__attribute__((packed));




/** 

    Global: multiboot information structure
    ----------------------------------------

**/

struct multiboot_info* mbi;


/**

   Prototypes
   ----------

   Give access to main setup function

**/

PUBLIC void setup_x86(u32_t magic, physaddr_t mbi_addr);

#endif
