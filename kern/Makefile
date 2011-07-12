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

ASM_SRC	=	khead.s klib.s
ASM_OUT	=	${ASM_SRC:.s=.o}
C_SRC	=	start.c seg.c tables.c pic.c pit.c irq.c exceptions.c physmem.c paging.c virtmem_buddy.c virtmem_slab.c main.c 
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

start.o: klib.h ../include/types.h tables.h seg.h irq.h start.h
seg.o: ../include/types.h seg.h
tables.o: ../include/types.h klib.h pic.h seg.h tables.h irq.h
pic.o: ../include/types.h klib.h pic.h
pit.o: ../include/types.h klib.h tables.h seg.h irq.h pit.h
irq.o: ../include/types.h ../include/llist.h klib.h pic.h irq.h
exceptions.o: ../include/types.h klib.h exceptions.h
physmem.o: ../include/types.h ../include/llist.h klib.h start.h physmem.h
paging.o: ../include/types.h klib.h start.h physmem.h paging.h
virtmem_buddy.o: ../include/types.h ../include/wmalloc.h klib.h start.h
virtmem_buddy.o: physmem.h paging.h virtmem_buddy.h
main.o: ../include/types.h ../include/llist.h start.h klib.h physmem.h
main.o: paging.h irq.h pit.h virtmem_buddy.h ../include/wmalloc.h
