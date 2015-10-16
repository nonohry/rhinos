There is 2 virtual memory manager (vmm) in kernel

The first will provide virtual memory as pages. To avoid interdependency with another vmm, this one has to be simple. A page stack could be a good choice. Implementation could look like this:

```
#define VM_SIZE       81920
#define VM_PAGE_SIZE  4096
#define VM_MASK       (VM_PAGE_SIZE-1)
#define STACK_SIZE    (VM_SIZE/VM_PAGE_SIZE)

#define VM_PAGE_OK    0
#define VM_PAGE_ERROR 1  /* Non-aligned value */

#define IS_ALIGNED(__addr) (!((__addr) & (VM_MASK)))


/* Stack (or Pool) */
u32_t stack[STACK_SIZE];
/* Stack top */
u32_t top;


u32_t vm_page_alloc()
{
	if ( (top+1) < STACK_SIZE )
	{
		return stack[top++];
	}
	
	return VM_PAGE_ERROR;
}

u32_t vm_page_free(u32_t addr)
{
	if ( (top) && (IS_ALIGNED(addr)) )
	{
		top--;
		stack[top] = addr;
		return VM_PAGE_OK;
	}
	
	return VM_PAGE_ERROR;
}
```


Kernel (virtual) space will be 256MB (which allows to manage more than 1 million processes). 256MB represents (for x86) 65536 pages. Storing all that pages in a stack cost 65536\*4 (size of address) = 262144 bytes that to say 64 pages.
Using a 16 bits counter (2<sup>16</sup>=65536) instead of address - multiply the counter by 4096 to retrieve page address, we can cut required pages down to 32.


The second is the kernel heap. It will implement kmalloc/kfree. A possible implementation can be a slab allocator. Because kernel will allocate only small object, we can implement on-slab mechanism only. When a cache need to grow, it will ask the first vmm for a page.