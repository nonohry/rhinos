# Rotating Staircase Deadline Scheduling #

## Overview ##

Each proc has a quotum at its priority and below
If it uses its quantum, it is lowered to the next priority with a new quantum. When last priority is reached, it is moved to an expired structure.

To prevent continous flow in high priority, each priority has a global quantum. When this quantum reaches 0, all process at that priority are lowered to the next priotity, even if they have never run at their original priority.

If the active structure becomes empty, the expired one becomes the active one and vice versa

Example, with 4 priorities, quantum of 1, Global quantum 2 :


t=0

```

     +-+                            +-+
     | | - P1 - P2 - P3             | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | | - P4                       | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+

    Active                        Expired

```

t=1, P1 is lowered

```

     +-+                            +-+
     | | - P2 - P3                  | |
     +-+                            +-+
     | | - P1                       | |
     +-+                            +-+
     | | - P4                       | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+

    Active                        Expired

```

t=2, P2 is lowered, as well as P3 because of global quantum in first priority

```

     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | | - P3 - P1 - P2             | |
     +-+                            +-+
     | | - P4                       | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+

    Active                        Expired

```

t=3, P3 is lowered

```

     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | | - P1 - P2                  | |
     +-+                            +-+
     | | - P4 - P3                  | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+

    Active                        Expired

```

t=4, P1 is lowered, P2 too because of global quantum

```

     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | | - P2 - P4 - P3 - P1        | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+

    Active                        Expired

```

t=5, P2 is lowered

```

     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | | - P4 - P3 - P1             | |
     +-+                            +-+
     | | - P2                       | |
     +-+                            +-+

    Active                        Expired

```

t=6, P4 is lowered, as well as P3 and P1 because of global quantum

```

     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | | - P3 - P1 - P2 - P4        | |
     +-+                            +-+

    Active                        Expired

```

t=7, P3 expired

```

     +-+                            +-+
     | |                            | | - P3
     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | |                            | | 
     +-+                            +-+
     | | - P1 - P2 - P4             | |
     +-+                            +-+

    Active                        Expired

```

t=8, P1 expired

```

     +-+                            +-+
     | |                            | | - P3 - P1
     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | |                            | | 
     +-+                            +-+
     | | - P2 - P4                  | |
     +-+                            +-+

    Active                        Expired

```

t=9, P2 expired

```

     +-+                            +-+
     | |                            | | - P3 - P1 - P2 
     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | |                            | | 
     +-+                            +-+
     | | - P4                       | |
     +-+                            +-+

    Active                        Expired

```

t=9, P4 expired, Active is empty so Expired become the Active.

```

     +-+                            +-+
     | |                            | | - P3 - P1 - P1 
     +-+                            +-+
     | |                            | |
     +-+                            +-+
     | |                            | | - P4 
     +-+                            +-+
     | |                            | |
     +-+                            +-+

 Active ->Expired              Expired -> Active

```

And so on....

Before a major rotation (Active -> Expired), P1 has run 3 times, P2 3 times, P3 2 times and P4 2 times. It seems unfair that P3 has run only 2 times, but at the end of the round, P3 is at the beginning of the highest priority so it will run on more time than P1 and P4. After N round (where N=number of proc), all proc in the same priority will have run the same amount of time, which will be more than lower priority proc.

To achieve this, notice that when a proc is lowered, it is enqueue whereas when all proc of an priority are lowered due to global quantum, they are placed at the top of priority list.

Another note: if a global quantum switch occur, lowered threads dont't obtain a new quantum.

## Implementation Hints ##


Need a thread list

```
struct thread_list
{
   u16_t quantum;
   struct thread* list;
};
```

Need a scheduler structure

```
struct scheduler
{
   struct thread_list active[PRIO_MAX];
   struct thread_list expired[PRIO_MAX];
};
```

Threads need a sched structure

```
struct sched
{
   u8_t static_prio;
   u8_t dynamic_prio;
   u16_t static_quantum;
   u16_t dynamic_quantum;
};
```

Scheduler needs an API, other modules dont have to know the implementation. Old calls like `sched_enqueue` or direct queue manipulation with LLIST\_ADD/REMOVE (seen in syscall.c) have to be prohibited, even direct thread state manipulation. One should interact with scheduler through api like `sched_set_blocked(thread)`, `sched_set_ready(thread)`...


We have to disassociate the tick handling and the thread election.