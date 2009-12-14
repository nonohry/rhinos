[BITS 16]

	
jmp	start			; Evite les declarations des donnees & fonctions
	
	;; 
	;; Declaration des donnees
	;;

	bootmsg		db	'Booting RhinOS ...',13,10,0
	cpumsg		db	'Checking CPU type ...',13,10,0
	i8086msg	db	'8086 CPU found, need 80386+ CPU',13,10,0
	i80286msg	db	'80286 CPU found, need 80386+ CPU',13,10,0
	i80386msg	db	'80386 CPU found !', 13,10,0
	bootdrv		db	0

	LOADOFF		equ	0x7E0 	; Offset du second programme de boot
	LOADSIZE	equ	2	; Taille (en secteur de 512o)
	CUROFF		equ	0x7C0 	; Offset Courant
	STACKPTR	equ	0x1C00 	; Offset SP
	STACKOFF	equ	0x600 	; Base de la pile
	
	;;
	;; Fonction Utiles
	;;
	%include "boot.inc"


cpu_type:
        pushf                   ; Sauve FLAGS sur la pile
        xor     ax,ax           ; Vide AX
        push    ax              ; Empile AX
        popf                    ; Pop AX dans FLAGS, FLAGS est donc reset

        pushf                   ; Empile le FLAGS reset
        pop     ax              ; Le recupere dans AX
        and     ax,0x0F000      ; AND pour avoir la valeur du bit 15
        cmp     ax,0x0F000      ; Compare cette valeur
        je      cpu_86          ; Saute si le bit est a 1

        mov     ax,0x0F000      ; On va tester de la meme
        push    ax              ; maniere si on peut
        popf                    ; modifier les bits 13-14
        pushf                   ; Si on ne peut pas
        pop     ax              ; alors nous sommes en
        and     ax,0x0F000      ; presence d un
        jz      cpu_286         ; 80286

        mov     si,i80386msg    ; Sinon on a un 80386
        popf                    ; Restaure FLAGS
        call    print_message   ; On l'affiche
        ret                     ; On retourne
cpu_86:
        mov     si,i8086msg     ; On a un 8086/8088
        jmp     cpu_end
cpu_286:
        mov     si,i80286msg    ; On a un 30286
cpu_end:
        popf                    ; Restaure FLAGS
        call    print_message   ; Affiche le CPU non 386
        call    reboot          ; Reboot

	
	;;
	;; Code du bootsector
	;; 

start:
	mov	ax,CUROFF	; Simule une origine a 0
	mov	ds,ax		; alors que le BIOS
	mov	es,ax		; nous place a 0x7C00

	cli			; Bloque les interruptions
	mov	ax,STACKOFF	; L'adresse de le pile
	mov	ss,ax		; On la met dans SS
	mov	sp,STACKPTR	; Positionne SP
	sti			; Restaure les interruptions

	mov	[bootdrv],dl	; Recupere le lecteur de boot
	
	mov	si,bootmsg	; Charge le message de boot
	call	print_message	; et l'affiche

	mov	si,cpumsg	; Charge le message de CPU
	call	print_message	; et l'affiche
	call	cpu_type	; Detecte le type de CPU

	mov	dl,[bootdrv]	; Le boot drive dans DL
	mov	ax,LOADOFF	; Le segment dans AX
	mov	bx,0x0		; L'offset dans BX
	mov	ch,LOADSIZE	; La taille dans CH
	mov	cl,2		; Le numero de secteur dans CL
	call	load_sect	; Appelle la fonction de chargement

	jmp	LOADOFF:0x0	; Saute au second boot jmp	LOADOFF:0x0
	
	times	510-($-$$) db 0	; Padding pour atteinde 512 octets
	dw	0xAA55		; BIOS Magic Number