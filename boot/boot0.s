	;;
	;; Boot0.s
	;; RhinOS Bootsector
	;;
	
[BITS 16]

	;; 
	;;  Recopie du bootsector a un meilleur endroit
	;; 
	
	SRCADDR		equ	0x7C00
	DSTADDR		equ	0x600
	
	cld			; Fixe le sens du decompte
	mov  	esi,SRCADDR	; Recupere l addresse source
	mov	edi,DSTADDR	; Recupere l addresse de destination
	mov	ecx,512 	; Recupere la taille (512o)
	shr	ecx,0x2		; Divise la taille par 4
	rep movsd		; Copie !
	jmp	0:DSTADDR+start ; Saute au nouveau point d entree
	
	;; 
	;; Declaration des donnees
	;;

	bootmsg		db	'Booting RhinOS ...',13,10,0
	noi80386msg	db	'No 80386 CPU found',13,10,0
	i80386msg	db	'80386 CPU found !', 13,10,0
	bootdrv		db	0

	LOADOFF		equ	0x800 	; Offset du second programme de boot
	LOADSEG		equ	0x0	; Segment du second programme de boot
	LOADSIZE	equ	2	; Taille (en secteur de 512o)
	CUROFF		equ	0x60 	; Offset Courant
	STACKPTR	equ	0x1C00 	; Offset SP
	STACKOFF	equ	0x600 	; Base de la pile
	
	;;
	;; Fonctions Utiles
	;;
	
	%include "boot.inc"
	%include "boot0.inc"
	
	;;
	;; Code du bootsector
	;; 

start:
	mov	ax,CUROFF	; Simule une origine a 0
	mov	ds,ax		; alors que la copie
	mov	es,ax		; nous place a 0x600

	cli			; Bloque les interruptions
	mov	ax,STACKOFF	; L'adresse de le pile
	mov	ss,ax		; On la met dans SS
	mov	sp,STACKPTR	; Positionne SP
	sti			; Restaure les interruptions

	mov	[bootdrv],dl	; Recupere le lecteur de boot
	
	mov	si,bootmsg	; Charge le message de boot
	call	print_message	; et l'affiche

	;; 
	;; S'assure du bon CPU
	;; 
	
	call	cpu_type	; Detecte le type de CPU
	cmp	ax, CPU_SUCCESS	; Teste la valeur de retour
	je	cpu_ok		; Si la valeur est bonne on continue
	mov	si,noi80386msg	; Message pour le CPU non trouve
	call	print_message	; et l'affiche
	call	reboot		; pui on reboot

cpu_ok:
	mov	si,i80386msg	; Message pour le CPU trouve
	call	print_message	; et l'affiche

	;;
	;; Charge le second programme de boot
	;; 
	
	mov	dl,[bootdrv]	; Le boot drive dans DL
	call	get_geometry	; Recupere la geometrie du disque de boot
	

	mov	dl,[bootdrv]	; Le boot drive dans DL
	mov	bx,LOADOFF	; L'offset dans BX
	mov	ax,LOADSEG	; Le segment dans AX
	mov	byte [drv_size],LOADSIZE	; La taille dans [drv_size]
	mov	word [drv_sect],1		; Le numero de secteur LBA dans [drv_sect]
	call	load_sect	; Appelle la fonction de chargement
	
	jmp	0x0:LOADOFF	; Saute au second boot jmp	LOADOFF:0x0
	
	times	510-($-$$) db 0	; Padding pour atteinde 512 octets
	dw	0xAA55		; BIOS Magic Number