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
#include <ipc.h>
#include "const.h"


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
#define THREAD_HASH_SIZE             1024

#define THREAD_CPU_MIN_STACK         64
#define THREAD_CPU_INTFLAG_SHIFT     9
#define THREAD_CPU_FEC               0xFEC


/*========================================================================
 * Structures
 *========================================================================*/


/* Etats */

PUBLIC enum thread_state
{
  THREAD_READY,
  THREAD_RUNNING,
  THREAD_BLOCKED,
  THREAD_BLOCKED_SENDING,
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

PUBLIC struct id_info
{
  s32_t id;
  struct thread * thread;
  struct id_info* prev;
  struct id_info* next;
};


/* Donnees IPC */

PUBLIC struct ipc_info
{
  u8_t state;
  struct thread* send_to;
  struct ipc_message* send_message;
  physaddr_t send_phys_message;
  struct thread* receive_from;
  struct ipc_message* receive_message;
  physaddr_t receive_phys_message;
  struct thread* receive_waitlist;
};


/* Contexte CPU */

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
  reg32_t ret_addr;   /* Adresse de retour empilee par les appels de fonctions */
  reg32_t error_code;
  reg32_t eip;
  reg32_t cs;
  reg32_t eflags;
  reg32_t esp;
  reg32_t ss;
} __attribute__ ((packed));


/* Structure thread */

PUBLIC struct thread
{
  struct cpu_info cpu;
  char name[THREAD_NAMELEN];
  struct id_info* id;
  virtaddr_t stack_base;
  u32_t stack_size;
  enum thread_state state;
  enum thread_state next_state;
  s8_t nice;
  struct sched_info sched;
  struct ipc_info ipc;
  struct thread* prev;
  struct thread* next;
}__attribute__ ((packed));


/*========================================================================
 * Macros
 *========================================================================*/


#define THREAD_NICE2PRIO(__nice)		\
  ( THREAD_NICE_TOP - (__nice) )


#define THREAD_HASHID_FUNC(__id)		\
  ( (__id)%THREAD_HASH_SIZE )


/*========================================================================
 * Prototypes
 *========================================================================*/


PUBLIC struct id_info* thread_hashID[THREAD_HASH_SIZE];
PUBLIC struct thread* cur_th;
PUBLIC struct thread* kern_th;

PUBLIC u8_t thread_init(void);
PUBLIC struct thread* thread_create(const char* name, s32_t id, virtaddr_t start_entry, void* start_arg, u32_t stack_size, s8_t nice_level, u8_t quantum);
PUBLIC u8_t thread_destroy(struct thread* th);
PUBLIC void thread_switch_to(struct thread* th);
PUBLIC u8_t thread_cpu_init(struct cpu_info* ctx, virtaddr_t start_entry, void* start_arg, virtaddr_t exit_entry, void* exit_arg, virtaddr_t stack_base, u32_t stack_size);
PUBLIC void thread_cpu_postsave(reg32_t ss, reg32_t* esp);
PUBLIC struct thread* thread_id2thread(s32_t n);

#endif



