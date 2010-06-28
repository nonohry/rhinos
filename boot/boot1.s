[BITS 16]
	
jmp 	start

	fastmsg         db      'Checking BIOS fast A20 support ...',13,10,0
	fastokmsg       db      'BIOS fast A20 support found !',13,10,0
	nofastmsg       db      'Failed to enable A20 or no BIOS fast A20 support found',13,10,0
	a20msg          db      'A20 Gate enabled',13,10,0
	gdtmsg		db	'GDT Loaded',13,10,0
	memerrmsg	db	'Failed to get memory size',13,10,0
	bootdrv		db	0

boot_info:	
	mem_lower	dw	0
	mem_upper	dw	0
	
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
	KSIZE		equ	15	; Taille du noyau (KSIZE*512o)
	KSECTOR		equ	3	; Numero de secteur
	
	;;
	;; Chargement du Memory Manager
	;; 

	MMLOADOFF	equ	0x0	; Offset de chargement du mm
	MMSEGMENT	equ	0x3000 	; Segemnt de chargement du mm (pour obtenir 0x30000)
	MMSIZE		equ	10	; Taille du mm (MMSIZE*512o)
	MMSECTOR	equ	18	; Numero de secteur
	
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
	;; Point d'entree
	;;
	
start:
	mov	ax,CUROFF	; Simule une origine a 0
	mov	ds,ax		; alors que le boot
	mov	es,ax		; nous place a 0x800
	
	mov	[bootdrv],dl	; Recupere le bootdrive (sauvegarde par boot0)
	
	;;
	;;	Taille memoire selon int 0x15 (AX=0xE801)
	;; 
	
	mov	ax,0xE801	; Argument de l interruption
	int	0x15		; Interruption
	jc	get_mem_e801_err; Erreur si CF
	mov	[mem_upper],dx	; DX= memoire haute en nombre de blocs de 64K 
	mov	[mem_lower],cx	; CX = taille memoire basse en K
	jmp	get_mem_e801_end; Saut a la fin
get_mem_e801_err:
	mov	si,memerrmsg	; Charge le message d'erreur
	call    print_message	; Affiche le message
	call	reboot		; Reboot
get_mem_e801_end:

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
	;; Chargement du Memory Manager 
	;;

 	mov	dl,[bootdrv]	; Le boot drive dans DL
	mov	bx,MMLOADOFF	; L'offset dans BX
	mov	ax,MMSEGMENT		; Le segment dans AX
	mov	byte [drv_size],MMSIZE		; La taille dans [drv_size]
	mov	word [drv_sect],MMSECTOR	; Le numero de secteur dans [drv_sect]
	call	load_sect	; Appelle la fonction de chargement
	
	;;
	;; Mise en place de la ligne A20
	;; 
	
	cli			; On coupe les interruptions
	mov	si,fastmsg	; Charge le message de Fast A20
	call	print_message	; et l'affiche
	mov 	ax,0x2403	; Interroge le BIOS
	int	0x15		; pour le support du Fast A20 Gate
	jc	no_fast		; Si CF=1, echec de l'interruption
	cmp	ah,0		; Si AH<>0
	jne	no_fast		; alors pas de support par le BIOS
	and	bx,2		; BX contient le type du support
	jz	no_fast		; Si 2eme bit de BX n'est pas mis, pas de fast A20
	mov	si,fastokmsg	; Sinon, le support est present
	call	print_message	; On l'affiche

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
	mov 	si,nofastmsg	; Affiche que le support du fast A20
	call 	print_message	; n'est pas present
	call	reboot		; Reboot

fast_ok:
	sti			; Restaure les interruptions
	mov	si,a20msg	; Sinon, l'activation a reussi
	call	print_message	; On l'affiche

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
	