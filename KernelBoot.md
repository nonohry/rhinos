| **[GRUB](KernelGRUB.md)** |
|:--------------------------|

> ![http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png](http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png)

| | Memory model setup |
|:|:-------------------|
| **[Arch](KernelArch.md)** |    Interrupt mechanism setup |
| | Relocate servers after kernel |


> ![http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png](http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png)


| **[Pager0](KernelPager0.md)**| Default page fault handler |
|:|:---------------------------|
| | Physical memory manager    |

> ![http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png](http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png)

| **[Virtual Memory](KernelVMem.md)**| Virtual Memory Managers |
|:|:------------------------|

> ![http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png](http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png)

| **[Scheduler](KernelSched.md)** | Initialize queues |
|:--------------------------------|:------------------|

> ![http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png](http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png)


| | Convert boot modules into ring 3 threads |
|:|:-----------------------------------------|
| **[Threads](KernelThreads.md)** | Convert curent execution into kernel (ring 0) thread |
| | Update scheduler                         |

> ![http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png](http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png)


| | Put previously threads in a proc each |
|:|:--------------------------------------|
| **[Proc](KernelProc.md)** | Adapt each page directory             |
| | Map boot modules in corresponding address space |

> ![http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png](http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png)

| **[Clock](KernelClock.md)** | Init clock harware |
|:----------------------------|:-------------------|
|                             | Add flih & activate IRQ |

> ![http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png](http://upload.wikimedia.org/wikipedia/commons/e/e6/Darr.png)

| **Waiting for scheduler** |
|:--------------------------|