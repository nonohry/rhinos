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
   - arch_vm.h : architecture dependant virtual memory
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
 
   Structure: struct proc 
   ----------------------

   Describe a process from the kernel point of view.
   Members are:

   - addr_space   : address space
   - name         : process name
   - thread_list  : threads in process


**/

PUBLIC struct proc
{
  addrspace_t addr_space; 
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
