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
   - arch_ctx.h : CPU context
   - sched.h    : scheduler
   - thread.h   : self header

**/


#include <define.h>
#include <types.h>
#include <llist.h>
#include <arch_io.h>
#include <arch_ctx.h>
#include "sched.h"
#include "thread.h"



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
  //th = (struct thread*)mem_alloc(sizeof(struct thread));
  if (th == NULL)
    {
      return NULL;
    }

  /* Clean it */
  arch_memset(0,(addr_t)th,sizeof(struct thread));

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

  return th;

 err:
  
  //mem_free(th);
  return NULL;
  
}


/**

   Function: u8_t thread_switch_to(thread* th)
   -------------------------------------------

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
