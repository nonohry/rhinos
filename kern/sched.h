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
   - const.h
   - thread.h : struct thread needed

**/

#include <define.h>
#include <types.h>
#include "const.h"
#include "thread.h"


/**
   Constants: Scheduling relatives
   -------------------------------

**/



#define SCHED_RUNNING_QUEUE          1
#define SCHED_READY_QUEUE            2
#define SCHED_BLOCKED_QUEUE          3
#define SCHED_DEAD_QUEUE             4

#define SCHED_QUEUE_SIZE             (THREAD_NICE_TOP-THREAD_NICE_BOTTOM+1)
#define SCHED_PRIO_MIN               THREAD_NICE2PRIO(THREAD_NICE_TOP)
#define SCHED_PRIO_MAX               THREAD_NICE2PRIO(THREAD_NICE_BOTTOM)
#define SCHED_PRIO_RR                THREAD_NICE2PRIO(-21)

#define SCHED_DEFAULT                0
#define SCHED_FROM_PIT               1
#define SCHED_FROM_SEND              2
#define SCHED_FROM_RECEIVE           4


/**

   Prototypes
   ----------

   Give access to initialization, queue manipulation ans scheduling itself

**/

PUBLIC u8_t sched_init(void);
PUBLIC u8_t sched_enqueue(u8_t queue, struct thread* th);
PUBLIC u8_t sched_dequeue(u8_t queue, struct thread* th);
PUBLIC void sched_schedule(u8_t flag);
PUBLIC struct thread* sched_get_running_thread(void);

#endif
