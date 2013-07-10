/**

   setup.c
   =======

   Set up a clean x86 environnement for kernel

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - x86_const.h
   - serial.h       : output on serial port
   - e820.h         : e820 memory map
   - setup.h        : self header

**/

#include <define.h>
#include <types.h>
#include "x86_const.h"
#include "serial.h"
#include "e820.h"
#include "setup.h"




/**

   Function: void setup_x86(u32_t magic, physaddr_t mbi_addr)
   -----------------------------------------------------------

   Entry point.
   Initialize serial port for external communication
   Retrieve memory information from bootloader and correct them
   Check boot modules (user progs)
   Create GDT & IDT

**/


PUBLIC void setup_x86(u32_t magic, physaddr_t mbi_addr)
{
  struct multiboot_mod_entry* mod_entry;
  u8_t i;

  /* Initialize serial port */
  serial_init();

  /* Retrieve multiboot header */
  mbi = (struct multiboot_info*)mbi_addr;
  
  if (magic != MULTIBOOT_MAGIC)
    {
      goto err_magic;
    }

  /* Setup e820 memory map */
  if (e820_setup(mbi) != EXIT_SUCCESS)
    {
      goto err_mem;
    }
  
  /* Kernel boundaries */
  serial_printf("Kernel is at 0x%x - 0x%x\n",X86_CONST_KERN_START,X86_CONST_KERN_END);
  
  /* Boot modules */
  serial_printf("Boot modules list is at 0x%x\n",mbi->mods_addr);
  for(i=0,mod_entry=(struct multiboot_mod_entry*)mbi->mods_addr;
      i<mbi->mods_count;
      i++,mod_entry++)
    {
      serial_printf("0x%x - 0x%x %s\n",
		    mod_entry->start,
		    mod_entry->end,
		    mod_entry->cmdline);
    }


  /* Debug loop */
  while(1)
    {}

 err_mem:

  serial_printf("Memory Error\n");

 err_magic:

  serial_printf("Multiboot Error\n");

  /* Debug loop */
  while(1)
    {}

  return;
}

