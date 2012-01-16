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

#define THREAD_QUANTUM_DEFAULT       2
#define THREAD_NICE_DEFAULT          0

#define THREAD_NICE_TOP              19
#define THREAD_NICE_BOTTOM           -24

#define THREAD_ID_DEFAULT            0
#define THREAD_HASH_SIZE             2//1024


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


/* Donnees Ordonnanceur */

PUBLIC struct sched_info
{
  u8_t static_prio;
  u8_t dynamic_prio;
  s8_t head_prio;
  u8_t static_quantum;
  s8_t dynamic_quantum;
};


/* Donnees d identification */

PUBLIC struct threadID
{
  s32_t id;
  struct thread * thread;
  struct threadID* prev;
  struct threadID* next;
};


/* Structure thread */

PUBLIC struct thread
{
  char name[THREAD_NAMELEN];
  struct context_cpu* ctx;
  struct threadID* id;
  virtaddr_t stack_base;
  u32_t stack_size;
  enum thread_state state;
  enum thread_state next_state;
  s8_t nice;
  struct sched_info sched;
  struct thread* prev;
  struct thread* next;
};


/*========================================================================
 * Macros
 *========================================================================*/


#define THREAD_NICE2PRIO(__nice)			\
  ( THREAD_NICE_TOP - (__nice) )


/*========================================================================
 * Prototypes
 *========================================================================*/


PUBLIC struct threadID* thread_hashID[THREAD_HASH_SIZE];

PUBLIC void thread_init(void);
PUBLIC struct thread* thread_create(const char* name, s32_t id, virtaddr_t start_entry, void* start_arg, u32_t stack_size, s8_t nice_level, u8_t quantum);
PUBLIC u8_t thread_destroy(struct thread* th);

#endif



