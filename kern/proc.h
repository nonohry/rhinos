/**
   
   proc.h
   ======

   Process management header

**/

#ifndef PROC_H
#define PROC_H


/**
 
   Includes
   --------

   - define.h
   - types.h
   - const.h
   - thread.h  : struct thread needed

**/


#include <define.h>
#include <types.h>
#include "const.h"
#include "thread.h"


/**
 
   Constant: PROC_NAMELEN
   ----------------------

   Max size of a process name

**/

#define PROC_NAMELEN               32


/**

   Structure: struct thread_info
   -----------------------------

   Helper structure to double link threads in process structure 
   whereas a thread is already a double linked structure.
   Members are:

   - thread  : thread to link to process
   - prev    : previous item in linked list
   - next    : next item in linked list
   
**/


PUBLIC struct thread_info
{
  struct thread* thread;
  struct thread_info* prev;
  struct thread_info* next;
};


/**
 
   Structure: struct proc 
   ----------------------

   Describe a process from the kernel point of view.
   Members are:

   - p_pd         : physical address of process page directory 
   - v_pd;        : virtual address of process page directory 
   - name         : process name
   - thread_list  : threads in process


**/

PUBLIC struct proc
{
  physaddr_t p_pd;
  struct pde* v_pd;
  char name[PROC_NAMELEN];
  struct thread_info* thread_list;
}__attribute__ ((packed));



/**
   
   Prototypes
   ----------

   Give access to process initialization, creation and thread addition/removal

**/

PUBLIC u8_t proc_init(void);
PUBLIC struct proc* proc_create(char* name);
PUBLIC u8_t proc_add_thread(struct proc* proc, struct thread* th);
PUBLIC u8_t proc_remove_thread(struct proc* proc, struct thread* th);


#endif
