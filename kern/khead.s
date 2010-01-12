[BITS 32]


extern	bochs_print
extern	cstart			; Fonction d'initialisation en C
extern  gdt_desc		; Descripteur de la GDT en C
extern  idt_desc		; Descripteur de l'IDT en C
extern	irq_handle		; Handlers pour les IRQ en C
extern	irq_active		; Tableau des ISR actives en C
extern	irq_disable		; Desactive une ligne IRQ en C
extern  main			; RhinOS Main en C
	
	
start:
	push	pmodemsg	; Empile le message
	call	bochs_print	; Affiche dans Bochs
	add	esp,4		; Depile le message
	call	cstart
	
	lgdt	[gdt_desc]
	jmp	next
	
next:	

	mov     ax,DS_SELECTOR
	mov     ds,ax  	 	; Reinitialisation
	mov     ax,ES_SELECTOR	;
	mov     es,ax   	; des registres
	mov     ax,SS_SELECTOR	;
	mov     ss,ax   	; de segments (ESP invariable)
	
	jmp	CS_SELECTOR:main


	;;
	;; Traitement generique des IRQ (macro maitre)
	;;
	
%macro	hwint_generic0	1
	call	hwint_save	; Sauvegarde des registres
	push	%1		; Empile l'IRQ
	call	irq_handle	; Appel les handles C
	add	esp,4		; Depile l'IRQ
	cmp	dword [irq_active+4*%1],0 ; Verifie qu'aucune ISR n est active
	je	%%noactive0	; Saute a l EOI si pas d ISR active
	in	al,IRQ_MASTER+1	; Lit OCW1
	mov	bl,1		; Prepare le masque
	shl	bl,%1		; %1 ieme bit mis a 1
	or	al,bl		; L'IRQ %1 est masquee
	out	IRQ_MASTER+1,al	; Envoie OCW1
%%noactive0:
	mov	al,IRQ_EOI	; Envoi la fin d interruption
	out	IRQ_MASTER,al	; au PIC Maitre
	ret			; Retourne a hwint_ret !
%endmacro

	;;
	;; Traitement generique des IRQ (macro esclave)
	;;

%macro	hwint_generic1	1
	call	hwint_save	; Sauvegarde des registres
	push	%1		; Empile l'IRQ 
	call	irq_handle	; Appel les handles C
	add	esp,4		; Depile l'IRQ
	cmp	dword [irq_active+4*%1],0 ; Verifie qu'aucune ISR n est active
	je	%%noactive1	; Saute a l EOI si pas d ISR active
	in	al,IRQ_SLAVE+1	; Lit OCW1
	mov	bl,1		; Prepare le masque
	shl	bl,%1		; %1 ieme bit mis a 1
	or	al,bl		; L'IRQ %1 est masquee
	out	IRQ_SLAVE+1,al	; Envoie OCW1
%%noactive1:
	mov	al,IRQ_EOI	; Envoi la fin d interruption
	out	IRQ_SLAVE,al	; au PIC Esclave
	out	IRQ_MASTER,al	; puis au Maitre
	ret			; Retourne a hwint_ret !
%endmacro

	;;
	;; IRQ handlers
	;; 

hwint_00:
	hwint_generic0	0

hwint_01:
	hwint_generic0	1

hwint_02:
	hwint_generic0	2

hwint_03:
	hwint_generic0	3
	
hwint_04:
	hwint_generic0	4

hwint_05:
	hwint_generic0	5

hwint_06:
	hwint_generic0	6

hwint_07:
	hwint_generic0	7

hwint_08:
	hwint_generic1	8

hwint_09:
	hwint_generic1	9

hwint_10:
	hwint_generic1	10

hwint_11:
	hwint_generic1	11

hwint_12:
	hwint_generic1	12

hwint_13:
	hwint_generic1	13

hwint_14:
	hwint_generic1	14

hwint_15:
	hwint_generic1	15
	
	;;
	;; Sauvegarde du contexte pour les IRQ
	;;

hwint_save:
	pushad			; Sauve les registres generaux 32bits
	mov	eax,esp		; Sauve la position de ESP pour le retour
	push	ds		; Sauve les registres de segments
	push	es
	push	fs
	push	gs
	mov	dx,DS_SELECTOR	; Ajuste les segments noyau (CS & SS sont deja positionnes)
	mov	ds,dx
	mov	dx,ES_SELECTOR
	mov	es,dx		; note: FS & GS ne sont pas utilises par le noyau
	push	hwint_ret	; Empile l'adresse de hwint_ret comme adresse de retour
	jmp	[eax+32]	; 32 = 8 registres (pusad) => jmp a l'adresse empilee par call

	;;
	;;Restauration du contexte pour les IRQ
	;; 
	
hwint_ret:
	pop	gs		; Restaure les registres
	pop	fs		; sauves par hwint_save
	pop	es
	pop	ds
	popad
	add	esp,4		; Ignore l'adresse empilee par le call de hwint_generic
	iret			; Retour d'interruption

	
	;;
	;; Declaration des Donnees
	;;

	pmodemsg 	db	'Protected Mode enabled !',13,10,0
	savemsg		db	'message from hwint_save',13,10,0
	retmsg		db	'message from hwint_ret',13,10,0
	hwintmsg	db	'message from hwint_01',13,10,0
	
	;; 
	;; Segment Selector
	;;

	CS_SELECTOR	equ	8  ; CS = 00000001  0  00   = (byte) 8
	DS_SELECTOR	equ	16 ; DS = 00000010  0  00   = (byte) 16
	ES_SELECTOR	equ	24 ; ES = 00000011  0  00   = (byte) 24
	SS_SELECTOR	equ	32 ; SS = 00000100  0  00   = (byte) 32

	;;
	;; IRQ Magic Numbers
	;;

	IRQ_EOI		equ	0x20
	IRQ_MASTER	equ	0x20
	IRQ_SLAVE	equ	0xA0