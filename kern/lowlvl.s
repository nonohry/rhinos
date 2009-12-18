[BITS 32]
[ORG 0x8200]

global	_bochs_print

segment .text
	
jmp	start

	;;
	;; Affichage dans Bochs
	;; 
	
_bochs_print:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	mov  	esi,[ebp+8]	; Recupere l'argument dans ESI
bochs_loop:	
	lodsb			; Identique a print_message
	cmp	al,0		; Fin de chaine ?
	je	bochs_end	; On retourne si oui
	mov	dx,0xe9		; Port de Bochs
	out	dx,al		; Emet le caractere courant
	jmp	bochs_loop	; Boucle
bochs_end:
	mov	ebp,esp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret

	;;
	;; Point d'Entree
	;; 

start:
	push	pmodemsg
	call	_bochs_print
hang:
	jmp 	hang

	
	;;
	;; Section de donnees
	;; 
	
segment .data

	pmodemsg	db	'Protected mode enabled !',13,10,0