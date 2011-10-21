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


/*========================================================================
 * Prototypes
 *========================================================================*/

void sched_init(void);
u8_t sched_enqueue(u8_t queue, struct thread* th);
u8_t sched_dequeue(u8_t queue, struct thread* th);
struct thread* sched_run(void);

#endif
