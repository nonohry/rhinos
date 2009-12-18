[BITS 32]
[ORG 0x8200]

jmp	start

	pmodemsg	db	'Protected mode enabled !',13,10,0

	;;
	;; Affichage dans Bochs
	;; 
	
bochs_print:	
	lodsb			; Identique a print_message
	cmp	al,0		; Fin de chaine ?
	je	bochs_end	; On retourne si oui
	mov	dx,0xe9		; Port de Bochs
	out	dx,al		; Emet le characterfe courant
	jmp	bochs_print	; Boucle
bochs_end:
	ret

start:
	mov	esi,pmodemsg
	call	bochs_print
hang:
	jmp 	hang
