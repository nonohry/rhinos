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

PRIVATE void thread_exit(struct thread* th);

/*========================================================================
 * Initialisation
 *========================================================================*/


PUBLIC void thread_init(void)
{
  /* Alloue un cache */
  thread_cache = virtmem_cache_create("thread_cache",sizeof(struct thread),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  ASSERT_FATAL( thread_cache!=NULL );

  return;
}


/*========================================================================
 * Creation d un thread
 *========================================================================*/


PUBLIC struct thread* thread_create(const char* name, virtaddr_t start_entry, void* start_arg, u32_t stack_size, s8_t nice_level, u8_t quantum)
{
  struct thread* th;
  u8_t i;

  /* Controles */
  ASSERT_RETURN( (start_entry!=0)&&(stack_size>CTX_CPU_MIN_STACK) , NULL);

  /* Allocation dans le cache */
  th = (struct thread*)virtmem_cache_alloc(thread_cache,VIRT_CACHE_DEFAULT);
  ASSERT_RETURN( th!=NULL , NULL);
  
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

  /* Chainage */
  sched_enqueue(SCHED_READY_QUEUE,th);

  /* Nice Level */
  th->nice = nice_level;

  /* Priorite */
  th->sched.static_prio = SCHED_NICE2PRIO(nice_level);
  th->sched.dynamic_prio = th->sched.static_prio;

  /* Quantum */
  th->sched.static_quantum = quantum;
  th->sched.dynamic_quantum = quantum;

  /* Retour */
  return th;

 err02:
  /* Libere la pile */
  virt_free((void*)th->stack_base);

 err01:
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
  /* Controle */
  ASSERT_RETURN( th!=NULL , EXIT_FAILURE);

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
