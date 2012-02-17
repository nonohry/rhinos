#ifndef IPC_H
#define IPC_H


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>


/*========================================================================
 * Constantes
 *========================================================================*/

#define IPC_ANY    0

#define IPC_SUCCESS   0
#define IPC_FAILURE   1
#define IPC_DEADLOCK  2


/*========================================================================
 * Structures
 *========================================================================*/


PUBLIC struct ipc_message
{
  s32_t from;
  u32_t len;
  u32_t code;
} __attribute ((packed))__ ;


/*========================================================================
 * Prototypes
 *========================================================================*/

EXTERN void ipc_send(int to, struct ipc_message* msg);
EXTERN void ipc_receive(int from, struct ipc_message* msg);
EXTERN void ipc_notify(int to);

#endif
