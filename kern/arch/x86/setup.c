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
   - boot.h         : Kernel boot info structure
   - x86_const.h
   - x86_lib.h
   - serial.h       : output on serial port
   - e820.h         : e820 memory map
   - vm_segment.h   : segmentation setup
   - vm_paging.h    : paging setup
   - pic.h          : pic setup
   - pit.h          : pit setup
   - interrupt.h    : interrupt setup
   - setup.h        : self header

**/

#include <define.h>
#include <types.h>
#include <boot.h>
#include "x86_const.h"
#include "x86_lib.h"
#include "serial.h"
#include "e820.h"
#include "vm_segment.h"
#include "pic.h"
#include "pit.h"
#include "vm_paging.h"
#include "interrupt.h"
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
  size_t bitmap_size,vm_stack_size;
  u32_t mem=0;
  struct boot_mmap_entry* mmap;
  physaddr_t bitmap,limit,vm_stack;

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


  /* Run through memory map to get memory amount */
  mmap = (struct boot_mmap_entry*)mbi.mmap_addr;
  for(i=0;i<mbi.mmap_length;i++)
    {
      /* Update mem */
      mem += mmap[i].len;
      
    }
  /* Compute bitmap size */
  bitmap_size = (mem >> X86_CONST_PAGE_SHIFT)>>3;
  /* Reserve the bitmap  */
  bitmap = limit;
  /* Update first available byte */
  limit +=  (((bitmap_size >> X86_CONST_PAGE_SHIFT)+1) << X86_CONST_PAGE_SHIFT);


  /* Compute virtual page stack size */
  vm_stack_size = (X86_CONST_KERN_HIGHMEM >> X86_CONST_PAGE_SHIFT);
  /* Reserve pages stack */
  vm_stack = limit;
 /* Update first available byte */
  limit +=  (((vm_stack_size >> X86_CONST_PAGE_SHIFT)+1) << X86_CONST_PAGE_SHIFT);
  
  /* Setup PIC */
  if (pic_setup() != EXIT_SUCCESS)
    {
      serial_printf("PIC setup error\n");
      goto err;
    }

  /* Setup PIT */
  if (pit_setup() != EXIT_SUCCESS)
    {
      serial_printf("PIT setup error\n");
      goto err;
    }

  /* Setup Interrupts */
  if (interrupt_setup() != EXIT_SUCCESS)
    {
      serial_printf("Interrupt setup error\n");
      goto err;
    }

  /* Memory model setup */
  if (vm_segment_setup() != EXIT_SUCCESS)
    {
      serial_printf("Segmentation setup error\n");
      goto err;
    }

  if (vm_paging_setup(bitmap,&limit) != EXIT_SUCCESS)
    {
      serial_printf("Paging setup error\n");
      goto err;
    }

  /* Note: `limit` is now the first available byte address in upper mem */

  /* Fill kernel boot info structure */
  boot.mods_count = mbi.mods_count;
  boot.mods_addr = mbi.mods_addr;
  boot.mmap_length = mbi.mmap_length;
  boot.mmap_addr = mbi.mmap_addr;
  boot.bitmap = bitmap;
  boot.bitmap_size = bitmap_size;
  boot.vm_stack = vm_stack;
  boot.vm_stack_size = vm_stack_size;
  boot.start = limit;

  return;

 err:

  serial_printf("Setup Error\n");

  /* Debug loop */
  while(1)
    {}

  return;
}

