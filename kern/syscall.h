#ifndef SYSCALL_H
#define SYSCALL_H


/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "const.h"


/*========================================================================
 * Constantes
 *========================================================================*/


#define SYSCALL_SEND        1
#define SYSCALL_RECEIVE     2
#define SYSCALL_NOTIFY      3
#define SYSCALL_SENDREC     4

#define SYSCALL_IPC_SENDING    1
#define SYSCALL_IPC_RECEIVING  2
#define SYSCALL_IPC_NOTIFYING  4


/*========================================================================
 * Prototypes 
 *========================================================================*/

PUBLIC u8_t syscall_handle();

#endif
