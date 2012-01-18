/*
 * Gestion des threads
 *
 */


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include <llist.h>
#include "const.h"
#include "klib.h"
#include "assert.h"
#include "virtmem_slab.h"
#include "virtmem.h"
#include "context_cpu.h"
#include "sched.h"
#include "thread.h"


/*========================================================================
 * Declaration Private
 *========================================================================*/


PRIVATE struct vmem_cache* thread_cache;
PRIVATE struct vmem_cache* threadID_cache;
PRIVATE s32_t thread_IDs;

PRIVATE void thread_exit(struct thread* th);

/*========================================================================
 * Initialisation
 *========================================================================*/


PUBLIC void thread_init(void)
{
  u16_t i;

  /* Alloue des caches */
  thread_cache = virtmem_cache_create("thread_cache",sizeof(struct thread),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  ASSERT_FATAL( thread_cache!=NULL );

  threadID_cache = virtmem_cache_create("threadID_cache",sizeof(struct threadID),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  ASSERT_FATAL( threadID_cache!=NULL );

  /* Nullifie la hashtable */
  for(i=0;i<THREAD_HASH_SIZE;i++)
    {
      LLIST_NULLIFY(thread_hashID[i]);
    }

  /* Initialise le compteur d ID global */
  thread_IDs = 1;

  return;
}


/*========================================================================
 * Creation d un thread
 *========================================================================*/


PUBLIC struct thread* thread_create(const char* name, s32_t id, virtaddr_t start_entry, void* start_arg, u32_t stack_size, s8_t nice_level, u8_t quantum)
{
  struct thread* th;
  struct threadID* thID;
  u8_t i;

  /* Controles */
  ASSERT_RETURN( (start_entry!=0)&&(stack_size>CTX_CPU_MIN_STACK) , NULL);
  ASSERT_RETURN( (nice_level<=THREAD_NICE_TOP)&&(nice_level>=THREAD_NICE_BOTTOM) , NULL);

  /* Allocation dans les cache */
  th = (struct thread*)virtmem_cache_alloc(thread_cache,VIRT_CACHE_DEFAULT);
  ASSERT_RETURN( th!=NULL , NULL);

  thID = (struct threadID*)virtmem_cache_alloc(threadID_cache,VIRT_CACHE_DEFAULT);
  if (thID == NULL)
    {
      goto err00;
    }
  
  /* Alloue la pile */
  th->stack_base = (virtaddr_t)virt_alloc(stack_size);
  if ((void*)th->stack_base == NULL)
    {
      goto err01;
    }

  /* Definit la taille de la pile */
  th->stack_size = stack_size;

  /* Alloue le contexte */
  th->ctx = context_cpu_create(start_entry,start_arg,(virtaddr_t)thread_exit,(void*)th,th->stack_base,stack_size);
  if (th->ctx == NULL)
    {
      goto err02;
    }
  
  /* Copie du nom */
  i=0;
  while( (name[i]!=0)&&(i<THREAD_NAMELEN-1) )
    {
      th->name[i] = name[i];
      i++;
    }
  th->name[i]=0;

  /* Etats */
  th->state = THREAD_READY;
  th->next_state = THREAD_READY;

  /* Nice Level */
  th->nice = nice_level;

  /* Priorite */
  th->sched.static_prio = THREAD_NICE2PRIO(nice_level);
  th->sched.dynamic_prio = th->sched.static_prio;
  th->sched.head_prio = th->sched.static_prio;

  /* Quantum */
  th->sched.static_quantum = quantum;
  th->sched.dynamic_quantum = quantum;

  /* Chainage */
  sched_enqueue(SCHED_READY_QUEUE,th);

  /* ID */
  if (id == THREAD_ID_DEFAULT)
    {
      thID->id = thread_IDs;
      thread_IDs++;
     }
  else
    {
      thID->id = id;
    }
  thID->thread = th;

  /* Back pointer ID */
  th->id = thID;

  /* Chainage ID */
  LLIST_ADD(thread_hashID[THREAD_HASHID_FUNC(thID->id)],thID);

  /* Retour */
  return th;

 err02:
  /* Libere la pile */
  virt_free((void*)th->stack_base);

 err01:
  /* Libere le threadID */
  virtmem_cache_free(threadID_cache,thID);

 err00:
  /* Libere le thread */
  virtmem_cache_free(thread_cache,th);

  /* retour */
  return NULL;

}


/*========================================================================
 * Destruction d un thread
 *========================================================================*/


PUBLIC u8_t thread_destroy(struct thread* th)
{
  u16_t i;
  struct threadID* thID;

  /* Controle */
  ASSERT_RETURN( th!=NULL , EXIT_FAILURE);

  /* Libere le threadID correspondant */
  i=THREAD_HASHID_FUNC(th->id->id);
  if (!LLIST_ISNULL(thread_hashID[i]))
    {
      thID = LLIST_GETHEAD(thread_hashID[i]);
      do
	{
	  if (thID->thread == th)
	    {
	      LLIST_REMOVE(thread_hashID[i],thID);
	      virtmem_cache_free(threadID_cache,thID);
	      break;
	    }
	  thID = LLIST_NEXT(thread_hashID[i],thID);
	}while(!LLIST_ISHEAD(thread_hashID[i],thID));
    }

  /* Libere simplement les parties allouees */
  return virt_free((void*)th->stack_base) 
    && context_cpu_destroy(th->ctx) 
    &&  virtmem_cache_free(thread_cache,th);

}


/*========================================================================
 * Sortie d un thread
 *========================================================================*/


PRIVATE void thread_exit(struct thread* th)
{
  /* Controle */
  ASSERT_RETURN_VOID( th!=NULL );

  /* Nouvel etat */
  th->next_state = THREAD_DEAD;

  /* DEBUG TEMPORAIRE: Attend une interruption ... */
  while(1){}

  return;
}
