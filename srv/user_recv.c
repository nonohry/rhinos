/**

   user.c
   ======

   Test program that do a sendrec to a computing thread

**/


#include <define.h>
#include <types.h>
#include <ipc.h>

void mem_copy(addr_t src, addr_t dst, u32_t len);

struct calc_msg
{
  u8_t op_code;
  u16_t op_1;
  u16_t op_2;
  u32_t op_res;
} __attribute__((packed));




int main()
{
  struct ipc_message m;
  struct calc_msg cm;

  while(ipc_receive(IPC_ANY,&m)==IPC_SUCCESS)
    {
      mem_copy((addr_t)m.data,(addr_t)&cm,sizeof(struct calc_msg)); 
      
      switch(cm.op_code)
        {
        case 1:
          cm.op_res = cm.op_1 + cm.op_2;
          break;
        case 2:
          cm.op_res = cm.op_1 * cm.op_2;
          break;
        default:
          cm.op_res = 0;
        }
      mem_copy((addr_t)&cm,(addr_t)m.data,sizeof(struct calc_msg));
      ipc_notify(m.from);
      ipc_send(m.from,&m);
    }
  
  while(1){}
  return 0;
}




void mem_copy(addr_t src, addr_t dst, u32_t len)
{
  char* s = (char*)src;
  char* d = (char*)dst;

  while(len--)
    {
      *d++ = *s++;
    }

  return;      
}
