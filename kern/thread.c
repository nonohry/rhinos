/**

   thread.c
   ========


   Threads management

**/


/**

   Includes
   --------

   - define.h
   - types.h
   - llist.h
   - arch_io.h
   - arch_const : architecture dependent constants
   - arch_ctx.h : CPU context
   - vm_slab.h  : slab allocator
   - sched.h    : scheduler
   - thread.h   : self header

**/


#include <define.h>
#include <types.h>
#include <llist.h>
#include <arch_io.h>
#include <arch_const.h>
#include <arch_ctx.h>
#include "vm_slab.h"
#include "sched.h"
#include "thread.h"


/**

   Global: thread_cache
   --------------------

   Cache for `struct thread` allocation

**/


struct vm_cache* thread_cache;


/**

   Global: ksetup_th
   -----------------

   Kernel setup execution flow, must be created before cache 
   as cache will generate page fault and page fault handler need a current thread

**/

struct thread ksetup_th;


/**

   Function: u8_t thread_setup(void)
   ---------------------------------

   Initialize threads subsystem.

   Create "manually" a thread for current execution flow as cache_create will generate a page fault.
   Create a cache for `struct thread` allocation.
   Create a thread for current execution flow.

**/


PUBLIC u8_t thread_setup(void)
{
 
  /* Needed fields for current execution thread */
  ksetup_th.ctx.ss = ARCH_STACK_SELECTOR;
  ksetup_th.name[0] = '[';
  ksetup_th.name[1] = 'k';
  ksetup_th.name[2] = 's';
  ksetup_th.name[3] = 'e';
  ksetup_th.name[4] = 't';
  ksetup_th.name[5] = 'u';
  ksetup_th.name[6] = 'p';
  ksetup_th.name[7] = ']';
  ksetup_th.name[8] = 0;

  
  /* Define it as current thread */
  ksetup_th.state = THREAD_READY;
  sched_enqueue(SCHED_READY_QUEUE,&ksetup_th);
  cur_th = &ksetup_th;

  /* Create cache for allocation */
  thread_cache = vm_cache_create("Thread_Cache",sizeof(struct thread));
  if (thread_cache == NULL)
    {
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;

}



/**

   Function: struct thread* thread_create(const char* name, virtaddr_t base, virtaddr_t stack_base, size_t stack_size)
   -------------------------------------------------------------------------------------------------------------------


**/


PUBLIC struct thread* thread_create(const char* name, virtaddr_t base, virtaddr_t stack_base, size_t stack_size)
{
  u8_t i;
  struct thread* th=NULL;

  /* Sanity checks */
  if (!((base)&&(stack_base)&&(stack_size)))
    {
      return NULL;
    }

  /* Allocate a thread structure */
  th = (struct thread*)vm_cache_alloc(thread_cache);
  if (th == NULL)
    {
      return NULL;
    }

  /* Clean it */
  arch_memset(0,(addr_t)th,sizeof(struct thread));

  /* Set a name if it does not exist */
  if (name == NULL)
    {
      name = "NONAME";
    }

  /* Name copy */
  i=0;
  while( (name[i]!=0)&&(i<THREAD_NAMELEN-1) )
    {
      th->name[i] = name[i];
      i++;
    }
  th->name[i]=0;

  /* Set stack  */
  th->stack_base = stack_base;
  th->stack_size = stack_size;

  /* Set up context */
  if (arch_ctx_setup((arch_ctx_t*)th,
		base,
		th->stack_base,
		th->stack_size) != EXIT_SUCCESS)
    {
      goto err;
    }

  /* Link in scheduler */
  th->state = THREAD_READY;
  sched_enqueue(SCHED_READY_QUEUE,th);

  /* Nullify `proc` back pointer */
  th->proc = NULL;

  return th;

 err:
  
  vm_cache_free(thread_cache,th);
  return NULL;
  
}



/**

   Function: u8_t thread_destroy(struct thread* th)
   ------------------------------------------------

   Destroy a thread
   
   Unlink from scheduler then return thread structure to cache

**/



PUBLIC  u8_t thread_destroy(struct thread* th)
{

  u8_t res;

  /* Sanity check */
  if (th == NULL)
    {
      return EXIT_FAILURE;
    }

  /* Remove from scheduler, according to `state` */
  switch(th->state)
    {

    case THREAD_READY:
      {
	res = sched_dequeue(SCHED_READY_QUEUE,th);
	break;
      }

    case THREAD_RUNNING:
      {
	/* Cannot remove current running thread */
	return EXIT_FAILURE;
	break;
      }

    case THREAD_BLOCKED:
    case THREAD_BLOCKED_SENDING:
      {
	res = sched_dequeue(SCHED_BLOCKED_QUEUE,th);
	break;
      }

    case THREAD_DEAD:
      {
	res = sched_dequeue(SCHED_DEAD_QUEUE,th);
	break;
      }

    default:
      {
	return EXIT_FAILURE;
	break;
      }

    }

  /* Return if dequeue has failed */
  if (res != EXIT_SUCCESS)
    {
      return EXIT_FAILURE;
    }

  /* Return to cache */
  res = vm_cache_free(thread_cache,th);
  
  return res;
}



/**

   Function: u8_t thread_switch_to(struct thread* th)
   --------------------------------------------------

   Switch current thread to `th`

**/


PUBLIC u8_t thread_switch_to(struct thread* th)
{
  if (th)
    {
      cur_th = th;
      return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;
}
