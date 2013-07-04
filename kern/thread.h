/**

   thread.h
   ========

   Thread management header

**/


#ifndef THREAD_H
#define THREAD_H


/**
 
   Includes
   --------

   - types.h
   - ipc.h
   - const.h
   - proc.h  : struct proc needed

**/


#include <define.h>
#include <types.h>
#include <ipc.h>
#include "const.h"
#include "proc.h"


/**
   
   Constants: thread relatives
   ---------------------------

**/


#define THREAD_NAMELEN               32

#define THREAD_QUANTUM_DEFAULT       2
#define THREAD_NICE_DEFAULT          0

#define THREAD_NICE_TOP              19
#define THREAD_NICE_BOTTOM           -24

#define THREAD_ID_DEFAULT            0
#define THREAD_HASH_SIZE             1024

#define THREAD_CPU_INTFLAG_SHIFT     9
#define THREAD_CPU_FEC               0xFEC


/**
   
   Typedef: void (*thread_cpu_func_t)(void*)
   -----------------------------------------

   Type of a thread entry point
   
**/

typedef void (*thread_cpu_func_t)(void*);


/**
 
   Enum: enum thread_state
   -----------------------

   Define thread states as follow:

   0- THREAD_READY            : thread ready for execution
   1- THREAD_RUNNING          : thread is currently being executed
   2- THREAD_BLOCKED          : thread is blocked
   3- THREAD_BLOCKED_SENDING  : thread is blocked sending a message
   4- THREAD_DEAD             : thread is terminated


**/

PUBLIC enum thread_state
{
  THREAD_READY,
  THREAD_RUNNING,
  THREAD_BLOCKED,
  THREAD_BLOCKED_SENDING,
  THREAD_DEAD
};



/**

   Structure: struct sched_info
   ----------------------------

   Aggregate scheduler relatives. Member are:

   - static_prio     : defined priority
   - dynamic_prio    : moving priority
   - head_prio       : start priority in staircase scheduler
   - static_quantum  : defined quantum
   - dynamic_quantum : moving quantum

**/

PUBLIC struct sched_info
{
  u8_t static_prio;
  u8_t dynamic_prio;
  s8_t head_prio;
  u8_t static_quantum;
  s8_t dynamic_quantum;
};


/**

   Structure: struct id_info
   -------------------------

   Helper to associate a thread and an id.
   id_info structures are linked together in a hash table which forms the kernel process table

**/

PUBLIC struct id_info
{
  s32_t id;
  struct thread * thread;
  struct id_info* prev;
  struct id_info* next;
};


/**

   Structures: struct ipc_info
   ---------------------------

   Aggregate IPC relatives. Members are:

   - state                : IPC state (SENDING, RECEIVING, ...)
   - send_to              : thread to send to
   - send_message         : sent message virtual address
   - send_phys_message    : sent message physical address
   - receive_from         : thread to receive from
   - receive_message      : received message virtual address
   - receive_phys_message : received message physical address
   - receive_waitlist     : waitlist to link sending thread

**/

PUBLIC struct ipc_info
{
  u8_t state;
  struct thread* send_to;
  struct thread* receive_from;
  struct thread* receive_waitlist;
};


/**

   Structure: struct cpu_info
   --------------------------

   CPU Context.
   Registers order match push order during assembly context save

**/

PUBLIC struct cpu_info
{
  reg16_t gs;
  reg16_t fs;
  reg16_t es;
  reg16_t ds;
  reg32_t edi;
  reg32_t esi;
  reg32_t ebp;
  reg32_t orig_esp;
  reg32_t ebx;
  reg32_t edx;
  reg32_t ecx;
  reg32_t eax;
  reg32_t ret_addr;
  reg32_t error_code;
  reg32_t eip;
  reg32_t cs;
  reg32_t eflags;
  reg32_t esp;
  reg32_t ss;
} __attribute__ ((packed));


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
  struct cpu_info cpu;
  char name[THREAD_NAMELEN];
  struct id_info* id;
  virtaddr_t stack_base;
  u32_t stack_size;
  struct proc* proc;
  enum thread_state state;
  enum thread_state next_state;
  s8_t nice;
  struct sched_info sched;
  struct ipc_info ipc;
  struct thread* prev;
  struct thread* next;
}__attribute__ ((packed));


/**
 
   Macro: THREAD_NICE2PRIO(__nice)
   -------------------------------
   
   Convert a nice value to a scheduler priority value

**/


#define THREAD_NICE2PRIO(__nice)		\
  ( THREAD_NICE_TOP - (__nice) )



/**

   Macro: THREAD_HASHID_FUNC(__id)
   -------------------------------

   Hash function for thread id hash table

**/

#define THREAD_HASHID_FUNC(__id)		\
  ( (__id)%THREAD_HASH_SIZE )


/**

   Global: thread_hashID
   ---------------------

   Thread id hash table (thread tables)

**/

PUBLIC struct id_info* thread_hashID[THREAD_HASH_SIZE];

/**

   Globals: cur_th & cur_proc
   --------------------------

   Current thread and current process

**/
PUBLIC struct thread* cur_th;
PUBLIC struct proc* cur_proc;


/**

   Global: kern_th
   ---------------

   First execution flow

**/

PUBLIC struct thread* kern_th;


/**

   Prototypes
   ----------

   Give access to thread initialization, creation, destruction and helpers

**/

PUBLIC u8_t thread_init(void);
PUBLIC struct thread* thread_create_kern(const char* name, s32_t id, virtaddr_t start_entry, void* start_arg, s8_t nice_level, u8_t quantum);
PUBLIC struct thread* thread_create_user(const char* name, s32_t id, virtaddr_t start_entry, virtaddr_t stack_base, u32_t stack_size, s8_t nice_level, u8_t quantum);
PUBLIC u8_t thread_destroy(struct thread* th);
PUBLIC void thread_switch_to(struct thread* th);
PUBLIC u8_t thread_cpu_init(struct cpu_info* ctx, virtaddr_t start_entry, void* start_arg, virtaddr_t exit_entry, void* exit_arg, virtaddr_t stack_base, u32_t stack_size, u8_t ring);
PUBLIC void thread_cpu_postsave(struct thread* th, reg32_t* esp);
PUBLIC struct thread* thread_id2thread(s32_t n);

#endif



