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
   - thread.h        : struct thread needed
   - proc.h          : self header

**/


#include <define.h>
#include <types.h>
#include <llist.h>
#include "thread.h"
#include "proc.h"



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
