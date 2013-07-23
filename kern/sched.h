/**
   sched.h
   =======

   Scheduler header
 
 **/

#ifndef SCHED_H
#define SCHED_H


/**
 
   Includes
   --------

   - define.h
   - types.h
   - thread.h : struct thread needed

**/

#include <define.h>
#include <types.h>
#include "thread.h"


/**
   Constants: Scheduling relatives
   -------------------------------

**/

#define SCHED_RUNNING_QUEUE          1
#define SCHED_READY_QUEUE            2
#define SCHED_BLOCKED_QUEUE          3
#define SCHED_DEAD_QUEUE             4



/**

   Prototypes
   ----------

   Give access to initialization, queue manipulation ans scheduling itself

**/

PUBLIC u8_t sched_setup(void);
PUBLIC u8_t sched_enqueue(u8_t queue, struct thread* th);
PUBLIC u8_t sched_dequeue(u8_t queue, struct thread* th);
PUBLIC struct thread* sched_elect();

#endif
