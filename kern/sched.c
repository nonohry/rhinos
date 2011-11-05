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

PRIVATE struct thread* sched_ready;
PRIVATE struct thread* sched_running;
PRIVATE struct thread* sched_blocked;
PRIVATE struct thread* sched_dead;

PRIVATE u64_t sched_bitmap;

PRIVATE void sched_set_queue(u8_t qid);
PRIVATE void sched_unset_queue(u8_t qid);
PRIVATE u8_t sched_enqueue_readyq(struct thread* th);
PRIVATE u8_t sched_dequeue_readyq(struct thread* th);
PRIVATE u8_t sched_get_prio_queue(void);


/*========================================================================
 * Initialisation
 *========================================================================*/


PUBLIC void sched_init(void)
{
  /* Initialisation des listes */
  LLIST_NULLIFY(sched_ready);
  LLIST_NULLIFY(sched_running);
  LLIST_NULLIFY(sched_blocked);
  LLIST_NULLIFY(sched_dead);

  /* Bitmap */
  sched_bitmap = 0;

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
      break;

    case SCHED_READY_QUEUE:
      LLIST_ADD(sched_ready, th);
      break;

    case SCHED_BLOCKED_QUEUE:
      LLIST_ADD(sched_blocked, th);
      break;

    case SCHED_DEAD_QUEUE:
      LLIST_ADD(sched_dead, th);
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
      LLIST_REMOVE(sched_ready, th);
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


PUBLIC struct thread* sched_run(void)
{
  return LLIST_GETHEAD(sched_ready);
}


/*========================================================================
 * Thread courant
 *========================================================================*/


PUBLIC struct thread* sched_get_running_thread(void)
{
  return LLIST_GETHEAD(sched_running);
}


/*========================================================================
 * Ajoute a la ready queue
 *========================================================================*/

PRIVATE u8_t sched_enqueue_readyq(struct thread* th)
{
  return EXIT_SUCCESS;
}


/*========================================================================
 * Enleve de la ready queue
 *========================================================================*/


PRIVATE u8_t sched_dequeue_readyq(struct thread* th)
{
  return EXIT_SUCCESS;
}


/*========================================================================
 * Active/Desactive une queue
 *========================================================================*/


PRIVATE void sched_set_queue(u8_t qid)
{
  sched_bitmap |= (1<<qid);
  return;
}


PRIVATE void sched_unset_queue(u8_t qid)
{ 
  sched_bitmap &= ~(1<<qid);
  return;
}


/*========================================================================
 * Renvoie la queue de plus basse priorite
 *========================================================================*/


PRIVATE u8_t sched_get_prio_queue()
{
  return klib_lsb(sched_bitmap);
}
