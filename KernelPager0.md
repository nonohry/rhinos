Provide physical page when page fault occurs. In fact, Pager0 is a physical memory mangaer

Physical pages come from a tiny pool, sufficient for initialization (lower memory on x86 for example)

In this pool, a bitmap is used to represent free or allocated physical page

Code looks like that:

```
#include <stdio.h>

#define FREE 0
#define USED 1

char pager[3];

int getPage(int i)
{
        return (pager[i>>3] & (1 << (i%8)));
}

void setPage(int i, int state)
{
        if (state == FREE)
        {
                pager[i>>3] &= ~(1<<(i%8));
        }
        else
        {
                pager[i>>3] |= (1<<(i%8));
        }

        return;
}

int main()
{

        int i;

        for(i=0;i<3*8;i++)
        {
                printf("Page %d is %s\n",i,getPage(i)?"USED":"FREE");
        }

        printf("\nSetting page 18 and 0 as USED\n\n");
        setPage(18,USED);
        setPage(0,USED);

        for(i=0;i<3*8;i++)
        {
                printf("Page %d is %s\n",i,getPage(i)?"USED":"FREE");
        }


        printf("\nSetting page 18 as FREE\n\n");
        setPage(18,FREE);

        for(i=0;i<3*8;i++)
        {
                printf("Page %d is %s\n",i,getPage(i)?"USED":"FREE");
        }


        return 0;
}
```


In general, pager will be linked to threads. 2 kinds of pager:
  * Internal pager : pager in kernel or trhread adrees space
  * External pager : pager is another processus

page fault handler will look like this:

```

pf_handle(thread* faulting_th)
{
   vaddr = get_faulting_address();
   type = get_pf_type(); // Internal (page table) or external (missing physical frame)

   if (faulting_th->pager.type == PAGER_INTERNAL)
   {
      (paddr,rw,super) = fauting_th->pager(vaddr,type);
   }
   else
   {
     send(faulting_th->pager,MESSAGE(vaddr,type))
     receive(faulting_th->pager,&msg)
     (paddr,rw,super)=ExtractPhysAddr(msg)
   }

   fix_pf(vaddr,paddr,type,rw,super); //arch dependant

   return;
}

```

Pager will be defined with an union and a enum :

```

enum pager_type
{
   PAGER_INTERNAL,
   PAGER_EXTERNAL
};

struct pager
{
   enum pager_type type;
   union
   {
      (*func)(vaddr_t, u8_t);
      u32_t id;
   }pager;
};

```

Each proc & thread will be created with a pager and could change its pager during its execution.
Boot modules are created with pager0 as a pager
They will change for their own pager or for an external one after having their working set in memory. This change is possible through message passing to kernel. (Message passing is done through registers and will not trigger page fault)

In case of program loading, Pager0 will only do in memory mapping. It does not retrieve pages from disk or swap space (as there is no drivers at that time). Only pages already present in memory will be retrieved. As a result pager0 can only be used for boot modules as their code and data are loaded in memory by bootloader.
Pager0 wil have to read ELF format to decide if a page is a code or data page and set the rw/ro flag accordingly. If faulting adress is not in an elf section, pager0 will provide in 0-fill page (in case of paging structures, heap,...)
Pseudo C code could be:

```

phyaddr_t pager0(virtaddr_t vaddr,u8_t* flag)
{
   /* Paging Internal Structures */
   if (flag == INTERNAL)
   {
      p=pager0_alloc();
      return p;
   }

   /* Not internal structures */
   if (flag = EXTERNAL)
   {
      /* Page needed for code or data */
      if ( IS_CODE_OR_DATA(cur_th,vaddr) )
      {
         /* Get in memory code/data */
         p = cur_th->elf[vaddr]

         /* Super flag */
         if (vaddr < ARCH_CONST_KERN_HIGHMEM)
         {
             *flag |= ARCH_PF_SUPER;
         }

         /* RW flag */
         if (IS_DATA(vaddr,cur_th))
         {
             *flag |= ARCH_PF_RW;
         }

         /* ELF flag */
         *flag |= ARCH_PF_ELF;

         return p;
      }
      else
      {
         /* Page need for other thing (heap,...) */
         p = pager0_alloc()
         return p;
      }
   }

   return EXIT_FAILURE;
}

```


Pager0 will map all the physical adress space. Other pagers can request a part of pager0's space to operate. As a result, we can have a recursive construction of pagers, all requests from child to parent is done through message passing