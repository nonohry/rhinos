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

   Structure: struct interval_tree
   -------------------------------

   Represent a node of an interval tree.
   Members are:
   - min     : Start of interval
   - max     : End of interval
   - type    : Type (in the sense of e820 mmap entry type)
   - left    : left son
   - right   : right son

**/

struct interval_tree
{
  u64_t min;
  u64_t max;
  u32_t type;
  struct interval_tree* left;
  struct interval_tree* right;
};



/**

   Privates
   --------

   Clean and truncate a memory map

**/


PRIVATE u8_t e820_partition(struct multiboot_mmap_entry* t,u8_t start,u8_t end);
PRIVATE void e820_qsort(struct multiboot_mmap_entry* t, u8_t start, u8_t end);
PRIVATE u8_t e820_tree_insert(u64_t min, u64_t max, u32_t type, struct interval_tree** t);
PRIVATE void e820_tree_convert(struct interval_tree* t, u8_t index, struct multiboot_mmap_entry* mmap);
PRIVATE u8_t e820_tree_count(struct interval_tree* t);
PRIVATE struct interval_tree* e820_tree_alloc(void);
PRIVATE u8_t e820_truncate32b(struct multiboot_info* bootinfo);

/**
 
   Static: mmap
   ------------

   Memory map in a controlled location

**/


static struct multiboot_mmap_entry mmap[MULTIBOOT_MMAP_MAX] __attribute__((section(".data")));


/**

   Static: tree_pool
   -----------------

   Pool of `struct interval_tree` for allocation

**/


static struct interval_tree tree_pool[MULTIBOOT_MMAP_MAX] __attribute__((section(".data")));



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
  struct interval_tree* tree=NULL;
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

  /* Reverse sort mmap according to memory type */
  e820_qsort(mmap,0,bootinfo->mmap_length-1);

  /* Build tree */
  for(i=0;i<bootinfo->mmap_length;i++)
    {
      if (e820_tree_insert(mmap[i].addr,mmap[i].addr+mmap[i].len-1,mmap[i].type,&tree) != EXIT_SUCCESS)
	{
	  return EXIT_FAILURE;
	}
    }
  
  /* Update memory map lenght */
  bootinfo->mmap_length = e820_tree_count(tree);
  if (bootinfo->mmap_length == 0)
    {
	  return EXIT_FAILURE;
    }

  /* Convert tree to mmap */
  e820_tree_convert(tree,0,mmap);
  
  /* Truncate memory at 4GB */ 
  if ( e820_truncate32b(bootinfo) != EXIT_SUCCESS )
    {
      return EXIT_FAILURE;
    } 
  
  return EXIT_SUCCESS;
}


/**

   Function: u8_t partition(struct multiboot_mmap_entry* t,u8_t start,u8_t end)
   ----------------------------------------------------------------------------

   Create a memory map partition for reverse quick sorting mmap according to its `type`.
   Partition is based on a pivot element `type` which is the last element for ease of programming.

   The partition process result in a memeory map partition where all elements before the pivot have
   a greater `type`and all element after have a smaller `type`.

   It returns the pivot index in the memory map.

**/


PRIVATE u8_t e820_partition(struct multiboot_mmap_entry* t,u8_t start,u8_t end)
{
  if (start+1<end)
    {
      u8_t i,j;

      /* Partition with t[end] as a pivot */
      i=start;
      j=end-1;
      
      while( i<j )
	{
	  /* Find first index with type greater than t[end] */
	  while( (t[i].type>=t[end].type)&&(i<end) )
	    {
	      i++;
	    }
	  
	  
	  /* Find last index with type lower than t[end] */
	  while( (t[j].type<t[end].type)&&(j>start) )
	    {
	      j--;
	    }
	  
	  /* Swap if possible */
	  if ( i<j )
	    {
	      SWAP(t[i].size,t[j].size);
	      SWAP(t[i].addr,t[j].addr);
	      SWAP(t[i].len,t[j].len);
	      SWAP(t[i].type,t[j].type);
	    }
	}
     
      /* Place pivot element at the boundary */
      if (i!=end)
	{
	  SWAP(t[i].size,t[end].size);
	  SWAP(t[i].addr,t[end].addr);
	  SWAP(t[i].len,t[end].len);
	  SWAP(t[i].type,t[end].type);
	}

      /* Return pivot index */
      return i;

    }
  
  /* Return `start` as pivot index */
  return start;
  
}


/**

   Function: void e820_qsort(struct multiboot_mmap_entry* t, u8_t start, u8_t end)
   -------------------------------------------------------------------------------

   Reverse quick sort memory map according to `type` fields.
   
   It first create a partition using `e820_partition` then sort recursively 
   the two resulting parts.
   

**/


PRIVATE void e820_qsort(struct multiboot_mmap_entry* t, u8_t start, u8_t end)
{
    if (start+1<end)
    {
      u8_t n = e820_partition(t,start,end);
      e820_qsort(t,start,n-1);
      e820_qsort(t,n+1,end);
    }
  return;
}


/**

   Function: u8_t e820_tree_insert(u64_t min, u64_t max, u32_t type, struct interval_tree** t)
   -------------------------------------------------------------------------------------------

   Insert a new node made of `min`, `max` and `type` in tree `t`.
   Thus function is recursive.

**/

PRIVATE u8_t e820_tree_insert(u64_t min, u64_t max, u32_t type, struct interval_tree** t)
{
  if (*t == NULL)
   {
     /* Create a new node */
     *t=e820_tree_alloc();
     if (*t == NULL)
       {
	 return EXIT_FAILURE;
       }
     (*t)->min = min;
     (*t)->max = max;
     (*t)->type = type;
     (*t)->left = NULL;
     (*t)->right = NULL;
   }
   else
   {
     if (min < (*t)->min)
       {
	 /* Insert remaining left interval */
	 if (e820_tree_insert(min,(*t)->min > max ? max : (*t)->min-1 ,type,&((*t)->left)) != EXIT_SUCCESS)
	   {
	     return EXIT_FAILURE;
	   }
	 
       }
     
     /* Insert remaining right interval */
     if (max > (*t)->max)
       {
	 if (e820_tree_insert( (*t)->max < min ? min : (*t)->max+1 ,max,type,&((*t)->right)) != EXIT_SUCCESS)
	   {
	     return EXIT_FAILURE;
	   }
       }
   }
   
   return EXIT_SUCCESS;
}

/**

   Function: u8_t e820_tree_count(struct interval_tree* t)
   -------------------------------------------------------

   Count number of elements in a tree `t`
   This function is recursive

**/

PRIVATE u8_t e820_tree_count(struct interval_tree* t)
{
  if (t!=NULL)
   {
     return (1+e820_tree_count(t->left)+e820_tree_count(t->right));
   }
  
  return 0;
}


/**

   Function: void e820_tree_convert(struct interval_tree* t, u8_t index, struct multiboot_mmap_entry* t)
   -----------------------------------------------------------------------------------------------------

   Convert an interval tree into a memory map stored in `t`.

   Do a Deep First Search, remenbering the index where to store the values into table `t`.
   This function is recursive.

**/

PRIVATE void e820_tree_convert(struct interval_tree* t, u8_t index, struct multiboot_mmap_entry* mmap)
{
  if (t!=NULL)
    {
      u8_t n = 0;

      n = e820_tree_count(t->left);
 
      e820_tree_convert(t->left,index,mmap);

      mmap[index+n].addr=t->min;
      mmap[index+n].len=t->max - t->min + 1;
      mmap[index+n].type=t->type;

      e820_tree_convert(t->right,index+n+1,mmap);
    }

  return;
}


/**

   Function: struct interval_tree* e820_tree_alloc(void)
   -----------------------------------------------------

   Allocate a `struct interval_tree` from `tree_pool`

   Simply return the current element of `tree_pool`.

**/

PRIVATE struct interval_tree* e820_tree_alloc(void)
{
  static u8_t i=0;
  if (i<MULTIBOOT_MMAP_MAX)
    {
      return (&(tree_pool[i++]));
    }

  return NULL;
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
