# Kernel Makefile
#

# Macros

AS	=	nasm -f elf
CC	=	gcc -Wall -c
RM	=	rm -f
LD	=	ld -s --oformat binary -Ttext 0x8200

# Regle de Suffixes

.SUFFIXES: .s .c

.s.o:
	$(AS) $<

.c.o: 
	$(CC) $<

# Fichiers source et de sortie
ASM_SRC	=	khead.s klib.s
ASM_OUT	=	${ASM_SRC:.s=.o}
C_SRC	=	start.c prot.c i8259.c main.c 
C_OUT	=	${C_SRC:.c=.o}
HEADER  =       types.h prot.h klib.h i8259.h
OBJ	=	$(ASM_OUT) $(C_OUT)

# Cibles

kern: $(OBJ) $(HEADER)
	$(LD) -o kern $(OBJ)

lowlvl:	$(ASM_OUT)

ccode: $(C_OUT)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT) $(C_OUT) kern
