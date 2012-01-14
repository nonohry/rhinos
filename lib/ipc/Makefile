# IPC Makefile
#

# Macros

AS	=	nasm -f elf
CC	=	gcc -Wall -c
RM	=	rm -f

# Regle de Suffixes

.SUFFIXES: .s

.s.o:
	$(AS) $<

# Fichiers source et de sortie

ASM_SRC	=	ipc.s
ASM_OUT	=	${ASM_SRC:.s=.o}
OBJ	=	$(ASM_OUT) 

# Cibles

all: $(OBJ)

asm:	$(ASM_OUT)

depend:
	makedepend -- -- $(ASM_SRC)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT)

# DO NOT DELETE
