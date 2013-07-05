# Kernel Makefile
#

# Macros

AS	=	nasm -f elf
CC	=	gcc -Wall -c
RM	=	rm -f
LD	=	ld -s -T link.ld
CFLAGS	=	-I../include -I../include/arch/x86

# Suffixes rules
.SUFFIXES: .s

.s.o:
	$(AS) $<

# Files
ASM_SRC	=	khead.s klib_s.s interrupt.s
ASM_OUT	=	${ASM_SRC:.s=.o}
C_SRC	=	start.c seg.c tables.c pic.c pit.c irq.c exceptions.c physmem.c paging.c virtmem_buddy.c virtmem_slab.c virtmem.c thread.c sched.c syscall.c klib_c.c proc.c main.c 
C_OUT	=	${C_SRC:.c=.o}
OBJ	=	$(ASM_OUT) $(C_OUT)

# Targets

all: $(OBJ)


asm:	$(ASM_OUT)

depend:
	makedepend -- $(CFLAGS) -- $(C_SRC)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT) $(C_OUT) kern

# DO NOT DELETE

start.o: ../include/define.h ../include/arch/x86/types.h const.h paging.h
start.o: klib.h tables.h seg.h start.h
seg.o: ../include/define.h ../include/arch/x86/types.h const.h paging.h seg.h
tables.o: ../include/define.h ../include/arch/x86/types.h const.h paging.h
tables.o: klib.h pic.h seg.h interrupt.h thread.h ../include/ipc.h proc.h
tables.o: tables.h
pic.o: ../include/define.h ../include/arch/x86/types.h const.h paging.h
pic.o: klib.h pic.h
pit.o: ../include/define.h ../include/arch/x86/types.h const.h paging.h
pit.o: klib.h interrupt.h thread.h ../include/ipc.h proc.h irq.h sched.h
pit.o: pit.h
irq.o: ../include/define.h ../include/arch/x86/types.h const.h paging.h
irq.o: ../include/llist.h klib.h interrupt.h thread.h ../include/ipc.h proc.h
irq.o: pic.h irq.h
exceptions.o: ../include/define.h ../include/arch/x86/types.h const.h
exceptions.o: paging.h klib.h thread.h ../include/ipc.h proc.h exceptions.h
physmem.o: ../include/define.h ../include/arch/x86/types.h const.h paging.h
physmem.o: ../include/llist.h klib.h start.h physmem.h virtmem_slab.h
physmem.o: virtmem_buddy.h
paging.o: ../include/define.h ../include/arch/x86/types.h const.h paging.h
paging.o: klib.h start.h physmem.h virtmem_slab.h virtmem_buddy.h
virtmem_buddy.o: ../include/define.h ../include/arch/x86/types.h const.h
virtmem_buddy.o: paging.h ../include/llist.h klib.h start.h physmem.h
virtmem_buddy.o: virtmem_slab.h virtmem_buddy.h
virtmem_slab.o: ../include/define.h ../include/arch/x86/types.h const.h
virtmem_slab.o: paging.h ../include/llist.h klib.h start.h physmem.h
virtmem_slab.o: virtmem_slab.h virtmem_buddy.h
virtmem.o: ../include/define.h ../include/arch/x86/types.h const.h paging.h
virtmem.o: virtmem_buddy.h virtmem_slab.h virtmem.h
thread.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
thread.o: const.h paging.h klib.h tables.h seg.h virtmem_slab.h virtmem.h
thread.o: sched.h thread.h ../include/ipc.h proc.h
sched.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
sched.o: klib.h const.h paging.h thread.h ../include/ipc.h proc.h sched.h
syscall.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
syscall.o: ../include/ipc.h const.h paging.h klib.h thread.h proc.h sched.h
syscall.o: syscall.h
klib_c.o: ../include/define.h ../include/arch/x86/types.h klib.h const.h
klib_c.o: paging.h
proc.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
proc.o: const.h paging.h klib.h virtmem_slab.h thread.h ../include/ipc.h
proc.o: proc.h
main.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
main.o: const.h paging.h start.h klib.h physmem.h virtmem_slab.h
main.o: virtmem_buddy.h virtmem.h thread.h ../include/ipc.h proc.h sched.h
main.o: irq.h interrupt.h pit.h
