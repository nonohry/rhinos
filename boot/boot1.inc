	;;
	;; Boot1.inc
	;; Fonctions utilitaires pour boot1.s
	;;

	;;
	;; Structure bootinfo
	;;

boot_info:
	kern_start	dd	0
	kern_end	dd	0
	mem_lower	dw	0
	mem_upper	dw	0

	;;
	;; Conversion adressage reel en physique
	;;
	
real2phys:			; Arg: DX=Segment, AX=Offset	
	mov 	cl,0x04		; Nb de decalage
	mov 	ch,dh		; Sauvegarde de DH
	shr 	ch,cl		; Ne garde que le 2nd octet de DH ds CH
	shl 	dx,cl		; Decale DX (Multiplie par 16)
	add 	ax,dx		; Addition avec AX
	adc 	ch,0x00		; Addition de la retenue (si elle existe)
	mov 	dl,ch		; Met le resultat dans DL
	xor 	dh,dh		; Vide DH
	ret			; DH-AX contient l'adresse physique sur 20bits

	;;
	;;	Taille memoire selon int 0x15 (AX=0xE801)
	;;

	MEM_SUCCESS	equ	0x01
	MEM_FAIL	equ	0x00
	
get_mem_size:	
	mov	ax,0xE801	; Argument de l interruption
	int	0x15		; Interruption
	jc	get_mem_e801_err; Erreur si CF
	mov	[mem_upper],dx	; DX= memoire haute en nombre de blocs de 64K 
	mov	[mem_lower],cx	; CX = taille memoire basse en K
	xor	ax,ax		; Reset AX
	mov	ax, MEM_SUCCESS	; Valeur de retour
	jmp	get_mem_e801_end; Saut a la fin
get_mem_e801_err:
	xor	ax,ax		; Reset AX
	mov	ax, MEM_FAIL	; Valeur de retour
get_mem_e801_end:
	ret			; Retour

	;;
	;; A20 Gate
	;;

	A20_SUCCESS	equ	0x01
	A20_FAIL	equ	0x00

set_a20:	
	cli			; On coupe les interruptions
	mov 	ax,0x2403	; Interroge le BIOS
	int	0x15		; pour le support du Fast A20 Gate
	jc	no_fast		; Si CF=1, echec de l'interruption
	cmp	ah,0		; Si AH<>0
	jne	no_fast		; alors pas de support par le BIOS
	and	bx,2		; BX contient le type du support
	jz	no_fast		; Si 2eme bit de BX n'est pas mis, pas de fast A20

	mov 	ax,0x2402	; Appel du BIOS
	int	0x15		; pour determiner l'etat A20 Gate
	jc	no_fast		; Si CF=1, echec de l'interruption
	cmp	ah,0		; Si AH<>0
	jne	no_fast		; alors echec de l'interrogation
	cmp	al,0		; Si A20 Gate est deja activee
	jne	fast_ok		; on saute au message
	mov 	ax,0x2401	; Sinon, on appelle le BIOS
	int	0x15		; pour activer A20 Gate
	jc	no_fast		; Si CF=1, echec de l'interruption
	cmp	ah,0		; Si AH=0
	je	fast_ok		; alors reussite de l'activation

no_fast:	
	sti			; Restaure les interruptions
	xor	ax,ax		; Reset AX
	mov	ax, A20_FAIL	; Valeur de retour
	ret			; Retourne

fast_ok:
	sti			; Restaure les interruptions
	xor	ax,ax		; Reset AX
	mov	ax, A20_SUCCESS	; Valeur de retour
	ret			; Retourne
