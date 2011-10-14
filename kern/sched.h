/*
 * sched.h
 * Header de sched.c
 *
 */


#ifndef SCHED_H
#define SCHED_H


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "const.h"
#include "thread.h"


/*========================================================================
 * Prototypes
 *========================================================================*/


PUBLIC struct thread* sched_ready;
PUBLIC struct thread* sched_running;
PUBLIC struct thread* sched_blocked;
PUBLIC struct thread* sched_dead;


#endif
