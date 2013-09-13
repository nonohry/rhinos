/**

   proc.c
   ======

   Process management

**/



/**

   Includes
   ========

   - define.h
   - types.h
   - llist.h
   - arch_const.h    : architecture dependant constant
   - arch_vm.h        :architecture dependant virtual memory
   - vm_slab.h       : slab allocator needed
   - thread.h        : struct thread needed
   - proc.h          : self header

**/


#include <define.h>
#include <types.h>
#include <llist.h>
#include <arch_const.h>
#include <arch_vm.h>
#include "vm_slab.h"
#include "thread.h"
#include "proc.h"



/**

   Global: proc_cache
   ------------------

   Cache for `struct proc` allocation

**/


struct vm_cache* proc_cache;


/**

   Global: thread_wrapper_cache
   -----------------------------

   Cache for `struct thread_wrapper` allocation

**/


struct vm_cache* thread_wrapper_cache;


/**

   Global: addrspace_cache
   -----------------------------

   Cache for address space allocation

**/


struct vm_cache* addrspace_cache;



/**
   
   Function:  u8_t proc_setup(void)
   --------------------------------

   Setup proc subsystem.

   Create a cache for structures allocations

**/


PUBLIC u8_t proc_setup(void)
{
  /* Create cache for `struct proc` allocation */
  proc_cache = vm_cache_create("Proc_Cache",sizeof(struct proc));
  if (proc_cache == NULL)
    {
      return EXIT_FAILURE;
    }
  
  /* Create cache for `struct thread_wrapper` allocation */
  thread_wrapper_cache = vm_cache_create("ThreadWrapper_Cache",sizeof(struct thread_wrapper));
  if (thread_wrapper_cache == NULL)
    {
      goto err0;
    }

  /* Create cache for address space allocation */
  addrspace_cache = vm_cache_create("AddrSpace_Cache",ARCH_CONST_ADDRSPACE_SIZE);
  if (addrspace_cache == NULL)
    {
      goto err1;
    }
  
  return EXIT_SUCCESS;

 err1:
  vm_cache_destroy(thread_wrapper_cache);

 err0:
  vm_cache_destroy(proc_cache);

  return EXIT_FAILURE;
}



/**

   Function: struct proc* proc_create(char* name)
   ----------------------------------------------

   Create a process named `name`. 
   It create a page directory and synchronize it with the kernel one.
   Return the brand new process or NULL if it fails.

**/


PUBLIC struct proc* proc_create(char* name)
{
  struct proc* proc=NULL;
  u16_t i;

  /* Allocate a process */
  //proc = (struct proc*)mem_alloc(sizeof(struct proc));
  if (proc == NULL)
    {
      return NULL;
    }

  /* Name check */
  if (name == NULL)
    {
      name = "NONAME";
    }

  /* Name copy*/
  i=0;
  while( (name[i]!=0)&&(i<PROC_NAMELEN-1) )
    {
      proc->name[i] = name[i];
      i++;
    }
  proc->name[i]=0;

  /* Threads list initialization */
  LLIST_NULLIFY(proc->thread_list);

  /* Clean page directory */
  klib_mem_set(0,(addr_t)pd,PAGING_ENTRIES*sizeof(struct pde));

  /* Synchronize page directory with kernel space */
  kern_pd = (struct pde*)PAGING_GET_PD();
  for(i=0;i<CONST_KERN_HIGHMEM/CONST_PAGE_SIZE/PAGING_ENTRIES;i++)
    {
      pd[i]=kern_pd[i];
    }

  /* link page directory to the process structure */
  proc->v_pd = pd;
  proc->p_pd = paging_virt2phys((virtaddr_t)pd);
  if (!(proc->p_pd))
    {
      goto err01;
    }

  /* Self Mapping */
  proc->v_pd[PAGING_SELFMAP].present = 1;
  proc->v_pd[PAGING_SELFMAP].rw = 1;
  proc->v_pd[PAGING_SELFMAP].user = 0;
  proc->v_pd[PAGING_SELFMAP].baseaddr = proc->p_pd >> PAGING_BASESHIFT;

  return proc;


  return NULL;
}


/**

   Function: u8_t proc_add_thread(struct proc* proc, struct thread* th)
   --------------------------------------------------------------------

   Add the thread `th` to the process `proc` using a helper structure thread_info

**/


PUBLIC u8_t proc_add_thread(struct proc* proc, struct thread* th)
{
  struct thread_info* thinfo;

  if ( (th == NULL) || (proc == NULL) )
    {
      return EXIT_FAILURE;
    }

  /* Allocate thread_info */
  thinfo = (struct thread_info*)virtmem_cache_alloc(thread_info_cache,VIRT_CACHE_DEFAULT);
  if (thinfo == NULL)
    {
      return EXIT_FAILURE;
    }

  /* thread back pointer to process */
  th->proc = proc;
  /* Set thread in thread info */
  thinfo->thread = th;
  /* Link thread_info with process other threads */
  LLIST_ADD(proc->thread_list,thinfo);

  return EXIT_SUCCESS;
}


/**

   Function: u8_t proc_remove_thread(struct proc* proc, struct thread* th)
   -----------------------------------------------------------------------

   Remove thread `th` from process `proc`

**/


PUBLIC u8_t proc_remove_thread(struct proc* proc, struct thread* th)
{
  struct thread_info* thinfo;

  if ( (th == NULL) || (proc == NULL) || LLIST_ISNULL(proc->thread_list) )
    {
      return EXIT_FAILURE;
    }

  /* Look for the corresponding thread_info */
  thinfo = LLIST_GETHEAD(proc->thread_list);
  do
    {
      if (thinfo->thread == th)
	{
	  /* Found ! Remove it from the list */
	  LLIST_REMOVE(proc->thread_list, thinfo);
	  /* Free it */
	  virtmem_cache_free(thread_info_cache,thinfo);
	  return EXIT_SUCCESS;
	}
      /* Run through linked list */
      thinfo = LLIST_NEXT(proc->thread_list,thinfo);
    }while(!LLIST_ISHEAD(proc->thread_list,thinfo));
    

  return EXIT_FAILURE;
}
