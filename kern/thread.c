/**

   thread.c
   ========

   Thread management

**/


/**

   Includes
   --------

   - types.h
   - llist.h
   - const.h
   - klib.h
   - tables.h        : tss needed
   - virtmem_slab.h  : cache manipulation needed
   - virtmem.h       : virt_alloc/free needed
   - sched.h         : scheduler queues manipulation
   - thread.h        : self header


**/

#include <types.h>
#include <llist.h>
#include "const.h"
#include "klib.h"
#include "tables.h"
#include "virtmem_slab.h"
#include "virtmem.h"
#include "sched.h"
#include "thread.h"


/**

   Privates
   --------

   thread & thread_info caches

**/


PRIVATE struct vmem_cache* thread_cache;
PRIVATE struct vmem_cache* id_info_cache;


/**

   Private: thread_IDs
   -------------------

   Thread ID counter

**/


PRIVATE s32_t thread_IDs;


/**

   Privates
   --------

   Thread trampoline and exit


**/

PRIVATE void thread_exit(struct thread* th);
PRIVATE void thread_cpu_trampoline(thread_cpu_func_t start_func, void* start_arg, thread_cpu_func_t exit_func, void* exit_arg);


/**

   Function: u8_t thread_init(void)
   --------------------------------

   Thread subsystem initialization.
   Create caches in slab allocator for thread and thread_info structures.
   Initialize threads table.
   Create a thread for current execution flow (kern_th) and set it as current thread.
   Set up TSS.

**/


PUBLIC u8_t thread_init(void)
{
  u16_t i;

  /* Allocate caches */
  thread_cache = virtmem_cache_create("thread_cache",sizeof(struct thread),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  if (thread_cache == NULL)
    {
      return EXIT_FAILURE;
    }
 
  id_info_cache = virtmem_cache_create("id_info_cache",sizeof(struct id_info),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  if (id_info_cache == NULL)
    {
      return EXIT_FAILURE;
    }

  /* Nullify hashtable (threads table) */
  for(i=0;i<THREAD_HASH_SIZE;i++)
    {
      LLIST_NULLIFY(thread_hashID[i]);
    }

  /* Create a thread to structure the current execution flow */
  kern_th = (struct thread*)virtmem_cache_alloc(thread_cache,VIRT_CACHE_DEFAULT);
  if ( kern_th == NULL )
    {
      return EXIT_FAILURE;
    }

  kern_th->cpu.ss = CONST_KERN_SS_SELECTOR;
  kern_th->name[0] = '[';
  kern_th->name[1] = 'K';
  kern_th->name[2] = 'e';
  kern_th->name[3] = 'r';
  kern_th->name[4] = 'n';
  kern_th->name[5] = ']';
  kern_th->name[6] = 0;

  /* Thread is set a running */
  kern_th->state = THREAD_RUNNING;
  kern_th->next_state = THREAD_DEAD;

  kern_th->nice = 0;

  kern_th->sched.static_prio = THREAD_NICE2PRIO(0);
  kern_th->sched.dynamic_prio = kern_th->sched.static_prio;
  kern_th->sched.head_prio = kern_th->sched.static_prio;

  /* Thread will not survive */
  kern_th->sched.static_quantum = 0;
  kern_th->sched.dynamic_quantum = 0;

  /* Set the thread as running in scheduler */
  sched_enqueue(SCHED_RUNNING_QUEUE,kern_th);

  /* set thread as current thread */
  cur_th = kern_th;

  /* Set TSS for futur scheduling  */
  tss.esp0 = (u32_t)kern_th;
  tss.ss0 = CONST_KERN_SS_SELECTOR;

  /* Initialize ID counter */
  thread_IDs = 1;

  return EXIT_SUCCESS;
}



/**

   Function:  struct thread* thread_create_kern(const char* name, s32_t id, virtaddr_t start_entry, void* start_arg, s8_t nice_level, u8_t quantum)
   ------------------------------------------------------------------------------------------------------------------------------------------------

   Create a kernel space thread.
   Set all necessary attributes according to parameters
   Set up cpu context and stack for execution.
   Link new thread with existing ones in scheduler and in a hash table.

   Return a pointer to the thread or NULL if creation fails

**/


PUBLIC struct thread* thread_create_kern(const char* name, s32_t id, virtaddr_t start_entry, void* start_arg, s8_t nice_level, u8_t quantum)
{
  struct thread* th;
  struct id_info* thID;
  u8_t i;

  /* Sanity checks */
  if ( (start_entry == 0)
       || (nice_level > THREAD_NICE_TOP)
       || (nice_level < THREAD_NICE_BOTTOM) )
    {
      return NULL;
    }

  /* Allocate a thread */
  th = (struct thread*)virtmem_cache_alloc(thread_cache,VIRT_CACHE_DEFAULT);
  if ( th == NULL )
    {
      return NULL;
    }

  /* Clean it */
  klib_mem_set(0,(addr_t)th,sizeof(struct thread));

 
  /* Allocate an id_info */
  thID = (struct id_info*)virtmem_cache_alloc(id_info_cache,VIRT_CACHE_DEFAULT);
  if (thID == NULL)
    {
      goto err00;
    }
  
  /* Allocate a stack */
  th->stack_base = (virtaddr_t)virt_alloc(CONST_PAGE_SIZE);
  if ((void*)th->stack_base == NULL)
    {
      goto err01;
    }
  
  /* Set stack size */
  th->stack_size = CONST_PAGE_SIZE;

  /* Cpu context initialization */
  if (thread_cpu_init((struct cpu_info*)th,start_entry,start_arg,(virtaddr_t)thread_exit,(void*)th,th->stack_base,th->stack_size,CONST_RING0) != EXIT_SUCCESS)
    {
      goto err02;
    }


  /* Name copy */
  i=0;
  while( (name[i]!=0)&&(i<THREAD_NAMELEN-1) )
    {
      th->name[i] = name[i];
      i++;
    }
  th->name[i]=0;

  /* Scheduler state */
  th->state = THREAD_READY;
  th->next_state = THREAD_READY;

  /* Nice Level */
  th->nice = nice_level;

  /* Priority */
  th->sched.static_prio = THREAD_NICE2PRIO(nice_level);
  th->sched.dynamic_prio = th->sched.static_prio;
  th->sched.head_prio = th->sched.static_prio;

  /* Quantum */
  th->sched.static_quantum = quantum;
  th->sched.dynamic_quantum = quantum;

  /* Scheduler enqueue */
  sched_enqueue(SCHED_READY_QUEUE,th);

  /* Id */
  if (id == THREAD_ID_DEFAULT)
    {
      thID->id = thread_IDs;
      thread_IDs++;
     }
  else
    {
      thID->id = id;
    }

  /* Back pointers */
  thID->thread = th;
  th->id = thID;

  /* Id_info linkage in hash table */
  LLIST_ADD(thread_hashID[THREAD_HASHID_FUNC(thID->id)],thID);

  /* IPC */
  th->ipc.send_to = NULL;
  th->ipc.send_message = NULL;
  th->ipc.receive_from = NULL;
  th->ipc.receive_message = NULL;
  LLIST_NULLIFY(th->ipc.receive_waitlist);

  /* Return */
  return th;

 err02:
  /* Free stack */
  virt_free((void*)th->stack_base);
  
 err01:
  /* Free id_info */
  virtmem_cache_free(id_info_cache,thID);

 err00:
  /* Free thread */
  virtmem_cache_free(thread_cache,th);

  /* Error return */
  return NULL;

}


/**

   Function: struct thread* thread_create_user(const char* name, s32_t id, virtaddr_t start_entry, virtaddr_t stack_base, u32_t stack_size, s8_t nice_level, u8_t quantum)
   -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

   Create an user space thread.
   Set all necessary attributes according to parameters
   Set up cpu context and stack for execution.
   Link new thread with existing ones in scheduler and  a hash table

   Function is similary to kernel one, main difference is that stack is not created in the function but passed as an argument.

   Return a pointer to the thread or NULL if creation fails

**/


PUBLIC struct thread* thread_create_user(const char* name, s32_t id, virtaddr_t start_entry, virtaddr_t stack_base, u32_t stack_size, s8_t nice_level, u8_t quantum)
{
  struct thread* th;
  struct id_info* thID;
  u8_t i;
  
  /* Sanity checks */
  if ( (start_entry == 0)
       || (nice_level > THREAD_NICE_TOP)
       || (nice_level < THREAD_NICE_BOTTOM) 
       || (stack_base == 0)
       || (stack_size == 0) )
    {
      return NULL;
    }

  /* Allocate thread */
  th = (struct thread*)virtmem_cache_alloc(thread_cache,VIRT_CACHE_DEFAULT);
  if ( th == NULL )
    {
      return NULL;
    }

  /* Clean it */
  klib_mem_set(0,(addr_t)th,sizeof(struct thread));


  /* Allocate id_info */
  thID = (struct id_info*)virtmem_cache_alloc(id_info_cache,VIRT_CACHE_DEFAULT);
  if (thID == NULL)
    {
      goto err00;
    }
  
  /* Set stack base address */
  th->stack_base = stack_base;

  /* Set stack size */
  th->stack_size = stack_size;

  /* Cpu context initialization */
  if (thread_cpu_init((struct cpu_info*)th,start_entry,NULL,(virtaddr_t)thread_exit,(void*)th,th->stack_base,th->stack_size,CONST_RING3) != EXIT_SUCCESS)
    {
      goto err01;
    }


  /* Name copy */
  i=0;
  while( (name[i]!=0)&&(i<THREAD_NAMELEN-1) )
    {
      th->name[i] = name[i];
      i++;
    }
  th->name[i]=0;

  /* Scheduler state */
  th->state = THREAD_READY;
  th->next_state = THREAD_READY;

  /* Nice Level */
  th->nice = nice_level;

  /* Priority */
  th->sched.static_prio = THREAD_NICE2PRIO(nice_level);
  th->sched.dynamic_prio = th->sched.static_prio;
  th->sched.head_prio = th->sched.static_prio;

  /* Quantum */
  th->sched.static_quantum = quantum;
  th->sched.dynamic_quantum = quantum;

  /* Scheduler enqueue */
  sched_enqueue(SCHED_READY_QUEUE,th);

  /* Id */
  if (id == THREAD_ID_DEFAULT)
    {
      thID->id = thread_IDs;
      thread_IDs++;
     }
  else
    {
      thID->id = id;
    }

  /* Back pointers */
  thID->thread = th;
  th->id = thID;

  /* id_info in hash table  */
  LLIST_ADD(thread_hashID[THREAD_HASHID_FUNC(thID->id)],thID);

  /* IPC */
  th->ipc.send_to = NULL;
  th->ipc.send_message = NULL;
  th->ipc.receive_from = NULL;
  th->ipc.receive_message = NULL;
  LLIST_NULLIFY(th->ipc.receive_waitlist);


  /* Return */
  return th;

 err01:
  /* Free id_info */
  virtmem_cache_free(id_info_cache,thID);

 err00:
  /* Free thread */
  virtmem_cache_free(thread_cache,th);

  /* Error return */
  return NULL;

}


/**

   Function: u8_t thread_destroy(struct thread* th)
   ------------------------------------------------

   Destroy thread `th`.

   Run through linked lists in hash table to free id_info structure.
   Then free all allocated spaces.

**/


PUBLIC u8_t thread_destroy(struct thread* th)
{
  u16_t i;
  struct id_info* thID;
  
  if ( th == NULL )
    {
      return EXIT_FAILURE;
    }
  
  /* Look for id_info */
  i=THREAD_HASHID_FUNC(th->id->id);
  if (!LLIST_ISNULL(thread_hashID[i]))
    {
      thID = LLIST_GETHEAD(thread_hashID[i]);
      do
	{
 	  if (thID->thread == th)
	    {
	      /* Found ! Free it */
	      LLIST_REMOVE(thread_hashID[i],thID);
	      virtmem_cache_free(id_info_cache,thID);
	      break;
	    }
	  thID = LLIST_NEXT(thread_hashID[i],thID);
	}while(!LLIST_ISHEAD(thread_hashID[i],thID));
    }

  /* Simply free allocated spaces (Problem for user thread becasue of stack release !) */
  return virt_free((void*)th->stack_base) 
    &&  virtmem_cache_free(thread_cache,th);

}


/**

   Function: u8_t thread_cpu_init(struct cpu_info* ctx, virtaddr_t start_entry, void* start_arg, virtaddr_t exit_entry, void* exit_arg, virtaddr_t stack_base, u32_t stack_size, u8_t ring)
   ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

   Initialize a new cpu context ready for execution.

   A user context does not support any argument for the moment.
   A kernel context can have several arguments, passed through a structure `start_arg`. A trampoline is used to pass arguments to entry point.

**/


PUBLIC u8_t thread_cpu_init(struct cpu_info* ctx, virtaddr_t start_entry, void* start_arg, virtaddr_t exit_entry, void* exit_arg, virtaddr_t stack_base, u32_t stack_size, u8_t ring)
{
  
  virtaddr_t* esp;
  
  /* Sanity checks */
  if ( (start_entry == 0)
       || (exit_entry == 0)
       || (stack_base == 0)
       || (stack_size == 0)
       || (ctx == NULL) )
    {
      return EXIT_FAILURE;
    }

  /* Rings */
  if ( (ring != CONST_RING0)&&(ring != CONST_RING3) )
    {
      return EXIT_FAILURE;
    }
  
  /* No arguments in ring 3 */
  if ( (ring==CONST_RING3)&&(start_arg!=NULL) )
    {
      return EXIT_FAILURE;
    }

  /* Clean stack */
  klib_mem_set(0,(addr_t)stack_base,stack_size);

  /* Get esp (for arguments) */
  esp = (virtaddr_t*)(stack_base+stack_size);

  /* Set up segment registers */
  ctx->cs = (ring == CONST_RING0 ? CONST_KERN_CS_SELECTOR : CONST_USER_CS_SELECTOR);
  ctx->ds = (ring == CONST_RING0 ? CONST_KERN_DS_SELECTOR : CONST_USER_DS_SELECTOR);
  ctx->es = (ring == CONST_RING0 ? CONST_KERN_ES_SELECTOR : CONST_USER_ES_SELECTOR);
  ctx->ss = (ring == CONST_RING0 ? CONST_KERN_SS_SELECTOR : CONST_USER_SS_SELECTOR);

  /* Fake error code */
  ctx->error_code = THREAD_CPU_FEC;

  /* Enable interrupts in EFLAGS */
  ctx->eflags = (1<<THREAD_CPU_INTFLAG_SHIFT);


  /* EIP points to entry point */
  ctx->eip = (reg32_t)start_entry;

  /* Trampoline in ring 0 */
  if (ring == CONST_RING0)
    {
      /* EIP points to trampoline */
      ctx->eip = (reg32_t)thread_cpu_trampoline;

      /* Push trampoline args */
      *(--esp) = (virtaddr_t)exit_arg;
      *(--esp) = exit_entry;
      *(--esp) = (virtaddr_t)start_arg;
      *(--esp) = start_entry;
  
      /* Fake return address for trampoline (no return) */
      *(--esp) = 0;
    
    
      /* Pretend to be an interrupted stask */
      *(--esp) = ctx->eflags;
      *(--esp) = CONST_KERN_CS_SELECTOR;
      *(--esp) = ctx->eip;
    
      *(--esp) = ctx->error_code;
      *(--esp) = 0;
    }
    

  /* Set up ESP */
  ctx->esp = (reg32_t)esp;


  return EXIT_SUCCESS;
}


/**

   Function: void thread_switch_to(struct thread* th)
   --------------------------------------------------

   Set `th` as the current thread. 
   If `th` belongs to a different process, we change address space loading a new page directory

**/


PUBLIC void thread_switch_to(struct thread* th)
{
  if (th != NULL)
    {
      /* Set `th` as current */
      cur_th = th;
      
      /* Change current process if needed */
      if (cur_proc != th->proc)
	{
	  cur_proc = th->proc;
	  /* Switch address space */
	  klib_load_CR3(cur_proc->p_pd);
	}

      /* Set tss according to new current thread */
      tss.esp0 = (u32_t)th+sizeof(struct cpu_info);

    }

  return;
}



/**

   Function: void thread_cpu_postsave(struct thread* th, reg32_t* esp)
   -------------------------------------------------------------------

   Finalize cpou context save in case of a kernel space switch.
   
   Simply retrieve register saved on stack by processor.

**/


PUBLIC void thread_cpu_postsave(struct thread* th, reg32_t* esp)
{


  /* Check 16 bits SS to determine ring */ 
  if ((th->cpu.ss & 0xFF) == CONST_KERN_SS_SELECTOR)
    {
      /* Retrieve remaining registers */
      cur_th->cpu.ret_addr = *(esp);
      cur_th->cpu.error_code = *(esp+1);
      cur_th->cpu.eip = *(esp+2);
      cur_th->cpu.cs = *(esp+3);
      cur_th->cpu.eflags = *(esp+4);
      cur_th->cpu.esp = (reg32_t)(esp);
      cur_th->cpu.ss = CONST_KERN_SS_SELECTOR;
    }
   
  return;
}


/**

   Function: void thread_cpu_trampoline(thread_cpu_func_t start_func, void* start_arg, thread_cpu_func_t exit_func, void* exit_arg)
   --------------------------------------------------------------------------------------------------------------------------------

   Trampoline for kernel thread.

   Pass `start_arg`  to `start_func`. When `start_func` returns (id est thread has terminated), `exit_func` is called
   with `exit_arg`.

**/


PRIVATE void thread_cpu_trampoline(thread_cpu_func_t start_func, void* start_arg, thread_cpu_func_t exit_func, void* exit_arg)
{
  /* Trampoline ! */
  start_func(start_arg);
  exit_func(exit_arg);

  /* In theory, there is no return */
  return;
}


/**

   Function: struct thread* thread_id2thread(s32_t n)
   --------------------------------------------------

   Get thread based on id `n`

   Return thread if found, NULL otherwise.
**/


PUBLIC struct thread* thread_id2thread(s32_t n)
{
  u16_t i;
  struct id_info* thID;

  if ( n == 0 )
    {
      return NULL;
    }

  /* Run through hash table */
  i=THREAD_HASHID_FUNC(n);
  if (!LLIST_ISNULL(thread_hashID[i]))
    {
      thID = LLIST_GETHEAD(thread_hashID[i]);
      do
	{
	  /* Id found, return thread */
	  if (thID->id == n)
	    {
	      return thID->thread;
	    }
	  thID = LLIST_NEXT(thread_hashID[i],thID);
	}while(!LLIST_ISHEAD(thread_hashID[i],thID));
    }
  
  /* Not found, return NULL */
  return NULL;
}


/**

   Function: void thread_exit(struct thread* th)
   ---------------------------------------------

   Handle thread end of life

**/


PRIVATE void thread_exit(struct thread* th)
{
  if ( th == NULL )
    {
      return;
    }

  /* Set state */
  th->next_state = THREAD_DEAD;

  /* Wait for an interrupt (TEMPORARY) */
  while(1){}

  return;
}
