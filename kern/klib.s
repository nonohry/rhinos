[BITS 32]

global bochs_print
global outb
global inb
global phys_copy
	
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
	;; outb(u16_t,u8_t)
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

	;;
	;; inb(u16_t,u8_t*)
	;;

inb:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	push	esi		; Sauvegarde ESI (Requis par GCC)
	push	edi		; Sauvegarde EDI (Requis par GCC)
	mov  	dx,[ebp+8]	; Recupere le port dans dx
	mov	ebx,[ebp+12]	; Recupere le buffer dans ebx
	in      al,dx		; Instruction in
	mov	byte [ebx],al	; Affecte la valeur
	pop	edi		; Restaure EDI
	pop	esi		; Restaure ESI
	mov	ebp,esp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret

	;;
	;; phys_copy(u32_t src, u32_t dest, u32_t len)

phys_copy:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	push	esi		; Sauvegarde ESI (Requis par GCC)
	push	edi		; Sauvegarde EDI (Requis par GCC)
	push	ecx		; Sauvegarde ECX
	cld			; Fixe le sens du decompte
	mov  	esi,[ebp+8]	; Recupere l addresse source
	mov	edi,[ebp+12]	; Recupere l addresse de destination
	mov	ecx,[ebp+16] 	; Recupere la taille
	shr	ecx,0x2		; Divise la taille par 4
	rep movsd		; Copie !
	pop	ecx		; Restaure ECX
	pop	edi		; Restaure EDI
	pop	esi		; Restaure ESI
	mov	ebp,esp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret	