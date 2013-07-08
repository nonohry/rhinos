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
   - serial.h       : output on serial port
   - setup.h        : self header

**/

#include <define.h>
#include <types.h>
#include "x86_const.h"
#include "serial.h"
#include "setup.h"



/**

  Macros: MIN and MAX
  -------------------

**/


#define MIN(_x,_y)     ((_x)<(_y)?(_x):(_y))
#define MAX(_x,_y)     ((_x)>(_y)?(_x):(_y))



/**
 
   Static: start_mmap
   ------------------

   Memory map in a controlled location

**/


static struct multiboot_mmap_entry mmap[MULTIBOOT_MMAP_MAX];



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

  struct multiboot_info* mbi;
  struct multiboot_mmap_entry* entry;
  u8_t i;

  /* Initialize serial port */
  serial_init();

  /* Retrieve multiboot header */
  mbi = (struct multiboot_info*)mbi_addr;
  
  if (magic != MULTIBOOT_MAGIC)
    {
      goto err_magic;
    }

  /* Relocate memory map */
  if (mbi->flags & MULTIBOOT_FLAG_MMAP)
    {
      for(entry = (struct multiboot_mmap_entry*)mbi->mmap_addr, i=0;
	  (unsigned long)entry <  mbi->mmap_addr + mbi->mmap_length;
	  i++, entry = (struct multiboot_mmap_entry*)((unsigned long)entry + entry->size + sizeof(entry->size)))
	{
	
	  mmap[i].size = entry->size;
	  mmap[i].addr = entry->addr;
	  mmap[i].len = entry->len;
	  mmap[i].type = entry->type;


	  serial_printf("0x%x%x \t 0x%x%x \t 0x%x\n",
			(u32_t)((mmap[i].addr)>>32),(u32_t)(mmap[i].addr),
			(u32_t)((mmap[i].len)>>32),(u32_t)(mmap[i].len),
			mmap[i].type);


	  /* Too much entries ! Probably buggy memory map */
	  if (i>MULTIBOOT_MMAP_MAX)
	    {
	      goto err_mem;
	    }
	  
	}
    }
  else
    {
      /* Build a memory memory map with upper/lower memory information */
      if (mbi->flags & MULTIBOOT_FLAG_MEMORY)
	{

	  if (!(mbi->mem_lower && mbi->mem_upper))
	    {
	      goto err_mem;
	    }

	  /* Number of entries in the memory map */
	  i=3;
	  
	  mmap[0].size = sizeof(struct multiboot_mmap_entry);
	  mmap[0].addr = 4096;
	  mmap[0].len = mbi->mem_lower*1024;
	  mmap[0].type = E820_AVAILABLE;

	  mmap[1].size = sizeof(struct multiboot_mmap_entry);
	  mmap[1].addr = X86_CONST_ROM_AREA_START;
	  mmap[1].len = X86_CONST_ROM_AREA_SIZE;
	  mmap[1].type = E820_RESERVED;


	  mmap[2].size = sizeof(struct multiboot_mmap_entry);
	  mmap[2].addr = 0x100000;
	  mmap[2].len = mbi->mem_upper*1024;
	  mmap[2].type = E820_AVAILABLE;
	  
	}
      else
	{
	  /* No memory information, Error */
	   goto err_mem;
	}
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
