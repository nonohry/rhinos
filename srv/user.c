/**

   user.c
   ======

   Test program that do a sendrec to a computing thread

**/


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
  int j;
  struct ipc_message m;
  struct calc_msg cm;

  cm.op_code = 2;
  j=1;

  while(j)
    {
      cm.op_1 = j%10;
      cm.op_2 = j%100;
      mem_copy((addr_t)&cm,(addr_t)m.data,sizeof(struct calc_msg));
      if (ipc_sendrec(3,&m)!=IPC_SUCCESS)
      	{
      	  break;
      	}
      //mem_copy((addr_t)m.data,(addr_t)&cm,sizeof(struct calc_msg));
      j++;
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
