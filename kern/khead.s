[BITS 32]


global	_bochs_print
extern	cstart

	;;
	;; Point d'Entree
	;; 

start:
	push	pmodemsg
	call	_bochs_print
	call	cstart

hang:
	jmp 	hang

	;;
	;; Affichage dans Bochs
	;; 
	
_bochs_print:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	push	esi		; Sauvegarde ESI (Requis par GCC)
	push	edi		; Sauvegarde EDI (Requis par GCC)
	mov  	esi,[ebp+8]	; Recupere l'argument dans ESI
bochs_loop:	
	lodsb			; Identique a print_message
	cmp	al,0		; Fin de chaine ?
	je	bochs_end	; On retourne si oui
	mov	dx,0xe9		; Port de Bochs
	out	dx,al		; Emet le caractere courant
	jmp	bochs_loop	; Boucle
bochs_end:
	pop	edi		; Restaure EDI
	pop	esi		; Restaure ESI
	mov	ebp,esp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret


	;;
	;; Declaration des Donnees
	;;

	pmodemsg 	db	'Protected Mode enabled !',13,10,0	