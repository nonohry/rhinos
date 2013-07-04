/**

   start.c
   =======

   Setup a controlled environnement in protected mode
   creating an GDT and an IDT.

   Retrieve useful information from multiboot header, like
   boot modules or memory map
   
**/


/**
 
   Includes 
   --------

   - types.h
   - const.h
   - klib.h
   - tables.h  : IDT and GDT descriptor needed
   - start.h   : self header

**/

#include <define.h>
#include <types.h>
#include "const.h"
#include "klib.h"
#include "tables.h"
#include "start.h"


/**

  Macros: MIN and MAX
  -------------------

**/

#define MIN(_x,_y)     ((_x)<(_y)?(_x):(_y))
#define MAX(_x,_y)     ((_x)>(_y)?(_x):(_y))


/**

   Privates
   --------

   Clean and truncate a memory map

**/


PRIVATE u8_t start_mmap_sanitize(struct multiboot_info* bootinfo);
PRIVATE u8_t start_mmap_truncate32b(struct multiboot_info* bootinfo);


/**
 
   Static: start_mmap
   ------------------

   Memory map in a controlled location

**/


static struct multiboot_mmap_entry start_mmap[START_MULTIBOOT_MMAP_MAX];


/**

   Function: void start_main(u32_t magic, physaddr_t mbi_addr)
   -----------------------------------------------------------

   Entry point.
   Initialize serial port for external communication
   Retrieve memory information from bootloader and correct them
   Check boot modules (user progs)
   Create GDT & IDT

**/



PUBLIC void start_main(u32_t magic, physaddr_t mbi_addr)
{ 

  u8_t i;
  struct multiboot_mmap_entry* mmap;

  /* Initialize serial port */
  klib_serial_init();

  /* Retrieve multiboot header */
  start_mbi = (struct multiboot_info*)mbi_addr;

  if (magic != START_MULTIBOOT_MAGIC)
    {
      goto err_magic;
    }
      

  /* Relocate memory map */
  if (start_mbi->flags & START_MULTIBOOT_FLAG_MMAP)
    {
      for(mmap = (struct multiboot_mmap_entry*)start_mbi->mmap_addr, i=0;
	  (unsigned long)mmap <  start_mbi->mmap_addr + start_mbi->mmap_length;
	  i++, mmap = (struct multiboot_mmap_entry*)((unsigned long)mmap + mmap->size + sizeof(mmap->size)))
	{
	
	  start_mmap[i].size = mmap->size;
	  start_mmap[i].addr = mmap->addr;
	  start_mmap[i].len = mmap->len;
	  start_mmap[i].type = mmap->type;

	  /* Too much entries ! Probably buggy memory map */
	  if (i>START_MULTIBOOT_MMAP_MAX)
	    {
	       goto err_mem;
	    }
	  
	}
    }
  else
    {
      /* Build a memory memory map with upper/lower memory information */
      if (start_mbi->flags & START_MULTIBOOT_FLAG_MEMORY)
	{

	  if (!(start_mbi->mem_lower && start_mbi->mem_upper))
	    {
	      goto err_mem;
	    }

	  /* Number of entries in the memory map */
	  i=3;
	  
	  start_mmap[0].size = sizeof(struct multiboot_mmap_entry);
	  start_mmap[0].addr = 4096;
	  start_mmap[0].len = start_mbi->mem_lower*1024;
	  start_mmap[0].type = START_E820_AVAILABLE;

	  start_mmap[1].size = sizeof(struct multiboot_mmap_entry);
	  start_mmap[1].addr = CONST_ROM_AREA_START;
	  start_mmap[1].len = CONST_ROM_AREA_SIZE;
	  start_mmap[1].type = START_E820_RESERVED;


	  start_mmap[2].size = sizeof(struct multiboot_mmap_entry);
	  start_mmap[2].addr = 0x100000;
	  start_mmap[2].len = start_mbi->mem_upper*1024;
	  start_mmap[2].type = START_E820_AVAILABLE;
	  
	}
      else
	{
	  /* No memory information, Error */
	   goto err_mem;
	}
    }

  /* Make the multiboot hearder memory map point to our relocated memory map */
  start_mbi->mmap_addr = (u32_t)start_mmap;
  /* Set number of entries */
  start_mbi->mmap_length = i;


  /* Sanitize memory map */
  if ( start_mmap_sanitize(start_mbi) != EXIT_SUCCESS )
    {
      goto err_mem;
    }

  /* Truncate memory at 4GB */
  if ( start_mmap_truncate32b(start_mbi) != EXIT_SUCCESS )
    {
      goto err_mem;
    }    

  /* Compute total available memory */
  start_mem_total = 0;
  mmap = (struct multiboot_mmap_entry*)start_mbi->mmap_addr;
  for(i=0;i<start_mbi->mmap_length;i++)
    {
      start_mem_total += mmap[i].len;
      
    }

  /* Check boot modules */
  if (!((start_mbi->flags & START_MULTIBOOT_FLAG_MODS)&&(start_mbi->mods_count == CONST_BOOT_MODULES)))
    {
      goto err_mods;
    }

  /* Initialize GDT & IDT */
  if ( (gdt_init() != EXIT_SUCCESS)||(idt_init() != EXIT_SUCCESS) ) 
    {
      goto err_tables;
    }
  klib_printf("GDT & IDT initialized\n");
  
  /* That's all folks */
  return;
  
 err_magic:
  klib_printf("Bad Magic Number\n");
  
 err_mem:
  klib_printf("Memory Error\n");
  
 err_mods:
  klib_printf("No Module Found\n");

 err_tables:
  klib_printf("GDT & IDT Error\n");
  
  while(1){}

  return;
}



/**

   Function: u8_t start_mmap_sanitize(struct multiboot_info* bootinfo)
   -------------------------------------------------------------------

   Sanitize memory map. Memory map provided by bootloader can have overlaps.
   The function merges these overlaps and sort the entry by starting address.
   
   It's a glutton algorithm that inspect entries one by one. If an overlap
   is discovered, we merge entries in terms of entries type. If new entries
   are created during merging, they are added at the end of the memory map.
   
**/



PRIVATE u8_t start_mmap_sanitize(struct multiboot_info* bootinfo)
{
  u8_t i,j,flag;
  u64_t A,B,C,D;
  struct multiboot_mmap_entry* mmap;
  struct multiboot_mmap_entry tmpEntry;

  mmap = (struct multiboot_mmap_entry*)bootinfo->mmap_addr;

  /* Glutton run through */
  for(i=0;i<bootinfo->mmap_length-1;i++)
    {
      /* Segment extremities */
      A=mmap[i].addr;
      B=mmap[i].len+mmap[i].addr-1;

      for(j=i+1;j<bootinfo->mmap_length;j++)
	{
	  /* Flag (re)initialization  */
	  flag = 3;

	  /* comapred segment extremities */
	  C=mmap[j].addr;
	  D=mmap[j].len+mmap[j].addr-1;

	  /* Overlap ! */
	  if ((s64_t)(MIN(B,D)-MAX(A,C))>0)
	    {
	      /* Flag represents overlap degree */
	      if (A!=C) flag&=2;
	      if (B!=D) flag&=1;

	      switch(flag)
		{
		case 0:
		  {
		    /* 3 new entries creation */
		    if (bootinfo->mmap_length+1 < START_MULTIBOOT_MMAP_MAX)
		      {
			u8_t t0,t1,t2;
			
			/* Types for each segment */
			t0 = (MIN(A,C) == A ? mmap[i].type : mmap[j].type);
			t1 = MAX(mmap[i].type,mmap[j].type);
			t2 = (MAX(B,D) == B ? mmap[i].type : mmap[j].type);

			mmap[i].addr = MIN(A,C);
			mmap[i].len = MAX(A,C)-MIN(A,C);
			mmap[i].type = t0;

			mmap[j].addr = MAX(A,C);
			mmap[j].len = MIN(B,D)-MAX(A,C)+1;
			mmap[j].type = t1;

			mmap[bootinfo->mmap_length].addr = MIN(B,D)+1;
			mmap[bootinfo->mmap_length].len = MAX(B,D)-MIN(B,D);
			mmap[bootinfo->mmap_length].type = t2;

			/* We have created one more entry */
			bootinfo->mmap_length++; 
		      }
		    else
		      {
			/* Too much overlaps */
			return EXIT_FAILURE;
		      }

		    break;
		  }
		case 1:
		  {
		    /* First extremity is shared, 2 entries creation */
		    u8_t t0,t1;
			
		    /* Types for each segment */
		    t0 = MAX(mmap[i].type,mmap[j].type);
		    t1 = (MAX(B,D) == B ? mmap[i].type : mmap[j].type);

		    mmap[i].addr = MAX(A,C);
		    mmap[i].len = MIN(B,D)-MAX(A,C)+1;
		    mmap[i].type = t0;
		    
		    mmap[j].addr = MIN(B,D)+1;
		    mmap[j].len = MAX(B,D)-MIN(B,D);
		    mmap[j].type = t1;
		    

		    break;
		  }
		case 2:
		  {
		    /* Last extremity is shared, 2 entries creation */
		    u8_t t0,t1;

		    t0 = (MIN(A,C) == A ? mmap[i].type : mmap[j].type);
		    t1 = MAX(mmap[i].type,mmap[j].type);
		    
		    mmap[i].addr = MIN(A,C);
		    mmap[i].len = MAX(A,C)-MIN(A,C);
		    mmap[i].type = t0;

		    mmap[j].addr = MAX(A,C);
		    mmap[j].len = MIN(B,D)-MAX(A,C)+1;
		    mmap[j].type = t1;


		    break;
		  }
		case 3:
		  {
		    /* Segments are mingled, we remove the one with lowest type */
		    u8_t k;

		    /* Get higest type */
		    mmap[i].type = MAX(mmap[i].type,mmap[j].type);
		    
		    /* Remove mingled segment */
		    for(k=j;k<bootinfo->mmap_length-1;k++)
		      {
			mmap[k]=mmap[k+1];
		      }
		    
		    bootinfo->mmap_length--;

		    break;
		  }
		default:
		  {
		    return EXIT_FAILURE;
		  }
		}
	      
	      /* We have removed an entry */
	      i--;
	      break;

	    }
	}
    }

  /* Memory map bubble sort */
  do
    {
      flag=0;
      for(i=0;i<bootinfo->mmap_length-1;i++)
	{
	  if (mmap[i].addr > mmap[i+1].addr)
	    {
	      /* Entries exchange */
	      tmpEntry.addr=mmap[i].addr;
	      tmpEntry.len=mmap[i].len;
	      tmpEntry.type=mmap[i].type;

	      mmap[i].addr=mmap[i+1].addr;
	      mmap[i].len=mmap[i+1].len;
	      mmap[i].type=mmap[i+1].type;

	      mmap[i+1].addr=tmpEntry.addr;
	      mmap[i+1].len=tmpEntry.len;
	      mmap[i+1].type=tmpEntry.type;

	      flag=1;

	    }
	}
    }while(flag);


  /* Memory map smooth */
  for(i=0;i<bootinfo->mmap_length-1;i++)
    {
      /* Merge entries if they share extremities and have same type */
      if ( (mmap[i].addr+mmap[i].len == mmap[i+1].addr)&&(mmap[i].type == mmap[i+1].type) )
	{
	  /* Update size */
	  mmap[i].len = mmap[i].len+mmap[i+1].len;

	  /* Remove the second entry */
	  for(j=i+1;j<bootinfo->mmap_length-1;j++)
	    {
	      mmap[j]=mmap[j+1];
	    }

	  /* Update number of entries */
	  bootinfo->mmap_length--;

	  /* We have removed an entry */
	  i--;

	}
    }

  return EXIT_SUCCESS;
}



/**

   Function: u8_t start_mmap_truncate32b(struct multiboot_info* bootinfo)
   ----------------------------------------------------------------------

   Truncate memory to limit it to 4GB in a sorted memory map

**/


PRIVATE u8_t start_mmap_truncate32b(struct multiboot_info* bootinfo)
{
  u8_t i;
  struct multiboot_mmap_entry* mmap;

  mmap = (struct multiboot_mmap_entry*)bootinfo->mmap_addr;

  for(i=0;i<bootinfo->mmap_length;i++)
    {

      /* Set memory above 4GB as reserved */
      if ( (mmap[i].addr>>32)&&(mmap[i].type == START_E820_AVAILABLE) )
	{
	  mmap[i].type = START_E820_RESERVED;
	}

      /* Astride available memory */
      if ((!(mmap[i].addr>>32))&& ((mmap[i].addr+mmap[i].len)>>32)&&(mmap[i].type == START_E820_AVAILABLE) )
	{
	  mmap[i].len = (u32_t)(-1) - mmap[i].addr;
	}

      
    }
  
  return EXIT_SUCCESS;
}
