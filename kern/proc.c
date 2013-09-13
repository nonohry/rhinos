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
   - arch_io.h       : memcopy
   - arch_vm.h       : architecture dependant virtual memory
   - vm_slab.h       : slab allocator needed
   - thread.h        : struct thread needed
   - proc.h          : self header

**/


#include <define.h>
#include <types.h>
#include <llist.h>
#include <arch_io.h>
#include <arch_vm.h>
#include "vm_pool.h"
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

   Global: ksetup_proc
   -------------------

   Kernel setup execution proc

**/

struct thread ksetup_proc;



/**
   
   Function:  u8_t proc_setup(void)
   --------------------------------

   Setup proc subsystem.

   Create a cache for structures allocations
   Create a proc for kernel setup flow

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


  return EXIT_SUCCESS;

 err0:
  vm_cache_destroy(proc_cache);

  return EXIT_FAILURE;
}



/**

   Function: struct proc* proc_create(char* name)
   ----------------------------------------------

   Create a process named `name`.
 
   create a synchronized adress space.
   Return the brand new process or NULL if it fails.

**/


PUBLIC struct proc* proc_create(char* name)
{
  struct proc* proc=NULL;
  virtaddr_t asp;
  u16_t i;

  /* Allocate a process */
  proc = (struct proc*)vm_cache_alloc(proc_cache);
  if (proc == NULL)
    {
      return NULL;
    }

  /* Allocate an address space (which is page size) */
  asp = (virtaddr_t)vm_pool_alloc();
  if (!asp)
    {
      goto err0;
    }
  proc->addrspace = asp;

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


  /* Sync address space with kernel */
  if (arch_sync_addrspace(proc->addrspace) != EXIT_SUCCESS)
    {
      goto err1;
    }

  return proc;

 err1:

  vm_pool_free(asp);

 err0:
  
  vm_cache_free(proc_cache,proc);

  return NULL;
}


/**

   Function: u8_t proc_add_thread(struct proc* proc, struct thread* th)
   --------------------------------------------------------------------

   Add the thread `th` to the process `proc` using a helper structure

**/


PUBLIC u8_t proc_add_thread(struct proc* proc, struct thread* th)
{
  struct thread_wrapper* wrapper;

  if ( (th == NULL) || (proc == NULL) )
    {
      return EXIT_FAILURE;
    }

  /* Allocate thread_info */
  wrapper = (struct thread_wrapper*)vm_cache_alloc(thread_wrapper_cache);
  if (wrapper == NULL)
    {
      return EXIT_FAILURE;
    }

  /* thread back pointer to process */
  th->proc = proc;
  /* Set thread in thread info */
  wrapper->thread = th;
  /* Link thread_info with process other threads */
  LLIST_ADD(proc->thread_list,wrapper);

  return EXIT_SUCCESS;
}


/**

   Function: u8_t proc_remove_thread(struct proc* proc, struct thread* th)
   -----------------------------------------------------------------------

   Remove thread `th` from process `proc`

**/


PUBLIC u8_t proc_remove_thread(struct proc* proc, struct thread* th)
{
  struct thread_wrapper* wrapper;

  if ( (th == NULL) || (proc == NULL) || LLIST_ISNULL(proc->thread_list) )
    {
      return EXIT_FAILURE;
    }

  /* Look for the corresponding thread_wrapper */
  wrapper = LLIST_GETHEAD(proc->thread_list);
  do
    {

      if (wrapper->thread == th)
	{
	  /* Found ! Remove it from the list */
	  LLIST_REMOVE(proc->thread_list, wrapper);
	  /* Free it */
	  vm_cache_free(thread_wrapper_cache,wrapper);
	  return EXIT_SUCCESS;
	}

      /* Run through linked list */
      wrapper = LLIST_NEXT(proc->thread_list,wrapper);
    }while(!LLIST_ISHEAD(proc->thread_list,wrapper));
    

  return EXIT_FAILURE;
}



/**

   Function: u8_t proc_memcopy(struct proc* proc, virtaddr_t src, virtaddr_t dest, size_t len)
   -------------------------------------------------------------------------------------------

   Copy in-memory data at address `src` to `proc` adress space at address `dest`.
   `src` must reside in kernel space.

**/


PUBLIC u8_t proc_memcopy(struct proc* proc, virtaddr_t src, virtaddr_t dest, size_t len)
{
  virtaddr_t cur_addrspace;

  /* Sanity check */
  if ( (proc == NULL) || (src > X86_CONST_KERN_HIGHMEM) )
    {
      return EXIT_FAILURE;
    }

  /* Save current address space */
  cur_addrspace = arch_get_addrspace();

  /* Change address space if needed */
  if (proc->addrspace != cur_addrspace)
    {
      if (arch_switch_addrspace(proc->addrspace) != EXIT_SUCCESS)
	{
	  return EXIT_FAILURE;
	}
    }  

  /* Copy (will generate page faults !) */
  arch_memcopy(src,dest,len);

  /* Switch back to current address space if needed */
  if (proc->addrspace != cur_addrspace)
    {
      if (arch_switch_addrspace(cur_addrspace) != EXIT_SUCCESS)
	{
	  return EXIT_FAILURE;
	}
    }
  
  return EXIT_SUCCESS;
}
