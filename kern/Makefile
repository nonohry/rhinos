# Kernel Makefile
#

# Macros

AS	=	nasm -f elf
CC	=	gcc -Wall -c
RM	=	rm -f
LD	=	ld --oformat binary -Ttext 0x8200

# Regle de Suffixes

.SUFFIXES: .s .c

.s.o:
	$(AS) $<

.c.o: 
	$(CC) $<

# Fichiers source et de sortie
ASM_SRC	=	khead.s
ASM_OUT	=	${ASM_SRC:.s=.o}
C_SRC	=	start.c
C_OUT	=	${C_SRC:.c=.o}
OBJ	=	$(ASM_OUT) $(C_OUT)

# Cibles

kern: $(OBJ)
	$(LD) -o kern $(OBJ)

lowlvl:	$(ASM_OUT)

ccode: $(C_OUT)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT) $(C_OUT) kern
