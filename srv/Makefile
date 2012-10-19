# Servers Makefile
#

# Macros

CC	=	gcc -Wall -c
CFLAGS	=	-I../include

# Fichiers source et de sortie

C_SRC	=	user.c
C_OUT	=	${C_SRC:.c=.o}
OBJ	=	$(ASM_OUT) $(C_OUT)

# Cibles

all: $(OBJ)

depend:
	makedepend -- $(CFLAGS) -- $(C_SRC)

clean:
	$(RM) *~
	$(RM) $(C_OUT)

# DO NOT DELETE

user.o: ../include/types.h ../include/ipc.h
