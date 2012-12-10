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

start.o: klib.h ../include/types.h const.h start.h paging.h tables.h seg.h
start.o: irq.h interrupt.h thread.h ../include/ipc.h proc.h
seg.o: ../include/types.h const.h start.h paging.h seg.h
tables.o: ../include/types.h const.h start.h paging.h klib.h pic.h seg.h
tables.o: interrupt.h thread.h ../include/ipc.h proc.h tables.h irq.h
pic.o: ../include/types.h const.h start.h paging.h klib.h pic.h
pit.o: ../include/types.h const.h start.h paging.h klib.h interrupt.h
pit.o: thread.h ../include/ipc.h proc.h irq.h sched.h pit.h
irq.o: ../include/types.h const.h start.h paging.h ../include/llist.h klib.h
irq.o: interrupt.h thread.h ../include/ipc.h proc.h pic.h irq.h
exceptions.o: ../include/types.h const.h start.h paging.h klib.h thread.h
exceptions.o: ../include/ipc.h proc.h exceptions.h
physmem.o: ../include/types.h const.h start.h paging.h ../include/llist.h
physmem.o: klib.h physmem.h virtmem_slab.h virtmem_buddy.h
paging.o: ../include/types.h const.h start.h paging.h klib.h physmem.h
paging.o: virtmem_slab.h virtmem_buddy.h
virtmem_buddy.o: ../include/types.h const.h start.h paging.h
virtmem_buddy.o: ../include/llist.h klib.h physmem.h virtmem_slab.h
virtmem_buddy.o: virtmem_buddy.h
virtmem_slab.o: ../include/types.h const.h start.h paging.h
virtmem_slab.o: ../include/llist.h klib.h physmem.h virtmem_slab.h
virtmem_slab.o: virtmem_buddy.h
virtmem.o: ../include/types.h const.h start.h paging.h virtmem_buddy.h
virtmem.o: virtmem_slab.h virtmem.h
thread.o: ../include/types.h ../include/llist.h const.h start.h paging.h
thread.o: klib.h tables.h seg.h irq.h interrupt.h thread.h ../include/ipc.h
thread.o: proc.h virtmem_slab.h virtmem.h sched.h physmem.h virtmem_buddy.h
sched.o: ../include/types.h ../include/llist.h klib.h const.h start.h
sched.o: paging.h thread.h ../include/ipc.h proc.h sched.h
syscall.o: ../include/types.h ../include/llist.h ../include/ipc.h const.h
syscall.o: start.h paging.h klib.h thread.h proc.h sched.h physmem.h
syscall.o: virtmem_slab.h virtmem_buddy.h virtmem.h syscall.h
klib_c.o: ../include/types.h klib.h const.h start.h paging.h
proc.o: ../include/types.h ../include/llist.h const.h start.h paging.h
proc.o: thread.h ../include/ipc.h proc.h klib.h physmem.h virtmem_slab.h
proc.o: virtmem_buddy.h
main.o: ../include/types.h ../include/llist.h const.h start.h paging.h klib.h
main.o: physmem.h virtmem_slab.h virtmem_buddy.h virtmem.h thread.h
main.o: ../include/ipc.h proc.h sched.h irq.h interrupt.h pit.h
