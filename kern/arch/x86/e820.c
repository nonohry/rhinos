/**

   e820.c
   ======


   E820 Memory Map management

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - x86_const.h
   - setup.h       : multiboot information
   - e820.h        : self header

**/


#include <define.h>
#include <types.h>
#include "x86_const.h"
#include "setup.h"
#include "e820.h"



/**
   
   Constants: types of memory in a E820 memory map
   -----------------------------------------------

**/

#define E820_AVAILABLE    0x1
#define E820_RESERVED     0x2
#define E820_ACPI         0x3
#define E820_ACPI_NVS     0x4


/**

  Macros: MIN and MAX
  -------------------

**/


#define MIN(_x,_y)     ((_x)<(_y)?(_x):(_y))
#define MAX(_x,_y)     ((_x)>(_y)?(_x):(_y))


/**

   Macro: SWAP
   -----------

   swap 2 values via XOR

**/

#define SWAP(__x,__y)				\
        (__x) = (__x) ^ (__y);			\
	(__y) = (__x) ^ (__y);			\
	(__x) = (__x) ^ (__y);


/**

   Privates
   --------

   Clean and truncate a memory map

**/


PRIVATE u8_t e820_sanitize(struct multiboot_info* bootinfo);
PRIVATE u8_t e820_truncate32b(struct multiboot_info* bootinfo);



/**
 
   Static: mmap
   ------------

   Memory map in a controlled location

**/


struct multiboot_mmap_entry mmap[MULTIBOOT_MMAP_MAX];



/**

   DEBUG

**/

#include "serial.h"

void print_mmap(u32_t n)
{
  u8_t i;

  serial_printf("\n");
  for(i=0;i<n;i++)
    {
      serial_printf("addr: 0x%x%x \t len: 0x%x%x \t type: 0x%x\n",
		    (u32_t)(mmap[i].addr >> 32), (u32_t)mmap[i].addr,
		    (u32_t)(mmap[i].len >> 32), (u32_t)mmap[i].len,
		    mmap[i].type);
    }
  
  return;
}
  


/**

   Function: u8_t e820_setup(struct multiboot_info* bootinfo)
   ----------------------------------------------------------


   Create a sanitized and sorted e820 memory map
   First, it relocates the multiboot memory map in a controlled area.
   Then the relocated memory map is sanitized and truncate to 4GB

**/



PUBLIC u8_t e820_setup(struct multiboot_info* bootinfo)
{
  struct multiboot_mmap_entry* entry;
  u8_t i;


  /* Buld memory map in a controled area */
  if (bootinfo->flags & MULTIBOOT_FLAG_MMAP)
    {
      for(entry = (struct multiboot_mmap_entry*)bootinfo->mmap_addr, i=0;
	  (unsigned long)entry <  bootinfo->mmap_addr + bootinfo->mmap_length;
	  i++, entry = (struct multiboot_mmap_entry*)((unsigned long)entry + entry->size + sizeof(entry->size)))
	{
	
	  mmap[i].size = entry->size;
	  mmap[i].addr = entry->addr;
	  mmap[i].len = entry->len;
	  mmap[i].type = entry->type;

	  /* Too much entries ! Probably buggy memory map */
	  if (i>MULTIBOOT_MMAP_MAX)
	    {
	      return EXIT_FAILURE;
	    }
	  
	}
    }
  else
    {
      /* Build a memory memory map with upper/lower memory information */
      if (bootinfo->flags & MULTIBOOT_FLAG_MEMORY)
	{

	  if (!(bootinfo->mem_lower && bootinfo->mem_upper))
	    {
	      return EXIT_FAILURE;
	    }

	  /* Number of entries in the memory map */
	  i=3;
	  
	  mmap[0].size = sizeof(struct multiboot_mmap_entry);
	  mmap[0].addr = 4096;
	  mmap[0].len = bootinfo->mem_lower*1024;
	  mmap[0].type = E820_AVAILABLE;

	  mmap[1].size = sizeof(struct multiboot_mmap_entry);
	  mmap[1].addr = X86_CONST_ROM_AREA_START;
	  mmap[1].len = X86_CONST_ROM_AREA_SIZE;
	  mmap[1].type = E820_RESERVED;


	  mmap[2].size = sizeof(struct multiboot_mmap_entry);
	  mmap[2].addr = 0x100000;
	  mmap[2].len = bootinfo->mem_upper*1024;
	  mmap[2].type = E820_AVAILABLE;
	  
	}
      else
	{
	  /* No memory information, Error */
	   return EXIT_FAILURE;
	}
    }

  /* Make the multiboot header memory map point to our relocated memory map */
  bootinfo->mmap_addr = (u32_t)mmap;
  /* Set number of entries */
  bootinfo->mmap_length = i;

  print_mmap(bootinfo->mmap_length);
  while(1);

 
  /* Sanitize memory map */
  if ( e820_sanitize(bootinfo) != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    }
  
  
  /* Truncate memory at 4GB */ 
  if ( e820_truncate32b(bootinfo) != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    } 
  
  return EXIT_SUCCESS;
}



/**

   Function: u8_t e820_sanitize(struct multiboot_info* bootinfo)
   -------------------------------------------------------------------

   Sanitize memory map. Memory map provided by bootloader can have overlaps.
   The function merges these overlaps and sort the entry by starting address.
   
   It's a glutton algorithm that inspect entries one by one. If an overlap
   is discovered, we merge entries in terms of entries type. If new entries
   are created during merging, they are added at the end of the memory map.
   
**/


PRIVATE u8_t e820_sanitize(struct multiboot_info* bootinfo)
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
		    if (bootinfo->mmap_length+1 < MULTIBOOT_MMAP_MAX)
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

   Function: u8_t e820_truncate32b(struct multiboot_info* bootinfo)
   ----------------------------------------------------------------------

   Truncate memory to limit it to 4GB in a sorted memory map

**/


PRIVATE u8_t e820_truncate32b(struct multiboot_info* bootinfo)
{
  u8_t i;
  struct multiboot_mmap_entry* mmap;

  mmap = (struct multiboot_mmap_entry*)bootinfo->mmap_addr;

  for(i=0;i<bootinfo->mmap_length;i++)
    {

      /* Set memory above 4GB as reserved */
      if ( (mmap[i].addr>>32)&&(mmap[i].type == E820_AVAILABLE) )
	{
	  mmap[i].type = E820_RESERVED;
	}

      /* Astride available memory */
      if ((!(mmap[i].addr>>32))&& ((mmap[i].addr+mmap[i].len)>>32)&&(mmap[i].type == E820_AVAILABLE) )
	{
	  mmap[i].len = (u32_t)(-1) - mmap[i].addr;
	}

      
    }
  
  return EXIT_SUCCESS;
}
