# Kernel Makefile
#

# Macros

AS	=	nasm -f elf
CC	=	gcc -Wall -c
RM	=	rm -f
LD	=	ld -s --oformat binary -Ttext 0xC00
CFLAGS	=	-I../include

# Regle de Suffixes

.SUFFIXES: .s .c

.s.o:
	$(AS) $<

.c.o: 
	$(CC) $(CFLAGS) $<

# Fichiers source et de sortie

ASM_SRC	=	khead.s klib.s
ASM_OUT	=	${ASM_SRC:.s=.o}
C_SRC	=	start.c prot.c i8259.c i82C54.c exceptions.c proc.c main.c 
C_OUT	=	${C_SRC:.c=.o}
HEADER  =       prot.h klib.h i8259.h i82C54.h exceptions.h proc.h start.h elf.h
OBJ	=	$(ASM_OUT) $(C_OUT)

# Cibles

kern: $(OBJ) $(HEADER)
	$(LD) $(CFLAGS) -o kern $(OBJ)

lowlvl:	$(ASM_OUT)

ccode: $(C_OUT)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT) $(C_OUT) kern
