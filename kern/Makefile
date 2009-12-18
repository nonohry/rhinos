# Kernel Makefile
#

# Macros

AS	=	nasm
RM	=	rm -f

# Regle de Suffixes

.SUFFIXES: .s 

.s:
	$(AS) $< 

# Fichiers source et de sortie
ASM_SRC	=	lowlvl.s
ASM_OUT	=	${ASM_SRC:.s=}

# Cibles

kern:	$(ASM_OUT)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT)
