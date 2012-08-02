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

#define IPC_DATA      246


/*========================================================================
 * Structures
 *========================================================================*/


PUBLIC struct ipc_message
{
  s32_t from;
  u32_t len;
  u8_t  data[IPC_DATA];
} __attribute ((packed))__ ;


/*========================================================================
 * Prototypes
 *========================================================================*/

EXTERN u8_t ipc_send(int to, struct ipc_message* msg);
EXTERN u8_t ipc_receive(int from, struct ipc_message* msg);
EXTERN u8_t ipc_notify(int to);
EXTERN u8_t ipc_sendrec(int to, struct ipc_message* msg);

#endif
