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
#include "thread.h"


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
  virtaddr_t v_pd;
  physaddr_t p_pd;
  struct thread_info* thread_list;
};



/*========================================================================
 * Prototypes
 *========================================================================*/


PUBLIC u8_t proc_init(void);


#endif
