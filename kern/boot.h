/**

   boot.h
   ======

   Header file relative to boot information


**/



#ifndef BOOT_H
#define BOOT_H


/**

   Includes
   --------

   - define.h
   - types.h
 
**/

#include <define.h>
#include <types.h>


/**
   
   Constants: types of memory in a memory map
   -------------------------------------------

**/

#define BOOT_AVAILABLE    0x1
#define BOOT_RESERVED     0x2
#define BOOT_ACPI         0x3
#define BOOT_ACPI_NVS     0x4

/**

   Structure: struct boot_info
   ---------------------------

   Structure containing information retrieved during boot
   and architecture dependant setup
   Members are:

   - mods_count   : Number of boot modules
   - mods_addr    : Boot modules list address
   - mmap_length  : Number of memory map entries
   - mmap_addr    : Memory map address
   - start        : First available byte after kernel

**/


PUBLIC struct boot_info
{
  u32_t  mods_count;
  addr_t mods_addr;
  u32_t  mmap_length;
  addr_t mmap_addr;
  addr_t bitmap;
  addr_t start; 
}__attribute__((packed));


/**

   Structure: struct boot_mmap_entry
   --------------------------------------

   Item in a memory map.
   Members are self explanatory.

**/

PUBLIC struct boot_mmap_entry
{
  u32_t size;
  u64_t addr;
  u64_t len;
  u32_t type;
}__attribute__((packed));



/**

   Structure: struct boot_mod_entry
   -------------------------------------

   Item in the modules list provided by bootloader
   Members are:
     - start   : Address of the first byte
     - end     : Ending address
     - cmdline : Command line (module arguments or parameters)
     - pad     : Padding to keep things aligned
   
**/


PUBLIC struct boot_mod_entry
{
  addr_t start;
  addr_t end;
  u32_t cmdline;
  u32_t pad;
}__attribute__((packed));


/**

   Global: boot
   ------------
   
**/   


struct boot_info boot;


#endif
