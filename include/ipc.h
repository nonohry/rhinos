#ifndef IPC_H
#define IPC_H


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>


/*========================================================================
 * Structures
 *========================================================================*/


PUBLIC struct ipc_message
{
  u32_t len;
  void* data;
};


/*========================================================================
 * Prototypes
 *========================================================================*/

EXTERN void ipc_send(int to, struct ipc_message* msg);
EXTERN void ipc_receive(int from, struct ipc_message* msg);
EXTERN void ipc_notify(int to);

#endif
