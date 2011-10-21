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


/*========================================================================
 * Initialisation
 *========================================================================*/


void sched_init(void)
{
  LLIST_NULLIFY(sched_ready);
  LLIST_NULLIFY(sched_running);
  LLIST_NULLIFY(sched_blocked);
  LLIST_NULLIFY(sched_dead);

  return;
}


/*========================================================================
 * Ajout a une file
 *========================================================================*/


u8_t sched_enqueue(u8_t queue, struct thread* th)
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


u8_t sched_dequeue(u8_t queue, struct thread* th)
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


struct thread* sched_run(void)
{
  return LLIST_GETHEAD(sched_ready);
}
