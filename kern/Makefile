# Kernel Makefile
#

# Macros

AS	=	nasm -f elf
CC	=	gcc -Wall -c
RM	=	rm -f
LD	=	ld -s -T link.ld
CFLAGS	=	-I../include

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

start.o: ../include/define.h ../include/types.h const.h paging.h klib.h
start.o: tables.h seg.h start.h
seg.o: ../include/define.h ../include/types.h const.h paging.h seg.h
tables.o: ../include/define.h ../include/types.h const.h paging.h klib.h
tables.o: pic.h seg.h interrupt.h thread.h ../include/ipc.h proc.h tables.h
pic.o: ../include/define.h ../include/types.h const.h paging.h klib.h pic.h
pit.o: ../include/define.h ../include/types.h const.h paging.h klib.h
pit.o: interrupt.h thread.h ../include/ipc.h proc.h irq.h sched.h pit.h
irq.o: ../include/define.h ../include/types.h const.h paging.h
irq.o: ../include/llist.h klib.h interrupt.h thread.h ../include/ipc.h proc.h
irq.o: pic.h irq.h
exceptions.o: ../include/define.h ../include/types.h const.h paging.h klib.h
exceptions.o: thread.h ../include/ipc.h proc.h exceptions.h
physmem.o: ../include/define.h ../include/types.h const.h paging.h
physmem.o: ../include/llist.h klib.h start.h physmem.h virtmem_slab.h
physmem.o: virtmem_buddy.h
paging.o: ../include/define.h ../include/types.h const.h paging.h klib.h
paging.o: start.h physmem.h virtmem_slab.h virtmem_buddy.h
virtmem_buddy.o: ../include/define.h ../include/types.h const.h paging.h
virtmem_buddy.o: ../include/llist.h klib.h start.h physmem.h virtmem_slab.h
virtmem_buddy.o: virtmem_buddy.h
virtmem_slab.o: ../include/define.h ../include/types.h const.h paging.h
virtmem_slab.o: ../include/llist.h klib.h start.h physmem.h virtmem_slab.h
virtmem_slab.o: virtmem_buddy.h
virtmem.o: ../include/define.h ../include/types.h const.h paging.h
virtmem.o: virtmem_buddy.h virtmem_slab.h virtmem.h
thread.o: ../include/define.h ../include/types.h ../include/llist.h const.h
thread.o: paging.h klib.h tables.h seg.h virtmem_slab.h virtmem.h sched.h
thread.o: thread.h ../include/ipc.h proc.h
sched.o: ../include/define.h ../include/types.h ../include/llist.h klib.h
sched.o: const.h paging.h thread.h ../include/ipc.h proc.h sched.h
syscall.o: ../include/define.h ../include/types.h ../include/llist.h
syscall.o: ../include/ipc.h const.h paging.h klib.h thread.h proc.h sched.h
syscall.o: syscall.h
klib_c.o: ../include/define.h ../include/types.h klib.h const.h paging.h
proc.o: ../include/define.h ../include/types.h ../include/llist.h const.h
proc.o: paging.h klib.h virtmem_slab.h thread.h ../include/ipc.h proc.h
main.o: ../include/define.h ../include/types.h ../include/llist.h const.h
main.o: paging.h start.h klib.h physmem.h virtmem_slab.h virtmem_buddy.h
main.o: virtmem.h thread.h ../include/ipc.h proc.h sched.h irq.h interrupt.h
main.o: pit.h
