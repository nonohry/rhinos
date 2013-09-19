# Servers Makefile
#

# Macros

ARCH	?=	x86

CC	:=	gcc -Wall -c
CFLAGS	:=	-I../include -I../include/arch/$(ARCH)

# Files

C_SRC	:=	user_send.c user_recv.c
C_OUT	:=	${C_SRC:.c=.o}
OBJ	:=	$(ASM_OUT) $(C_OUT)

# Targets

all: $(OBJ)

depend:
	makedepend -- $(CFLAGS) -- $(C_SRC)

clean:
	$(RM) *~
	$(RM) $(C_OUT)

# DO NOT DELETE

user.o: ../include/define.h ../include/arch/x86/types.h ../include/ipc.h
