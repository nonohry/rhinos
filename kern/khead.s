[BITS 32]


extern	bochs_print
extern	cstart			; Fonction d'initialisation en C
extern  gdt_desc		; Descripteur de la GDT en C
extern  idt_desc		; Descripteur de l'IDT en C
extern	irq_handle		; Handlers pour les IRQ en C
extern	irq_active		; Tableau des ISR actives en C
extern	excep_handle		; Handlers pour les exceptions en C
extern	tss			; TSS defini en C
extern	proc_current		; Pointeur sur le processus a executer
extern  main			; RhinOS Main en C

global	hwint_00		; ISR visibles pour le C
global	hwint_01
global	hwint_02
global	hwint_03
global	hwint_04
global	hwint_05
global	hwint_06
global	hwint_07
global	hwint_08
global	hwint_09
global	hwint_10
global	hwint_11
global	hwint_12
global	hwint_13
global	hwint_14
global	hwint_15
global	excep_00
global	excep_01
global	excep_02
global	excep_03
global	excep_04
global	excep_05
global	excep_06
global	excep_07
global	excep_08
global	excep_09
global	excep_10
global	excep_11
global	excep_12
global	excep_13
global	excep_14
global	excep_16
global	excep_17
global	excep_18

global	task_mgmt		; Gestion des taches
	
start:
	push	pmodemsg	; Empile le message
	call	bochs_print	; Affiche dans Bochs
	add	esp,4		; Depile le message
	call	cstart
	
	lgdt	[gdt_desc]	; Charge la nouvelle GDT
    	lidt	[idt_desc]	; Charge l IDT
	jmp	next		; Vide les caches processeurs
	
next:	

	mov     ax,DS_SELECTOR
	mov     ds,ax  	 	; Reinitialisation
	mov     ax,ES_SELECTOR	;
	mov     es,ax   	; des registres
	mov     ax,SS_SELECTOR	;
	mov     ss,ax   	; de segments (ESP invariable)
 	sti			; Restaure les interruptions

	mov	ax,TSS_SELECTOR	; Index du TSS
	ltr	ax		; Charge le task register

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
	;; Sauvegarde du contexte pour les IRQ et les exceptions
	;;

excep_save:	
hwint_save:
	cld		        ; Positionne le sens d empilement
	pushad			; Sauve les registres generaux 32bits
	mov	eax,esp		; Sauve la position de ESP pour le retour
	o16 push	ds	; Sauve les registres de segments (empile en 16bits)
	o16 push	es
	o16 push	fs
	o16 push	gs
	mov	esp,kstack_top 	;Positionne la pile noyau
	mov	dx,DS_SELECTOR	; Ajuste les segments noyau (CS & SS sont deja positionnes)
	mov	ds,dx
	mov	dx,ES_SELECTOR
	mov	es,dx		; note: FS & GS ne sont pas utilises par le noyau
 	push	task_mgmt	; Empile l'adresse de task_mgmt comme adresse de retour
	jmp	[eax+32]	; 32 = 8 registres (pusad) => jmp a l'adresse empilee par call


	;;
	;; Gestion des taches (s'acheve avec *_ret)
	;; 

task_mgmt:
	mov	esp,[proc_current] ; La pile pointe sur le contexte
 	lldt	[esp+PROC_LDT_SEL] ; Charge la LDT du processus courant
 	lea	eax,[esp+PROC_FRAME_END] ; Recupere l'adresse de la fin du stack_frame
 	mov	[tss+TSS_ESP0],eax ; Fait pointer ESP0 sur la fin du stack_frame
				   ; de ce fait, hwint_save, en pushant les registres, va en meme temps
				   ; les sauver dans le stack_frame du process
	;; 
	;; Restauration du contexte pour les IRQ et les exceptions
	;; 

excep_ret:	
hwint_ret:
	o16 pop gs		; Restaure les registres
	o16 pop fs		; sauves par hwint_save
	o16 pop	es		; en 16bits
	o16 pop	ds
	popad
	add	esp,4		; Ignore l'adresse empilee par le call de hwint_generic
	iretd			; Retour d'interruption


	;;
	;; Exceptions Handlers
	;; 

excep_00:
	push	DIVIDE_VECTOR
	jmp	excep_noerr

excep_01:
	push	DEBUG_VECTOR
	jmp	excep_noerr

excep_02:
	push	NMI_VECTOR
	jmp	excep_noerr

excep_03:
	push	BREAKPT_VECTOR
	jmp	excep_noerr

excep_04:
	push	OVERFLOW_VECTOR	
	jmp	excep_noerr

excep_05:	
	push	BOUND_VECTOR
	jmp	excep_noerr

excep_06:
	push	OPCODE_VECTOR	
	jmp	excep_noerr

excep_07:
	push	NOMATH_VECTOR
	jmp	excep_noerr
	
excep_08:
	push	DFAULT_VECTOR
	jmp	excep_err

excep_09:
	push	COSEG_VECTOR	
	jmp	excep_noerr	

excep_10:
	push	TSS_VECTOR	
	jmp	excep_err

excep_11:
	push	NOSEG_VECTOR
	jmp	excep_err	

excep_12:
	push	SSFAULT_VECTOR	
	jmp	excep_err

excep_13:
	push	GPROT_VECTOR	
	jmp	excep_err

excep_14:
	push	PFAULT_VECTOR	
	jmp	excep_err
	
excep_16:
	push	MFAULT_VECTOR
	jmp	excep_noerr	

excep_17:
	push	ALIGN_VECTOR
	jmp	excep_err

excep_18:
	push	MACHINE_VECTOR	
	jmp	excep_noerr	

	
	;;
	;; Gestion des exceptions sans code d erreur
	;; 

excep_noerr:

	mov	dword [excep_code],0	; Cree un faux code d erreur
	pop	dword [excep_num]	; Recupere le vecteur
	jmp	excep_err_next		; Saute a la gestion avec erreur

	;;
	;; Gestion des exceptions avec code erreur
	;; 

excep_err:
	pop	dword [excep_num] 	; Recupere le vecteur
	pop	dword [excep_code]	; Recupere le code d erreur (empile par le proc)	
excep_err_next:	
	call	excep_save	; Sauve le contexte
	push	dword [excep_code]	; Argument 2 de excep_handle
	push	dword [excep_num]	; Argument 1 de excep_handle
	call	excep_handle	; Gestion de l exception en C
	add	esp,2*4		; Depile les arguments
	ret			; Retourne a excep_ret !
	
	;;
	;; Declaration des Donnees
	;;

	pmodemsg 	db	'Protected Mode enabled !',13,10,0
	excep_code	dd	0 ; Code Erreur des exceptions
	excep_num	dd	0 ; Vecteur de l exception
	;; 
	;; Segment Selector
	;;

	CS_SELECTOR	equ	8  ; CS  = 00000001  0  00   = (byte) 8
	DS_SELECTOR	equ	16 ; DS  = 00000010  0  00   = (byte) 16
	ES_SELECTOR	equ	24 ; ES  = 00000011  0  00   = (byte) 24
	SS_SELECTOR	equ	32 ; SS  = 00000100  0  00   = (byte) 32
	TSS_SELECTOR    equ     40 ; TSS = 00000101  0  00   = (byte) 40
	
	;;
	;; IRQ Magic Numbers
	;;

	IRQ_EOI		equ	0x20
	IRQ_MASTER	equ	0x20
	IRQ_SLAVE	equ	0xA0

	;;
	;; Vecteurs Exceptions
	;;

	DIVIDE_VECTOR	equ	0
	DEBUG_VECTOR	equ	1
	NMI_VECTOR	equ	2
	BREAKPT_VECTOR	equ	3
	OVERFLOW_VECTOR	equ	4
	BOUND_VECTOR	equ	5
	OPCODE_VECTOR	equ	6
	NOMATH_VECTOR	equ	7
	DFAULT_VECTOR	equ	8
	COSEG_VECTOR	equ	9
	TSS_VECTOR	equ	10
	NOSEG_VECTOR	equ	11
	SSFAULT_VECTOR	equ	12
	GPROT_VECTOR	equ	13
	PFAULT_VECTOR	equ	14
	MFAULT_VECTOR	equ	16
	ALIGN_VECTOR	equ	17
	MACHINE_VECTOR	equ	18

	;;
	;; Offset dans la structure proc
	;;

	PROC_LDT_SEL	equ	64 ; Position, en octet, du selecteur
	PROC_FRAME_END	equ	64 ; Position, en octet, du dernier element de la stack_frame (SS)
	;;
	;; Offset dans le TSS
	;;

	TSS_ESP0	equ	4 ; Position, en octet, de ESP0

	;;
	;; Donnees de la pile noyau
	;;

	KERN_STACK_SIZE	equ	1024 ; Pile de 1024 octets


	;;
	;; La pile noyau pour gerer les interruptions
	;;

kstack:
	resb	KERN_STACK_SIZE
kstack_top:
	