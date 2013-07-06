# x86 Makefile
#

# Macros

ARCH	?=	x86

AS	:=	nasm -f elf
CC	:=	gcc -Wall -c
RM	:=	rm -f
LD	:=	ld -s -T link.ld
CFLAGS	:=	-I../../../include -I../../../include/arch/$(ARCH)

# Suffixes rules
.SUFFIXES: .s

.s.o:
	$(AS) $<

# Files
ASM_SRC	=	krt.s x86_lib.s
ASM_OUT	=	${ASM_SRC:.s=.o}
C_SRC	=	segment.c serial.c
C_OUT	=	${C_SRC:.c=.o}
OBJ	=	$(ASM_OUT) $(C_OUT)

# Targets

all: $(OBJ)

asm:	$(ASM_OUT)

depend:
	makedepend -- $(CFLAGS) -- $(C_SRC)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT) $(C_OUT)

# DO NOT DELETE

segment.o: ../../../include/define.h ../../../include/arch/x86/types.h
segment.o: x86_const.h segment.h
serial.o: ../../../include/define.h ../../../include/arch/x86/types.h
serial.o: x86_lib.h serial.h
