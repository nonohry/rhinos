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
ASM_INC = 	boot.inc boot0.inc boot1.inc

# Cibles

boot:	$(ASM_OUT) $(ASM_INC)

depend:
	makedepend -- -- $(ASM_SRC)
clean:
	$(RM) *~
	$(RM) $(ASM_OUT)
# DO NOT DELETE
