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
 * Constantes
 *========================================================================*/


#define SCHED_RUNNING_QUEUE          1
#define SCHED_READY_QUEUE            2
#define SCHED_BLOCKED_QUEUE          3
#define SCHED_DEAD_QUEUE             4

#define SCHED_NICE                   19


/*========================================================================
 * Structures
 *========================================================================*/



/*========================================================================
 * Macros
 *========================================================================*/


#define SCHED_NICE2PRIO(__nice)			\
  ( SCHED_NICE - (__nice) )


/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC void sched_init(void);
PUBLIC u8_t sched_enqueue(u8_t queue, struct thread* th);
PUBLIC u8_t sched_dequeue(u8_t queue, struct thread* th);
PUBLIC void sched_run(void);
PUBLIC struct thread* sched_get_running_thread(void);

#endif
