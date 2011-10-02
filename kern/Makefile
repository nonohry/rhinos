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

ASM_SRC	=	khead.s klib.s interrupt.s
ASM_OUT	=	${ASM_SRC:.s=.o}
C_SRC	=	start.c seg.c tables.c pic.c pit.c irq.c exceptions.c physmem.c paging.c virtmem_buddy.c virtmem_slab.c virtmem.c context_cpu.c main.c 
C_OUT	=	${C_SRC:.c=.o}
OBJ	=	$(ASM_OUT) $(C_OUT)

# Cibles

kern: $(OBJ)
	$(LD) $(CFLAGS) -o kern $(OBJ)

asm:	$(ASM_OUT)

depend:
	makedepend -- $(CFLAGS) -- $(C_SRC)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT) $(C_OUT) kern

# DO NOT DELETE

start.o: klib.h ../include/types.h const.h tables.h seg.h irq.h interrupt.h
start.o: context_cpu.h start.h
seg.o: ../include/types.h const.h seg.h
tables.o: ../include/types.h const.h klib.h pic.h seg.h interrupt.h
tables.o: context_cpu.h tables.h irq.h
pic.o: ../include/types.h const.h klib.h pic.h
pit.o: ../include/types.h const.h klib.h tables.h seg.h irq.h interrupt.h
pit.o: context_cpu.h pit.h
irq.o: ../include/types.h const.h ../include/llist.h klib.h interrupt.h
irq.o: context_cpu.h pic.h irq.h
exceptions.o: ../include/types.h const.h klib.h exceptions.h context_cpu.h
physmem.o: ../include/types.h const.h ../include/llist.h klib.h assert.h
physmem.o: start.h physmem.h virtmem_slab.h virtmem_buddy.h
paging.o: ../include/types.h const.h klib.h assert.h start.h physmem.h
paging.o: virtmem_slab.h virtmem_buddy.h paging.h
virtmem_buddy.o: ../include/types.h const.h ../include/llist.h klib.h
virtmem_buddy.o: assert.h start.h physmem.h virtmem_slab.h virtmem_buddy.h
virtmem_buddy.o: paging.h
virtmem_slab.o: ../include/types.h const.h ../include/llist.h klib.h assert.h
virtmem_slab.o: start.h physmem.h virtmem_slab.h virtmem_buddy.h paging.h
virtmem.o: ../include/types.h const.h assert.h klib.h virtmem_buddy.h
virtmem.o: virtmem_slab.h virtmem.h
context_cpu.o: ../include/types.h const.h klib.h assert.h virtmem.h
context_cpu.o: virtmem_slab.h context_cpu.h
main.o: ../include/types.h const.h ../include/llist.h start.h klib.h assert.h
main.o: physmem.h virtmem_slab.h virtmem_buddy.h paging.h virtmem.h
main.o: context_cpu.h irq.h interrupt.h pit.h
