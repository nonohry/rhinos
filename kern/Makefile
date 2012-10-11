# Kernel Makefile
#

# Macros

AS	=	nasm -f elf
CC	=	gcc -Wall -c
RM	=	rm -f
LD	=	ld -s -T link.ld
CFLAGS	=	-I../include

# Regle de Suffixes

.SUFFIXES: .s

.s.o:
	$(AS) $<

# Fichiers source et de sortie

ASM_SRC	=	khead.s klib_s.s interrupt.s
ASM_OUT	=	${ASM_SRC:.s=.o}
C_SRC	=	start.c seg.c tables.c pic.c pit.c irq.c exceptions.c physmem.c paging.c virtmem_buddy.c virtmem_slab.c virtmem.c thread.c sched.c syscall.c klib_c.c proc.c main.c 
C_OUT	=	${C_SRC:.c=.o}
OBJ	=	$(ASM_OUT) $(C_OUT)

# Cibles

all: $(OBJ)


asm:	$(ASM_OUT)

depend:
	makedepend -- $(CFLAGS) -- $(C_SRC)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT) $(C_OUT) kern

# DO NOT DELETE

start.o: klib.h ../include/types.h const.h tables.h seg.h irq.h interrupt.h
start.o: thread.h ../include/ipc.h start.h
seg.o: ../include/types.h const.h seg.h
tables.o: ../include/types.h const.h klib.h pic.h seg.h interrupt.h thread.h
tables.o: ../include/ipc.h tables.h irq.h
pic.o: ../include/types.h const.h klib.h pic.h
pit.o: ../include/types.h const.h klib.h interrupt.h thread.h
pit.o: ../include/ipc.h irq.h sched.h pit.h
irq.o: ../include/types.h const.h ../include/llist.h klib.h interrupt.h
irq.o: thread.h ../include/ipc.h pic.h irq.h
exceptions.o: ../include/types.h const.h klib.h thread.h ../include/ipc.h
exceptions.o: exceptions.h
physmem.o: ../include/types.h const.h ../include/llist.h klib.h start.h
physmem.o: physmem.h virtmem_slab.h virtmem_buddy.h
paging.o: ../include/types.h const.h klib.h start.h physmem.h virtmem_slab.h
paging.o: virtmem_buddy.h paging.h
virtmem_buddy.o: ../include/types.h const.h ../include/llist.h klib.h start.h
virtmem_buddy.o: physmem.h virtmem_slab.h virtmem_buddy.h paging.h
virtmem_slab.o: ../include/types.h const.h ../include/llist.h klib.h start.h
virtmem_slab.o: physmem.h virtmem_slab.h virtmem_buddy.h paging.h
virtmem.o: ../include/types.h const.h virtmem_buddy.h virtmem_slab.h
virtmem.o: virtmem.h
thread.o: ../include/types.h ../include/llist.h const.h klib.h tables.h seg.h
thread.o: irq.h interrupt.h thread.h ../include/ipc.h virtmem_slab.h
thread.o: virtmem.h sched.h physmem.h virtmem_buddy.h paging.h
sched.o: ../include/types.h ../include/llist.h klib.h const.h thread.h
sched.o: ../include/ipc.h sched.h
syscall.o: ../include/types.h ../include/llist.h ../include/ipc.h const.h
syscall.o: klib.h thread.h sched.h physmem.h virtmem_slab.h virtmem_buddy.h
syscall.o: virtmem.h paging.h syscall.h
klib_c.o: ../include/types.h klib.h const.h
proc.o: ../include/types.h ../include/llist.h const.h thread.h
proc.o: ../include/ipc.h klib.h virtmem_slab.h paging.h proc.h
main.o: ../include/types.h ../include/llist.h const.h start.h klib.h
main.o: physmem.h virtmem_slab.h virtmem_buddy.h paging.h virtmem.h thread.h
main.o: ../include/ipc.h sched.h irq.h interrupt.h pit.h
