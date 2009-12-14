# Boot Makefile
#

# Macros

AS	=	nasm
RM	=	rm -f

# Regle de Suffixes

.SUFFIXES: .s 

.s:
	$(AS) $< 

# Fichiers source et de sortie
ASM_SRC	=	boot0.s boot1.s
ASM_OUT	=	${ASM_SRC:.s=}

# Cibles

boot:	$(ASM_OUT)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT)
