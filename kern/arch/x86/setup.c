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
  /* Initialize serial port */
  serial_init();

  /* Retrieve multiboot header */
  mbi = (struct multiboot_info*)mbi_addr;
  
  if (magic != MULTIBOOT_MAGIC)
    {
      goto err_magic;
    }

  if (e820_setup(mbi) != EXIT_SUCCESS)
    {
      goto err_mem;
    }

  u8_t i;
  struct multiboot_mmap_entry* mmap = (u32_t)mbi->mmap_addr;
  serial_printf("Memory Map:\n");

  for(i=0;i<mbi->mmap_length;i++)
    {
	  serial_printf("0x%x%x \t 0x%x%x \t 0x%x\n",
			(u32_t)((mmap[i].addr)>>32),(u32_t)(mmap[i].addr),
			(u32_t)((mmap[i].len)>>32),(u32_t)(mmap[i].len),
			mmap[i].type);
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

