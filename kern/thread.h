/**

   thread.h
   ========

   Threads management header

**/

#ifndef THREAD_H
#define THREAD_H

/**

   Includes
   --------

   - define.h
   - types.h
   - llist.h
   - arch_ctx.h : CPU context 
   - proc.h     : struct proc needed

**/


#include <define.h>
#include <types.h>
#include <llist.h>
#include <arch_ctx.h>
#include "proc.h"


/**
   
   Constants: thread relatives
   ---------------------------

**/


#define THREAD_NAMELEN               32


/**
 
   Enum: enum state
   -----------------

   Define thread states as follow:

   0- THREAD_READY            : thread ready for execution
   1- THREAD_RUNNING          : thread is currently being executed
   2- THREAD_BLOCKED          : thread is blocked
   3- THREAD_BLOCKED_SENDING  : thread is blocked sending a message
   4- THREAD_DEAD             : thread is terminated


**/

PUBLIC enum state
{
  THREAD_READY,
  THREAD_RUNNING,
  THREAD_BLOCKED,
  THREAD_BLOCKED_SENDING,
  THREAD_DEAD
};



/**

   Structure: struct sched
   -----------------------

   Aggregate scheduler relatives. Member are:

   - static_prio     : defined priority
   - dynamic_prio    : moving priority
   - head_prio       : start priority in staircase scheduler
   - static_quantum  : defined quantum
   - dynamic_quantum : moving quantum

**/

PUBLIC struct sched
{
  u8_t static_prio;
  u8_t dynamic_prio;
  s8_t head_prio;
  u8_t static_quantum;
  s8_t dynamic_quantum;
};


/**

   Structure: struct thread
   ------------------------

   Describe a thread. Members are:

  - cpu        : cpu context. Placed at the beginning to correspond to thread address (for ease of push)
  - name       : thread name
  - id         : thread id (via an id_info structure)
  - stack_base : stack base virtual address 
  - stack_size : stack size
  - proc       : "parent" process
  - state      : scheduling state
  - next_state : future state for scheduler decision
  - nice       : nice level (priority)
  - sched      : scheduler info
  - ipc        : IPC info
  - prev       : previous thread in linked list
  - next       : next thread in linked list

**/

PUBLIC struct thread
{
  arch_ctx_t ctx;
  char name[THREAD_NAMELEN];
  //  struct id_info* id;
  virtaddr_t stack_base;
  size_t stack_size;
  struct proc* proc;
  enum state state;
  //enum state next_state;
  //s8_t nice;
  //struct sched sched;
  //struct ipc_info ipc;
  struct thread* prev;
  struct thread* next;
}__attribute__ ((packed));




/** 

    Global: cur_th
    --------------

    Current running thread

**/

PUBLIC struct thread* cur_th;


/**

   Prototypes
   ----------

   Give access to thread setup, creation and switch.

**/


PUBLIC u8_t thread_setup(void);
PUBLIC struct thread* thread_create(const char* name, virtaddr_t base, virtaddr_t stack_base, size_t stack_size);
PUBLIC u8_t thread_destroy(struct thread* th);
PUBLIC u8_t thread_switch_to(struct thread* th);

#endif
