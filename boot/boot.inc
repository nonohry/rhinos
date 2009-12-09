	;; 
	;; Fonction Utiles (mode reel)
	;;

	rebootmsg	db	'Press any key to reboot.',13,10,0
	readmsg		db	'Unable to read bootdrive',13,10,0
	bootdrive	db	0
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
	;; Charge en memoire des secteurs depuis un lecteur
	;; AX=Segment BX=Offset CH=Size CL=Secteur DL=bootdrv
	;;
	
load_sect:	
	
	push 	es		; Sauvegarde ES
	mov	[bootdrive],dl	; Sauvegarde du bootdrive
	mov	di,0x3		; 3 essais de lecture du boot drive
	
load_retry:	
	mov 	es,ax		; ES:BX est la zone de chargement
	mov	ah,0x02		; Argument de INT 0x13
	mov	al,ch		; Taille a charger (dans CH)
	mov	ch,0x00		; Cylindre 0
	mov	dh,0x00		; Tete 0
	int	0x13		; INT 13
	jnb	load_ok		; Interruption reussie
	dec	di		; Decremente DI sinon
	jl 	load_fail	; Si DL<0, on saute a load_fail
	
	xor 	ah,ah		; Sinon, mise a zero de AH (arg de INT 0x13)
	mov	dl,[bootdrive]	; Le boot drive dans DL
	int	0x13		; Reset la tete de lecture via la BIOS
	jnb	load_retry	; On reessaie
	
load_ok:	
	pop	es		; Restaure ES
	ret			; Retourne

load_fail:
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
