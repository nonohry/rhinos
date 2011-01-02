	;;
	;; Boot1.s
	;; Second programme de boot
	;; Passage en mode reel
	;; 

[BITS 16]
	
jmp 	start

	fastokmsg       db     	'A20 Gate enabled',13,10,0
	nofastmsg       db      'Failed to enable A20 or no BIOS fast A20 support found',13,10,0
	gdtmsg		db	'Boot GDT Loaded',13,10,0
	memerrmsg	db	'Failed to get memory size',13,10,0
	bootdrv		db	0
	
	;; 
	;; Segment Selector
	;;

	CS_SELECTOR	equ	8  ; CS = 00000001  0  00   = (byte) 8
	DS_SELECTOR	equ	16 ; DS = 00000010  0  00   = (byte) 16
	ES_SELECTOR	equ	24 ; ES = 00000011  0  00   = (byte) 24
	SS_SELECTOR	equ	32 ; SS = 00000100  0  00   = (byte) 32

	;;
	;; Chargement du Noyau
	;; 
	
	KLOADOFF	equ	0xC00	; Offset de chargement du noyau
	KSEGMENT	equ	0x0	; Segment de chargment du noyau
	KSIZE		equ	17	; Taille du noyau (KSIZE*512o)
	KSECTOR		equ	3	; Numero de secteur
	
	;;
	;; Autres
	;; 

	CUROFF		equ	0x80	; Offset courant
	STACKPTR	equ	0x7C00	; Pointeur de pile
	
	;;
	;; GDT  
	;; 

gdt_desc:
	dw	0x27		    ; 0x27 = 39 = 8*5-1
	db 	0x00,0x00,0x00,0x00 ; Adresse physique de gdt a calculer

gdt:
null_desc:
	db	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
cs_desc:	
	db 	0xFF,0xFF,0x00,0x00,0x00,0x9A,0xCF,0x00
ds_desc:
	db 	0xFF,0xFF,0x00,0x00,0x00,0x92,0xCF,0x00
es_desc:
	db 	0xFF,0xFF,0x00,0x00,0x00,0x92,0xCF,0x00
ss_desc:
	db 	0xFF,0xFF,0x00,0x00,0x00,0x92,0xCF,0x00

	;;
	;; Fonction Utiles
	;;
	
	%include "boot.inc"
	%include "boot1.inc"

	;; 
	;; Point d'entree
	;;
	
start:
	mov	ax,CUROFF	; Simule une origine a 0
	mov	ds,ax		; alors que le boot
	mov	es,ax		; nous place a 0x800
	
	mov	[bootdrv],dl	; Recupere le bootdrive (sauvegarde par boot0)
	
	;;
	;; Recupere la taille de la memoire
	;; 
	call	get_mem_size
	cmp  	ax, MEM_SUCCESS
	je   	mem_ok
	mov	si,memerrmsg
	call	print_message	
mem_ok:	
	
	;;
	;; Geometrie du disque
	;;
	mov	dl,[bootdrv]	; Le boot drive dans DL
	call	get_geometry	; Recupere la geometrie

	;;
	;; Chargement du Noyau a KLOADOFF
	;; 

 	mov	dl,[bootdrv]	; Le boot drive dans DL
	mov	bx,KLOADOFF	; L'offset dans BX
	mov	ax,KSEGMENT		; Le segment dans AX
	mov	byte [drv_size],KSIZE	; La taille dans [drv_size]
	mov	word [drv_sect],KSECTOR	; Le numero de secteur dans [drv_sect]
	call	load_sect	; Appelle la fonction de chargement
	
	;;
	;; Mise en place de la ligne A20
	;;
	
	call	set_a20		; Appel de la fonction
	cmp	ax, A20_SUCCESS	; Test du retour
	je	a20_ok		; On continue si tout est bon
	mov	si, nofastmsg	; Sinon on charge le message
	call	print_message	; et on l'affiche
	call	reboot		; puis on reboot
a20_ok:	
	mov	si, fastokmsg	; Charge la bonne nouvelle
	call	print_message	; et l'affiche
	
	;;
	;; Mise en place de la GDT
	;;

	mov	dx,ds		; Segment = DS
	mov	ax,gdt		; Offset = gdt
	call	real2phys	; Calcule l'adresse physique de gdt
	mov	word [gdt_desc+2],ax	; Affecte l'adresse dans notre
	mov	byte [gdt_desc+4],dl	; descripteur desc_gdt
	cli			; Inhibe les interruptions
	lgdt	[gdt_desc]	; Charge la GDT
	sti			; Restaure les interruptions

	mov	si,gdtmsg	; Affiche le chargement
	call	print_message	; de la GDT

	mov	dx,ds		; Segement = DS
	mov	ax,boot_info	; Offset = boot_info
	call	real2phys	; Calcule l'adresse physique de boot_info
	mov	cx, ax		; Sauve l adresse de la structure
	
	cli                     ; Inhibe les interruptions
	mov     eax,cr0		; Passe en mode protege
	or      ax,0x1		; via les registres de controle
	mov     cr0,eax		; du processeur

	jmp	next		; Vide les caches processeur en sautant

next:

	mov     ax,DS_SELECTOR
	mov     ds,ax  	 	; Reinitialisation
	mov     ax,ES_SELECTOR	;
	mov     es,ax   	; des registres
	mov     ax,SS_SELECTOR	;
	mov     ss,ax   	; de segments
	mov     esp,STACKPTR

	jmp	CS_SELECTOR:KLOADOFF 	; Saut vers les noyau
	