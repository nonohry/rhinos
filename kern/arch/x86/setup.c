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
   - x86_lib.h
   - serial.h       : output on serial port
   - e820.h         : e820 memory map
   - vm_segment.h   : segmentation setup
   - vm_paging.h    : paging setup
   - setup.h        : self header

**/

#include <define.h>
#include <types.h>
#include "x86_const.h"
#include "x86_lib.h"
#include "serial.h"
#include "e820.h"
#include "vm_segment.h"
#include "vm_paging.h"
#include "setup.h"




/**

   Static: mods_list
   -----------------

   Boot modules list in a controlled area

**/


static struct multiboot_mod_entry mods_list[MULTIBOOT_MODS_COUNT] __attribute__((section(".data")));


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
  u32_t limit;

  /* Initialize serial port */
  serial_init();

  /* Multiboot check */
  if (magic != MULTIBOOT_MAGIC)
    {
      goto err;
    }

  /* Retrieve multiboot header */
  x86_mem_copy(mbi_addr,(u32_t)&mbi,sizeof(struct multiboot_info));

  /* Setup e820 memory map */
  if (e820_setup(&mbi) != EXIT_SUCCESS)
    {
      serial_printf("Memory Error\n");
      goto err;
    }
  
  /* Check boot modules */
  if (mbi.mods_count != MULTIBOOT_MODS_COUNT)
    {
      serial_printf("Boot modules count error\n");
      goto err;
    }

  /* Relocate boot modules lists */
  for(i=0,mod_entry=(struct multiboot_mod_entry*)mbi.mods_addr;
      i<mbi.mods_count;
      i++,mod_entry++)
    {
      
      mods_list[i] = *mod_entry;
    }

  /* Relocate boot modules */
  limit = X86_ALIGN_SUP(X86_CONST_KERN_END);
  for(i=0;i<MULTIBOOT_MODS_COUNT;i++)
    {
 
      if (mods_list[i].start >= limit)
	{
	  /* Copy boot module */
	  x86_mem_copy(mods_list[i].start,limit,mods_list[i].end - mods_list[i].start + 1);

	  /* Update module list */
	  mods_list[i].end = limit + mods_list[i].end - mods_list[i].start + 1;
	  mods_list[i].start = limit;

	  limit += X86_ALIGN_SUP((mods_list[i].end - mods_list[i].start + 1));

	}
      else
	{
	  serial_printf("Overlapping boot module: 0x%x - 0x%x\n",mods_list[i].start,mods_list[i].end);
	  goto err;
	}
    }

  /* Update multiboot info boot modules part */
  mbi.mods_addr = (u32_t)mods_list;

  /* Note: `limit` is now the first available byte address in upper mem */

  /* Memory model setup */
  vm_segment_setup();
  vm_paging_setup(limit);

  return;

 err:

  serial_printf("Multiboot Error\n");

  /* Debug loop */
  while(1)
    {}

  return;
}

