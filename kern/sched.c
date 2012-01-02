/*
 * sched.c
 * Gestion de l ordonnancement
 *
 */


/*========================================================================
 * Includes
 *========================================================================*/


#include <types.h>
#include <llist.h>
#include "assert.h"
#include "const.h"
#include "thread.h"
#include "sched.h"


/*========================================================================
 * Declaration Private
 *========================================================================*/

PRIVATE struct thread* sched_ready[SCHED_QUEUE_SIZE];
PRIVATE struct thread* sched_running;
PRIVATE struct thread* sched_blocked;
PRIVATE struct thread* sched_dead;

PRIVATE void sched_enqueue_roundrobin(struct thread* th);
PRIVATE void sched_enqueue_staircase(struct thread* th);
PRIVATE u8_t sched_get_higher_prio_queue(void);


/*========================================================================
 * Initialisation
 *========================================================================*/


PUBLIC void sched_init(void)
{
  u8_t i;

  /* Initialisation des listes */
  for(i=0;i<SCHED_QUEUE_SIZE;i++)
    {
      LLIST_NULLIFY(sched_ready[i]);
    }
  LLIST_NULLIFY(sched_running);
  LLIST_NULLIFY(sched_blocked);
  LLIST_NULLIFY(sched_dead);

  return;
}


/*========================================================================
 * Ajout a une file
 *========================================================================*/


PUBLIC u8_t sched_enqueue(u8_t queue, struct thread* th)
{
  ASSERT_RETURN( th!=NULL, EXIT_FAILURE);

  switch(queue)
    {
    
    case SCHED_RUNNING_QUEUE:
      LLIST_ADD(sched_running, th);
      th->state = THREAD_RUNNING;
      break;

    case SCHED_READY_QUEUE:
      /* Enfile selon le niveau de priorite */
      if (th->sched.static_prio < SCHED_PRIO_RR)
	{
	  sched_enqueue_staircase(th);
	}
      else
	{
	  sched_enqueue_roundrobin(th);
	}
      th->state = THREAD_READY;
      break;

    case SCHED_BLOCKED_QUEUE:
      LLIST_ADD(sched_blocked, th);
      th->state = THREAD_BLOCKED;
      break;

    case SCHED_DEAD_QUEUE:
      LLIST_ADD(sched_dead, th);
      th->state = THREAD_DEAD;
      break;

    default:
      return EXIT_FAILURE;
      
    }

  return EXIT_SUCCESS;

}


/*========================================================================
 * Retrait d une file
 *========================================================================*/


PUBLIC u8_t sched_dequeue(u8_t queue, struct thread* th)
{
  ASSERT_RETURN( th!=NULL, EXIT_FAILURE);

  switch(queue)
    {
    
    case SCHED_RUNNING_QUEUE:
      LLIST_REMOVE(sched_running, th);
      break;

    case SCHED_READY_QUEUE:
      LLIST_REMOVE(sched_ready[th->sched.dynamic_prio], th);
      break;

    case SCHED_BLOCKED_QUEUE:
      LLIST_REMOVE(sched_blocked, th);
      break;

    case SCHED_DEAD_QUEUE:
      LLIST_REMOVE(sched_dead, th);
      break;

    default:
      return EXIT_FAILURE;
      
    }

  return EXIT_SUCCESS;

}


/*========================================================================
 * Ordonnancement 
 *========================================================================*/


PUBLIC void sched_run(u8_t flag)
{

  struct thread* cur_th;
  struct thread* new_th;
  u8_t high_prio;

  /* Controle */
  ASSERT_RETURN_VOID( !LLIST_ISNULL(sched_running) );

  /* Le thread courant */
  cur_th = sched_get_running_thread();
  ASSERT_RETURN_VOID( cur_th != NULL );

  /* Decremente son quantum dynamique si appel par le PIT */
  if (flag == SCHED_FROM_PIT)
    {
      cur_th->sched.dynamic_quantum--;
    }

  /* Trouve la file de plus haute priorite */
  high_prio = sched_get_higher_prio_queue();

  /* Avec cette condition, une tache de plus haute priorite est qd meme stoppee en fin de quantum  */
  if ( (cur_th->sched.dynamic_quantum<0) || (high_prio > cur_th->sched.dynamic_prio) || (cur_th->next_state != THREAD_READY) )
    {
      /* Enleve le thread courant de la file d execution */
      sched_dequeue(SCHED_RUNNING_QUEUE,cur_th);
      
      /* Ajoute le thread ou il faut en fonction du futur etat */
      if (cur_th->next_state == THREAD_READY)
	{
	  sched_enqueue(SCHED_READY_QUEUE,cur_th);
	}
      else if (cur_th->next_state == THREAD_BLOCKED)
	{
	  sched_enqueue(SCHED_BLOCKED_QUEUE,cur_th);
	}
      else
	{
	  sched_enqueue(SCHED_DEAD_QUEUE,cur_th);
	}
      
      /* Choisis un nouveau thread */
      new_th = LLIST_GETHEAD(sched_ready[high_prio]);
      ASSERT_RETURN_VOID( new_th!=NULL );
      
      /* Change le nouveau thread de queue */
      sched_dequeue(SCHED_READY_QUEUE,new_th);
      sched_enqueue(SCHED_RUNNING_QUEUE,new_th);
 
      /* Switch vers le nouveau contexte */
      context_cpu_switch_to(new_th->ctx); 

    }

  return;
}


/*========================================================================
 * Thread courant
 *========================================================================*/


PUBLIC struct thread* sched_get_running_thread(void)
{
  return LLIST_GETHEAD(sched_running);
}


/*========================================================================
 * Enfile dans une file de la ready queue en RoundRobin
 *========================================================================*/


PRIVATE void sched_enqueue_roundrobin(struct thread* th)
{
  if (th->sched.dynamic_quantum < 0)
    {
      /* Reajuste son quantum */
      th->sched.dynamic_quantum = th->sched.static_quantum;
    }

  /* Enfile en fin de queue */
  LLIST_ADD(sched_ready[th->sched.dynamic_prio], th);

  return;
}



/*========================================================================
 * Enfile dans une file de la ready queue en Staircase
 *========================================================================*/


PRIVATE void sched_enqueue_staircase(struct thread* th)
{
  if (th->sched.dynamic_quantum < 0)
    {
      /* Decremente la priorite dynamique */
      if (th->sched.dynamic_prio > 0)
	{
	  th->sched.dynamic_prio--;
	  /* Reaffecte un quantum */
	  th->sched.dynamic_quantum=th->sched.static_quantum;
	}
      else
	{
	  /* Ici, le thread est en fin de plus basse priorite, on decremente la priorite de tete */
	  th->sched.head_prio--;

	  if (th->sched.head_prio < 0)
	    {
	      /* Remet la priorite de tete a l originale en cas de fin de cycle */
	      th->sched.head_prio = th->sched.static_prio;
	    }

	  /* Ajuste le priorite dynamique sur la priorite de tete */
	  th->sched.dynamic_prio = th->sched.head_prio;
	  /* Ajuste le quantum en fonction de la priorite */
	  th->sched.dynamic_quantum = (th->sched.static_prio - th->sched.head_prio)*th->sched.static_quantum;

	}

    }

  /* Enfile en fin de queue */
  LLIST_ADD(sched_ready[th->sched.dynamic_prio], th);  

  return;
}




/*========================================================================
 * Trouve la file de la ready queue de plus haute priorite
 *========================================================================*/


PRIVATE u8_t sched_get_higher_prio_queue(void)
{
  u8_t i;

  /* Parcourt simplement les files de la ready queue */  
  for(i=SCHED_PRIO_MAX;(i!=0)&&(LLIST_ISNULL(sched_ready[i]));i--)
    {}

  return i;
}
