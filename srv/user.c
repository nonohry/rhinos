/**********************************
 * Includes
 **********************************/

#include <types.h>
#include <ipc.h>


/**********************************
 * Prototypes
 **********************************/

void mem_copy(addr_t src, addr_t dst, u32_t len);


/**********************************
 * Structures
 **********************************/

struct calc_msg
{
  u8_t op_code;
  u16_t op_1;
  u16_t op_2;
  u32_t op_res;
} __attribute__((packed));


/**********************************
 * Main
 **********************************/

int main()
{
  int j;
  struct ipc_message m;
  struct calc_msg cm;

  m.len = sizeof(struct calc_msg);
  cm.op_code = 2;
  j=1;

  while(j)
    {
      cm.op_1 = j%10;
      cm.op_2 = j%100;
      mem_copy((addr_t)&cm,(addr_t)m.data,m.len);
      if (ipc_sendrec(3,&m)!=IPC_SUCCESS)
      	{
      	  break;
      	}
      mem_copy((addr_t)m.data,(addr_t)&cm,m.len);
      j++;
    }
  while(1){}
  return 0;
}


/**********************************
 * Dirty memcopy
 **********************************/

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
