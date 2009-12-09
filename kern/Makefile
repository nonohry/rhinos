# Kernel Makefile
#

# Macros

AS	=	nasm

# Regle de Suffixes

.SUFFIXES: .s 

.s:
	$(AS) $< 

# Fichiers source et de sortie
ASM_SRC	=	boot2.s
ASM_OUT	=	${ASM_SRC:.s=}

# Cibles

kern:	$(ASM_OUT)
