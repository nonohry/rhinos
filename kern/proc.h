/*
 * proc.h
 * Header de proc.c
 *
 */


#ifndef PROC_H
#define PROC_H


/*========================================================================
 * Includes
 *========================================================================*/


#include <types.h>
#include "const.h"
#include "paging.h"
#include "thread.h"


/*========================================================================
 * Constantes
 *========================================================================*/


#define PROC_NAMELEN               32


/*========================================================================
 * Structures
 *========================================================================*/


/* Helper pour le chainage des threads */

PUBLIC struct thread_info
{
  struct thread* thread;
  struct thread_info* prev;
  struct thread_info* next;
};


/* Structure proc */

PUBLIC struct proc
{
  physaddr_t p_pd;
  struct pde* v_pd;
  char name[PROC_NAMELEN];
  struct thread_info* thread_list;
}__attribute__ ((packed));



/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC struct proc* cur_proc;

PUBLIC u8_t proc_init(void);
PUBLIC u8_t proc_create(char* name);
PUBLIC u8_t proc_add_thread(struct proc* proc, struct thread* th);
PUBLIC u8_t proc_remove_thread(struct proc* proc, struct thread* th);


#endif
