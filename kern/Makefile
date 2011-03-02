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
C_SRC	=	start.c seg.c tables.c i8259.c i82C54.c exceptions.c bootmem.c main.c 
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

start.o: tables.h ../include/types.h seg.h start.h
seg.o: ../include/types.h seg.h
tables.o: ../include/types.h klib.h i8259.h tables.h seg.h
i8259.o: ../include/types.h klib.h tables.h seg.h i8259.h
i82C54.o: ../include/types.h klib.h tables.h seg.h i8259.h i82C54.h
exceptions.o: ../include/types.h klib.h exceptions.h
bootmem.o: ../include/types.h klib.h start.h bootmem.h
main.o: ../include/types.h ../include/llist.h start.h klib.h i82C54.h
main.o: tables.h seg.h bootmem.h
