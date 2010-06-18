# Memory Manager Makefile
#

# Macros

CC	=	gcc -Wall -c
RM	=	rm -f
CFLAGS	=	-I../../include
LD	=	ld -Ttext 0x0

# Regle de Suffixes

.c.o: 
	$(CC) $(CFLAGS) $<

# Fichiers source et de sortie

C_SRC	=	main.c
C_OUT	=	${C_SRC:.c=.o}
HEADER  =       
OBJ	=	$(C_OUT)

# Cibles

mm: $(OBJ) $(HEADER)
	$(LD) $(CFLAGS) -o mm $(OBJ)

ccode:  $(C_OUT)

clean:
	$(RM) *~
	$(RM) $(C_OUT) mm