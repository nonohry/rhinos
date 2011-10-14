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
#include "const.h"
#include "thread.h"
#include "sched.h"


/*========================================================================
 * Initilisation
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
 * Ordonnancement 
 *========================================================================*/


struct thread* sched_run(void)
{
  return LLIST_GETHEAD(sched_ready);
}
