[BITS 32]

global bochs_print
global outb
	
	;; 
	;; Affichage dans bochs
	;; 
	
bochs_print:
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
	;; outb
	;;

outb:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	push	esi		; Sauvegarde ESI (Requis par GCC)
	push	edi		; Sauvegarde EDI (Requis par GCC)
	mov  	dx,[ebp+8]	; Recupere le port dans dx
	mov     al,[ebp+12]     ; Recupere la valeur dans al
	out     dx,al		; instruction out
	pop	edi		; Restaure EDI
	pop	esi		; Restaure ESI
	mov	ebp,esp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret
