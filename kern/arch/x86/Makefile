# x86 Makefile
#

# Macros

ARCH	?=	x86

AS	:=	nasm -f elf
CC	:=	gcc -Wall -c
RM	:=	rm -f
CFLAGS	:=	-I../../../include -I../../../include/arch/$(ARCH) -I../..

# Suffixes rules
.SUFFIXES: .s

.s.o:
	$(AS) $<

# Files
ASM_SRC	=	krt.s x86_lib.s
ASM_OUT	=	${ASM_SRC:.s=.o}
C_SRC	=	setup.c e820.c vm_segment.c vm_paging.c serial.c
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

setup.o: ../../../include/define.h ../../../include/arch/x86/types.h
setup.o: ../../boot.h x86_const.h x86_lib.h serial.h e820.h setup.h
setup.o: vm_segment.h vm_paging.h
e820.o: ../../../include/define.h ../../../include/arch/x86/types.h
e820.o: x86_const.h serial.h setup.h e820.h
vm_segment.o: ../../../include/define.h ../../../include/arch/x86/types.h
vm_segment.o: x86_const.h vm_segment.h
vm_paging.o: ../../../include/define.h ../../../include/arch/x86/types.h
vm_paging.o: x86_lib.h x86_const.h serial.h vm_paging.h
serial.o: ../../../include/define.h ../../../include/arch/x86/types.h
serial.o: x86_lib.h x86_const.h serial.h
