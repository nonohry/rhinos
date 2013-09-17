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
   - arch_vm.h : arch dependant virtual memory 
   - thread.h  : struct thread needed

**/


#include <define.h>
#include <types.h>
#include <arch_vm.h>
#include "thread.h"


/**
 
   Constant: PROC_NAMELEN
   ----------------------

   Max size of a process name

**/

#define PROC_NAMELEN               32



/**

   Typedef: pid_t
   --------------

   Process identifier

**/


typedef unsigned int pid_t;



/**
   
   Structure: struct thread_wrapper
   --------------------------------

   Wrap a thread into a linked list
   Members are self explanatory

**/


struct thread_wrapper
{
  struct thread* thread;
  struct thread_wrapper* prev;
  struct thread_wrapper* next;
};



/**
 
   Structure: struct proc 
   ----------------------

   Describe a process from the kernel point of view.
   Members are:

   - addr_space   : address space
   - name         : process name
   - thread_list  : threads in process
   - prev,next    : linkage in proc table

**/

PUBLIC struct proc
{
  virtaddr_t addrspace; 
  char name[PROC_NAMELEN];
  struct thread_wrapper* thread_list;
  struct proc* prev;
  struct proc* next;
}__attribute__ ((packed));



/**
   
   Prototypes
   ----------

   Give access to process initialization, creation and thread addition/removal

**/

PUBLIC u8_t proc_setup(void);
PUBLIC struct proc* proc_create(char* name);
PUBLIC u8_t proc_destroy(struct proc* proc);
PUBLIC u8_t proc_add_thread(struct proc* proc, struct thread* th);
PUBLIC u8_t proc_remove_thread(struct proc* proc, struct thread* th);
PUBLIC u8_t proc_memcopy(struct proc* proc, virtaddr_t src, virtaddr_t dest, size_t len);

#endif
