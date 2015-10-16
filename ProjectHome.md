## Introduction ##

RhinOS is a micro kernel created from scratch.

## Features ##

  * Boot via Grub2
  * Kernel output on serial port or bochs 0xe9 port
  * Protected mode
  * Hardware interrupt management
  * Physical memory management
  * Pagination
  * Virtual memory management
    * Slab Allocator
  * Thread and Proc management
    * User threads and Proc
    * Scheduling
  * IPC via message passing


## Under development ##

  * On demand paging
  * Elf loading for boot modules
  * Scheduler review
  * Kernel task