[BITS 32]


extern	bochs_print
extern	cstart			; Fonction d'initialisation en C
extern  gdt_desc		; Descripteur de la GDT en C
extern  idt_desc		; Descripteur de l'IDT en C
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
	;; Declaration des Donnees
	;;

	pmodemsg 	db	'Protected Mode enabled !',13,10,0

	;; 
	;; Segment Selector
	;;

	CS_SELECTOR	equ	8  ; CS = 00000001  0  00   = (byte) 8
	DS_SELECTOR	equ	16 ; DS = 00000010  0  00   = (byte) 16
	ES_SELECTOR	equ	24 ; ES = 00000011  0  00   = (byte) 24
	SS_SELECTOR	equ	32 ; SS = 00000100  0  00   = (byte) 32