/*
 * thread.h
 * Header de thread.c
 *
 */


#ifndef THREAD_H
#define THREAD_H


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "const.h"
#include "context_cpu.h"


/*========================================================================
 * Constantes
 *========================================================================*/


#define THREAD_NAMELEN               32
#define THREAD_STACK_SIZE            4096

#define THREAD_SWITCH_DEFAULT        0
#define THREAD_SWITCH_NO_INT         1
   
#define THREAD_PRIO_NUM              64  
#define THREAD_NORMAL_PRIO_MAX       63
#define THREAD_NORMAL_PRIO_MIN       23
#define THREAD_RT_PRIO_MAX           22
#define THREAD_RT_PRIO_MIN           0 

#define THREAD_RT_AMPLITUDE          11
#define THREAD_NORMAL_AMPLITUDE      20

#define THREAD_NORMAL_PRIO_DEFAULT   0
#define THREAD_RT_PRIO_DEFAULT       0
#define THREAD_QUANTUM_DEFAULT       20


/*========================================================================
 * Structures
 *========================================================================*/


/* Etats */

PUBLIC enum thread_state
{
  THREAD_READY,
  THREAD_RUNNING,
  THREAD_BLOCKED,
  THREAD_DEAD
};


/* Structure thread */

PUBLIC struct thread
{
  char name[THREAD_NAMELEN];
  struct context_cpu* ctx;
  virtaddr_t stack_base;
  u32_t stack_size;
  enum thread_state state;
  u8_t static_prio;
  u8_t dynamic_prio;
  u8_t static_quantum;
  u8_t dynamic_quantum;
  struct thread* prev;
  struct thread* next;
};



/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC void thread_init(void);
PUBLIC struct thread* thread_create(const char* nom, virtaddr_t start_entry, void* start_arg, u32_t stack_size, char prio, u8_t quantum);
PUBLIC u8_t thread_destroy(struct thread* th);
PUBLIC u8_t thread_switch(struct thread* th, enum thread_state switch_state, u8_t flags);

#endif



