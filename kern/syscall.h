/**

   syscall.h
   =========

   System calls header

**/

#ifndef SYSCALL_H
#define SYSCALL_H


/**

   Includes 
   --------

   - define.h
   - types.h
   - const.h

**/

#include <define.h>
#include <types.h>
#include "const.h"


/**
  
   Constants: Syscall numbers
   --------------------------
   
**/


#define SYSCALL_SEND        1
#define SYSCALL_RECEIVE     2
#define SYSCALL_NOTIFY      3


/**
  
   Constants: Syscall IPC states
   -----------------------------
   
**/


#define SYSCALL_IPC_SENDING    1
#define SYSCALL_IPC_RECEIVING  2
#define SYSCALL_IPC_NOTIFYING  3


/**

   Prototypes
   ----------

   Give access to the syscall handler

**/

PUBLIC void syscall_handle();

#endif
