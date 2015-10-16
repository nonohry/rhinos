# x86 #

## Virtual Memory ##

All x86 virtual memory mechanisms are described in Intel manuals found [here](http://www.intel.com/products/processor/manuals).
Basically, x86 provide 2 virtual memory mechanisms:
  1. Segmentation : Mandatory to work in 32 bits protected mode
  1. Paging : Optionnal


### Segmentation ###


**1- Description**

Segmentation partitions memory into fixed size segments. Segmentation mechanism "big picture" would look like this:

```

         - LOGICAL ADDRESS -

15             0      31              0
+--------------+      +---------------+
| Seg Selector |      |     Offset    |
+--------------+      +---------------+
|                         |
|                         |
|     +------------+      |
|     |            |      |
|     |------------|      |
+---->| Segment    |     / \
      | Descriptor |----| + |
      |------------|     \ /
      |            |      |
      |            |      |
      |            |      |
      +------------+      |
         GDT/LDT          |
                          |
                          |
                31        |             0
                +-----------------------+
                |     LINEAR ADDRESS    |
                +-----------------------+
```

In segmentation, adresses, refered as logical addresses, are defined by a 2-uplet (_Segment Selector_, _Offset_). _Offset_ is in fact the manipulated address. The _segment selector_ is stored in segment registers CS, DS, ES, FS, GS and SS. Its structure is:

```

15                        3 2 1     0
+-------------------------+---+-----+
|                         | T |  R  |
|         Index           | I |  P  |
|                         |   |  L  |
+-------------------------+---+-----+

```

(Refer to intel manuals for fields explanation)
`Index` field is used as an index into a table of _segment descriptors_. There are 2 kinds of tables:
  * Global Descriptors Table (GDT) : Table global to the system
  * Local Desriptors Table (LDT): Table linked to a particular process

These tables contains _segment descriptors_ wich, as the name suggests, describes a memory segment. Here is their structure:

```

31                24 23  22  21  20 19     16 15 14  13 12 11     8 7             0
+------------------+---+---+---+---+--------+---+-----+---+--------+--------------+
|                  |   | D |   | A | Seg    |   |  D  |   |        |              |
|   Base 31:24     | G | / | 0 | V | Limit  | P |  P  | S |  Type  | Base 23:16   |  (4->7 B)
|                  |   | B |   | L | 19:16  |   |  L  |   |        |              |
+------------------+---+---+---+---+--------+---+-----+---+--------+--------------+


31                                        16 15                                   0
+-------------------------------------------+-------------------------------------+
|                                           |                                     |
|           Base Address 15:0               |       Segment Limit 15:0            |  (0->3 B)
|                                           |                                     |
+-------------------------------------------+-------------------------------------+

```


Refer to Intel manuals to explain each fields. Such a descriptor points to a `Base Address` which is in fact the first byte of the memory segment. We use now the _Offset_ starting from that `Base Address` to reach the desired address. If the _Offset_ exceed the _Segment Limit_, a processor exception is raised. That final address is called _Linear Address_. If paging is not activated, linear address space is equivalent to physical address space.


**2- Implementation**


Segmentation will not be used as it should ne in RhinOS. We setup a flat memory model where only 4 segments exists:
  1. Kernel Code segment
  1. Kernel Data segment
  1. User Code segment
  1. User Data segment

These segments are overlapping each other, starting from byte 0 to 2<sup>32</sup>. The only difference is the privilege level (RPL and DPL fields in previous structures) which is set to 0 for kernel segments and 3 for user segments. All other memory isolation mechanisms will be implemented through paging. As a result, there is no LDT in RhinOS.


### Paging ###


**1- Description**


With paging, x86 virtual memory "big picture" become:

```
 Logical Address
+-------+   +------+
|Seg Sel|   |Offset|
+-------+   +------+
|           |
|   +----+  |
|   |    |  |  +------+       Linear  Address
|   |----|  |  |------|      +----+-------+-------+
+-->|Desc|--+->|Linear|----> |Dir | Table | Offset|
    |----|     |------|      +----+-------+-------+
    |    |     |      |       |     |           |
    |    |     +------+    +-----+  |  +-----+  |
    +----+      Linear     |     |  |  |     |  |  +--------+
      GDT       Space      |-----|  |  |-----|  |  |--------|
                           |Entry|--+->|Entry|--+->|Phys Add|
                           |-----|     |-----|     |--------|
                           +-----+     +-----+     |        |
                            Page        Page       |        |
                            Dir         Table      |        |
                                                   |        |
                                                   |        |
                                                   +--------+
                                                    Physical
                                                    Space


|--------------|--------------------------------------------|
  Segmentation                     Paging
```

With paging, linear address is cut into 3 parts:
  1. Fisrt 10 bits (most significant bits) : thet are interpreted as an index in a table called Page Directory. Each entry of a page directory describes another table called Page Table.
  1. Next 10 bits : they are interpreted as index in the Page Table pointed to by the page directory entry. Each page table entry describes a 4096 bytes physical memory chunk.
  1. Last 12 bits : they are interpreted as an offset in the physical frame pointed to by the page table entry.

Next figure illustrates that mechanism:

```
                     31    22 21  12 11        0
                      +------+------+----------+
                      | Dir  | Tbl  | Offset   |
                      +------+------+----------+
                        |       |            |
                        |       |            |        +--------+        
    +-------------------+    +--+            |        |        |
    |                        |               |        +--------+                
    |                        |   +--------+  +------> | Addr   |                
    |                        |   |        |           +--------+                
    |                        |   +--------+           |        |                
    |     +--------+         |   |        |           |        |                
    |     |        |         |   +--------+           |        |                
    |     +--------+         |   |        |           |        |                
    |     |        |         |   +--------+           |        |                
    |     +--------+         +-> | PTE    | --------> +--------+                
    |     |        |             +--------+                     
    |     +--------+             |        |           Physical Page
    +---> | PDE    | ----------> +--------+                
          +--------+            
          |        |             Page Table
          +--------+
           
        Page Directory
```

With such a method, we can address 2<sup>10</sup> x 2<sup>10</sup> x 2<sup>12</sup> = 4GB memory.


_a- Page Directory_


Page directory is a 1024 entries table stored in CR3 register via its physical address. Changing page directory in CR3 implies an address space switch. Entries structure:


```
  31                      12 11 9 8 7 6 5 4 3 2 1 0   
   +------------------------+----+-+-+-+-+-+-+-+-+-+            
   |                        | A  | | | | |P|P|U|R| |  
   | Page Table Base Addr   | V  |G|P|0|A|C|W|/|/|P|   
   |                        | L  | |S| | |D|T|S|W| |    
   +------------------------+----+-+-+-+-+-+-+-+-+-+   
```

(Refer to Intel manuals for full explanation) `Page Table base Addr` is the physical address of an page table.


_b- Page Table_


Page table is also a 1024 entries table. Entries have the following structures:

```
  31                      12 11 9 8 7 6 5 4 3 2 1 0   
   +------------------------+----+-+-+-+-+-+-+-+-+-+            
   |                        | A  | | | | |P|P|U|R| |  
   | Phys. Page Base Addr   | V  |G|0|D|A|C|W|/|/|P|   
   |                        | L  | | | | |D|T|S|W| |    
   +------------------------+----+-+-+-+-+-+-+-+-+-+   
```

`Phys. Page Base Addr` field refers to the physical address of a 4096 bytes physical memory chunk.


_c- Translation Lookaside Buffer (TLB)_


In order to speed up conversion from virtual address to physical address, processor maintains a cache of virtual-physical combination called Translation Lookaside Buffer (TLB). Keep in mind to clean this cache when doing virtual/physical mapping. There are 2 ways to clean TLB:
  1. Reload CR3: it flushes all TLB
  1. Use `invlpg` instruction : Only flush the provided virtual address


**2- Implementation**


To use paging, we have set up a page directory for kernel and  identity map in-used physical memory with virtual memory. By doing this, addresses used by kernel will always point to the same data or code after activating virtual memory.

At the kernel level, we only need 3 operations: initilization, mapping and unmapping.


_a- Initilization_


Initilization consists in creating the kernel page directory, loading it in CR3, identity map in-used physical memory then setting PG bit in CR0.

Page directory is an array of 1024 4 bytes (32 bits) entries. We need a physical 4096 bytes frame to create it. At that time, memory looks like that:

```

+-------------+
|             |
.             .
.             .
|             |
|             |
|             |
|             |
|             |
|             |
|             |
+-------------+
|             |
|             |
|    BOOT     |
|   MODULES   |
|             |
|             |
|             |
|             |
+-------------+
|             |
|             |
|   KERNEL    |
|             |
|             |
+-------------+ 0x1000
|    ROM      |
|             |
+-------------+
|             |
|             |
|             |
|             |
|             |
|             |
+-------------+ 0x0

```


We can use immediate memory after relocated boot modules to create the kernel page directory. Also, kernel space will be shared among all processes. To simplify synchronization between kernel space and kernel part of processes adress space, we preallocate kernel page tables. As a result, copying the kernel page directory entries into processes page directory at processes creation time will suffice to synchronize their adress space. That preallocation requires 4096 bytes physical frame per kernel page table. We pick up that frames right after kernel page directory frame. After kernel page directory creation, memory will look like that:

```

+-------------+
|             |
.             .
.             .
|             |
+-------------+
|  Kern PT03  | 4KB
+-------------+
|  Kern PT02  | 4KB
+-------------+
|  Kern PT01  | 4KB
+-------------+
|  Kern PD    | 4KB
+-------------+
|             |
|             |
|    BOOT     |
|   MODULES   |
|             |
|             |
|             |
|             |
+-------------+
|             |
|             |
|   KERNEL    |
|             |
|             |
+-------------+ 0x1000
|    ROM      |
|             |
+-------------+
|             |
|             |
|             |
|             |
|             |
|             |
+-------------+ 0x0

```

Once kernel page directory is created, we need to create an identity mapping between virtual memory and in-used physical memory. It suffices to create virtual addresses in kernel page directory (and page tables) that are identical to physical addresses used by kernel and to make them  point to their physical counterpart. Now, we can load kernel page directory physical address in CR3 then set PG bit in CR0. Paging is therefore activated seamlessly.


_b- Mapping_


During kernel initialization, only 1:1 mapping occurs. Because page tables in kernel space are pre-allocated, mapping is reduced to create correct entries in corresponding page directory and page table. Nevertheless, there is a problem. How can we access page tables or page directories with virtual addresses ? (Remember that those structures are manipulated via their physical address). We use a trick that originated from the similarity between page directory entry and page table entry. We can make a page directory entry point to that page directory, creating a self-map entry. This illustrated by the following figure:

```
+-------------------------------+
|                               |
|                               |
|                     +------+------+----------+
|                     | Dir  | Tbl  | Offset   |
|                     +------+------+----------+
|                       |                     
|                       |                    
|   +-------------------+                    
|   |                                        
|   |                            +--------+   
|   |                            |        |
|   |                            +--------+
|   |     +--------+             |        |
|   +---> | PDE    |-+           +--------+
|         +--------+ |           |        |
|         |        | |           +--------+             
|         +--------+ |           | PTE    |                 
|         |        | |           +--------+                     
|         +--------+ |           |        |          
+-------> | PDE    |-|---------> +--------+                
          +--------+ |          
          |        | |           Page Table
      +-> +--------+ |
      |              |
      +--------------+ 
        Page Directory
```

We set the N<sup>th</sup> page directory entry to be the self-map entry. We can acces Page directory using virtual address:
`(N<<22)+(N<<12)` (22 is the number of left shift to reach `Dir` part and 12 to reach `Tbl` part). In the same way, we can acces i<sup>th</sup> page table using virtual address: `(N<<22)+(i<<12)`.



_c- Unmapping_


Unmapping during initialization is reduced to nullify page table entry corresponding to the virtual address to unmap. The pysical memory mapped to the virtual address can be released (because of 1:1 mapping, there cannot be several virtual addresses that point to one physical address). Acces to page directory and page table is done with the previous self-map trick.


## Interrupts/Exceptions ##


Interrupts (this term gathers interrupts and exceptions here) have similarity with virtual memory. The following big picture illustrates the x86 interrupts mechanism:

```
                 IDT                                           Code Segment
           +--------------+                                  +--------------+
           |              |                                  |              |
           +--------------+      Offset      / \             +--------------+
 Vector -->|     Gate     | -+--------------| + |----------->|   Handler    |
           +--------------+  |               \ /             +--------------+
           |              |  |                |              |              |
           +--------------+  | Segment        |              |              |
           |              |  | Selector       |              |              |
           +--------------+  |                |              |              |
           |              |  |                |              |              |
           +--------------+  |                |              |              |
                             |                |------------->+--------------+
      +----------------------+                | Base 
      |                                       | Address
      |            GDT                        |  
      |       +--------------+                | 
      |       |              |                |
      |       +--------------+                | 
      +------>| Seg. Desc.   |----------------+
              +--------------+
              |              |
              +--------------+
              |              |
              +--------------+
              |              |
              +--------------+
```

Each interrupt, hardware or software, is associated with a number, called the interrupt vector. That vector is in fact an index in a table called Interrupt Descriptor Table (IDT). The IDT is, like the GDT, a table of descriptors. It exists 3 kinds of descriptors:
  1. Task Gate Descriptor
  1. Interrupt Gate Descriptor
  1. Trap Gate Descriptor

Task Gate descriptor are used in hardware task switch, not in used in RhinOS. Interrupt Gate descriptor are used when an interrupt occurs. Trap Gate descriptor works the same as Interrupt Gate descriptor with little difference (EFLAGS handling). Only one descriptor will be used in RhinOS: the interrupt gate descriptor. Its structire is:

```


31                                         16 15 14  13 12        8 7      5 4    0
+-------------------------------------------+---+-----+------------+--------+-----+
|                                           |   |  D  |            |        |/////|
|        Offset 31:16                       | P |  P  | 0 D 1 1 0  | 0 0 0  |/////|  (4->7 B)
|                                           |   |  L  |            |        |/////|
+-------------------------------------------+---+-----+------------+--------------+


31                                        16 15                                   0
+-------------------------------------------+-------------------------------------+
|                                           |                                     |
|           Segment Selector                |            Offset 15:0              |  (0->3 B)
|                                           |                                     |
+-------------------------------------------+-------------------------------------+

```

Refer to Intel manuals for fields explanation. What is important is the `Segment Selector` and `Offset` fields. The first one contains the segment selector corresponding to the GDT segment descriptor describing the memory where resides the interrupt handler. The offset indicates the  handler address from that segment base address.

Before executing the handler, there is a context switch: the current task is interrupted and kernel code is executed. The processor save information on the stack during the switch:
  * Interrupted task executs at same privilege level as handler : Only CS, EIP, EFLAGS and an error code are pushed on current stack stack.
  * Interrupted task executs at different privilege level: There is a stack switch. Kernel will use the stack pointed by `esp0` field in TSS structure. CS, EIP EFLAGS, ESP, SS and an error code are pushed on that new stack, as shown in the following figure:

```

                 No privilege switch (same stack)

.              .
.              .
+--------------+
|              | <-- ESP before interrupt
+--------------+
|  Error code  |
+--------------+
|  EIP         |
+--------------+
|  CS          |
+--------------+
|  EFLAGS      | <-- ESP after
+--------------+
|              |
+--------------+
.              .
.              .


                 Privilege switch


.              .                              .              .
.              .                              .              . 
+--------------+                              +--------------+
|              | <-- ESP before interrupt     |              | 
+--------------+                              +--------------+
|              |               ESP after -->  |  Error Code  |
+--------------+                              +--------------+
|              |                              |  EIP         |
+--------------+                              +--------------+
|              |                              |  CS          |
+--------------+                              +--------------+
|              |                              |  EFLAGS      |
+--------------+                              +--------------+
|              |                              |  ESP         |
+--------------+                              +--------------+
.              .                              |  SS          |
.              .                              +--------------+
                                              |              |
   Interrupted task                           +--------------+
   stack                                      .              .
                                              .              .

                                           Interrupt handler stack
```


## Hardware Interrupts ##

Hardware interrupts can be controlled by (at least) two chipsets:
  1. Programmable Interrupt Controller (PIC)
  1. Advance Programmable Interrupt Controller (APIC)

We describe Intel 8259A PIC here. That hardware has 8 interrupt lines and can be extend to 14 with a second cascading PIC. In this case, the first PIC is the master and the second the slave. x86 works with 2 PICs.

PIC has to be programmed in order connect it to the IDT. This is done thanks to 2 memory ports:
  * Initilization port 0x20 (0xA0 for slave)
  * Data port 0x21 (0xA1 for slave)

2 kinds of commands can be sent to PIC through these ports:
  1. Initialization Command Words (ICWs)
  1. Operational Command Words (OCWs)

There is 4 ICWs to initialize PIC properly. they must be send in that order.

### ICW1 ###

It is sent through initilization port to both master and slave. Its structure is:

```
+---+---+---+---+------+-----+------+-----+
| 0 | 0 | 0 | 1 | LTIM | ADI | SNGL | IC4 |
+---+---+---+---+------+-----+------+-----+ 
```

  * LTIM:  Edge Triggered if not set,  Level Triggered otherwise
  * ADI: Adress Interval, no effect on x86
  * SNGL: Only one PIC if set, Cascading PIC otherwise
  * IC4: ICW4 read if set

With 2 PICs in edge triggered mode and ICW4 needed, **ICW1 = 00010001b = 0x11**


### ICW2 ###


It must be send on master and slave data port

```
+----+----+----+----+----+---+---+---+
| O5 | O4 | O3 | O2 | O1 | 0 | 0 | 0 |
+----+----+----+----+----+---+---+---+ 
```

ICW2 is hardware interrupt offset in IDT. Thus, if X=O<sub>5</sub>O<sub>4</sub>O<sub>3</sub>O<sub>2</sub>O<sub>1</sub>000 then IRQ 0 will match entry X in IDT, IRQ1 will match X+1 and so on.

Intel requires the 32 first IDT entries to be reserved to exceptions. Hardware interrupts can come after, so master **ICW2 will be 0x20** (32) and slave **ICW2 wille be 0x28** (40 as 32+8)


### ICW3 ###

ICW3 must be send on data port and differs from PICs.
Master ICW3 has the following structure:

```
+----+----+----+----+----+----+----+----+
| S7 | S6 | S5 | S4 | S3 | S2 | S1 | S0 |
+----+----+----+----+----+----+----+----+ 
```

S<sub>i</sub> is set if i<sup>th</sup> line is connected to slave PIC. Here, IRQ2 is connected to slave so **master ICW3 is 00000100b = 0x04**.

Slave ICW3 has the following structure:

```
+---+---+---+---+---+-----+-----+-----+
| 0 | 0 | 0 | 0 | 0 | ID2 | ID1 | ID0 |
+---+---+---+---+---+-----+-----+-----+ 
```

Bits 2 to 0 identify slave to master connexion. They represent 2<sup>3</sup> values matching the 8 interrupt lines. Line 2 is used to connect PICs on x86 so **slave ICW3 is 00000010b = 0x02**


### ICW4 ###

It must be sent on master and slave data port. It defines PIC working mode. Its structure is:
```
+---+---+---+------+-----+-----+------+-----+
| 0 | 0 | 0 | SFNM | BUF | M/S | AEOI | µPM |
+---+---+---+------+-----+-----+------+-----+ 
```

  * SFNM: set "Fully Nested" mode
  * BUF: set Buffer mode
  * M/S: set for Master Buffer mode, unset for Slave Buffer Mode (with BUF set)
  * AEOI: Automatic end of interrupt.
  * µPM: set for 8086 architecture


If AEOI is not set, operating system has to set end of interrupt by sending word 0x20 on initialization port. Note that if interrupt occurs on slave, operating system has to send EOI to master after slave. Here, only µPM is set so **ICW4 is 00000001b = 0x01**.


Once ICW4 is sent, we can interact with PICs thanks to OCWs. Only 2 are interesting, OCW1 and OCW2.


### OCW1 ###

It must be send on data port. It permits to enable or disable an interrupt line. Its structire is:

```
+----+----+----+----+----+----+----+----+
| M7 | M6 | M5 | M4 | M3 | M2 | M1 | M0 |
+----+----+----+----+----+----+----+----+ 
```

Setting bit M<sub>i</sub> will disable IRQ<sub>i</sub>. Note that line 2 must be activate for cascading.


### OCW2 ###

It must be send on initilization port. It is in fact the EOI word. Its structure is:

```
+---+----+-----+---+---+----+----+----+
| R | SL | EOI | 0 | 0 | L2 | L1 | L0 |
+---+----+-----+---+---+----+----+----+ 
```

All we have to know is that EOI must be set to report an end of interrupt. That's why we have to send value 00100000b = 0x20.