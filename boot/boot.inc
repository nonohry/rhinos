	;; 
	;; Fonction Utiles (mode reel)
	;;

	rebootmsg	db	'Press any key to reboot.',13,10,0
	readmsg		db	'Unable to read bootdrive',13,10,0
	bootdrive	db	0

	;;
	;; LBA to CHS
	;; 

	drv_cyl		db	0
	drv_head	db	0
	drv_sect	dw	0
	drv_size	db	0
	drv_one_sect	db	0

	;;
	;; Caracteristiques d une disquette
	;; 

	FLOPPY_HEAD	equ	2
	FLOPPY_SECT	equ	18
	
	;;
	;; print_message
	;; Affiche un message contenu dans SI
	;; 
	
print_message:
        lodsb                   ; AL=SI & SI=SI +/- 1
        or      al,al           ; Teste si AL=0 ie Fin de chaine
        jz      print_end       ; Fin si AL=0
        mov     ah,0x0E         ; Argument de INT 0x10 (print tty char)
        int     0x10            ; Interruption
        jmp     print_message   ; Boucle
print_end:
        ret

	;;
	;; wait_key
	;; Attend une touche utilisateur
	;; 
	
wait_key:
        mov     ah,0x00         ; Argument de INT 0x16 (get keystroke - blocking)
        int     0x16            ; Interruption
        ret

	;;
	;; Load_sect
	;; Charge en memoire des secteurs un par un depuis un lecteur
	;; AX=Segment BX=Offset [drv_size]=Size [drv_sect]=Secteur DL=bootdrv
	;;

load_sect:
	;; LBA to CHS
	push 	ax		; Sauvegarde les registres essentiels
	push	cx
	push	dx	
	xor	ax,ax		; Vide les registres
	xor	cx,cx
	xor	dx,dx
	
	mov	ax,word [drv_sect] ; AX = LBA
	mov	cx,FLOPPY_SECT	   ; CX = FLOPPY_SECT
	div	cx		   ; Divise Dx:AX par CX
	inc	dl		   ; Incremente le reste
	mov	byte [drv_one_sect],dl ; Secteur = le reste + 1
	xor	dx,dx		       ; Nettoie DX
	mov	cx,FLOPPY_HEAD	   ; CX = FLOPPY_HEAD
	div	cx		   ; Divise DX:AX (quotient de la premier DIV) par CX
	mov	byte [drv_cyl],al  ; Cylindre = Quotient
	mov	byte [drv_head],dl ; Tete = Reste
	

	pop	dx		; Restaure les registres
	pop	cx
	pop	ax
	
	push	ax		; Sauvegarde les registres essentiels
	call	load_one_sect	; Appelle le chargement un par un 
	pop	ax		; Restaure les registres
	inc	word [drv_sect]	; Incremente le numero de secteur LBA
	add	bx,0x200	; Incremente de 512o la destination	
	dec	byte [drv_size]	; Decrement la taille
	jnz	load_sect	; On boucle si tout n'est pas charge
	ret
	
load_one_sect:	
	
	push 	es		; Sauvegarde ES
	push	di		; Sauvegarde DI
	mov	[bootdrive],dl	; Sauvegarde du bootdrive
	mov	di,0x3		; 3 essais de lecture du boot drive
	
load_retry:	
	mov 	es,ax		; ES:BX est la zone de chargement
	mov	ah,0x02		; Argument de INT 0x13
	mov	al,0x01		; Taille a charger (dans CH)
	mov	cl,byte [drv_one_sect] 	; Secteur
	mov	ch,byte [drv_cyl]	; Cylindre 
	mov	dh,byte [drv_head]	; Tete 
	int	0x13		; INT 13
	jnb	load_ok		; Interruption reussie
	dec	di		; Decremente DI sinon
	jl 	load_fail	; Si DL<0, on saute a load_fail
	
	xor 	ah,ah		; Sinon, mise a zero de AH (arg de INT 0x13)
	mov	dl,[bootdrive]	; Le boot drive dans DL
	int	0x13		; Reset la tete de lecture via la BIOS
	jnb	load_retry	; On reessaie
	
load_ok:
	pop	di		; Restaure DI
	pop	es		; Restaure ES
	ret			; Retourne

load_fail:
	pop	di		; Restaure DI
	pop	es		; Restaure ES
	mov	si,readmsg	; Charge le message d'erreur
	call	print_message	; Et l'affiche
	call	reboot		; Reboot


	;;
	;; reboot
	;; Redemarre :-)
	;; 
	
reboot:
        mov     si,rebootmsg    ; Charge le message de reboot
        call    print_message   ; et l'affiche
        call    wait_key        ; Attend une touche utilisateur
        jmp     0xFFFF:0x0000   ; Saut a la procedure de reset du BIOS
