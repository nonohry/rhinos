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


PUBLIC struct thread* thread_create(const char* name, virtaddr_t start_entry, void* start_arg, u32_t stack_size, char prio, u8_t quantum)
{
  struct thread* th;
  u8_t i;

  /* Controles */
  ASSERT_RETURN( (start_entry!=0)&&(stack_size>CTX_CPU_MIN_STACK) , NULL);
  ASSERT_RETURN( (prio<=THREAD_RT_AMPLITUDE)&&(prio>=(-THREAD_RT_AMPLITUDE)) ,NULL)


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

  /* Etat */
  th->state = THREAD_READY;

  /* Chainage */
  sched_enqueue(SCHED_READY_QUEUE,th);

  /* Priorite */
  th->static_prio = THREAD_RT_PRIO_MIN + ( (THREAD_RT_PRIO_MAX - THREAD_RT_PRIO_MIN)/2 ) + prio;
  th->dynamic_prio = th->static_prio;

  /* Quantum */
  th->static_quantum = 0;
  th->dynamic_quantum = th->static_quantum;

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
 * Switch d un thread
 *========================================================================*/


PUBLIC u8_t thread_switch(struct thread* th, enum  thread_state switch_state, u8_t flags)
{
  struct thread* new_th;

  /* Recuperation du contexte courant */
  ASSERT_RETURN( th!=NULL , EXIT_FAILURE);

  /* Enleve le thread courant de la file d execution */
  sched_dequeue(SCHED_RUNNING_QUEUE,th);

  /* Ajoute le threrad ou il faut en fonction du futur etat */
  if (switch_state == THREAD_READY)
    {
      sched_enqueue(SCHED_READY_QUEUE,th);
    }
  else if (switch_state == THREAD_BLOCKED)
    {
      sched_enqueue(SCHED_BLOCKED_QUEUE,th);
    }
  else
    {
      thread_exit(th);
    }

  /* Maj de l etat */
  th->state = switch_state;

  /* Choix du futur thread */
  new_th = sched_run();
  ASSERT_RETURN( new_th!=NULL , EXIT_FAILURE);

  /* Change le nouveau thread de queue */
  sched_dequeue(SCHED_READY_QUEUE,new_th);
  sched_enqueue(SCHED_RUNNING_QUEUE,new_th);

  /* Switch vers le nouveau contexte */
  if (flags == THREAD_SWITCH_NO_INT)
    {
      context_cpu_handle_switch_to(new_th->ctx); 
    }
  else
    {
      context_cpu_switch_to(new_th->ctx);
    } 


  return EXIT_SUCCESS;
}


/*========================================================================
 * Sortie d un thread
 *========================================================================*/


PRIVATE void thread_exit(struct thread* th)
{
  struct thread* new_th;

  /* Controle */
  ASSERT_RETURN_VOID( th!=NULL );

  /* Choix du futur thread */
  new_th = sched_run();
  ASSERT_RETURN_VOID( new_th!=NULL );

 /* Enleve le thread courant de la file d execution */
  sched_dequeue(SCHED_RUNNING_QUEUE,th);

  /* Ajoute le thread dans la queue adequate */
  sched_enqueue(SCHED_DEAD_QUEUE,th);
 
  /* Change le nouveau thread de queue */
  sched_dequeue(SCHED_READY_QUEUE,new_th);
  sched_enqueue(SCHED_RUNNING_QUEUE,new_th);

  /* Switch vers le nouveau contexte */
  context_cpu_exit_to(new_th->ctx);

  return;
}
