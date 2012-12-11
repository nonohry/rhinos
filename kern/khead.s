[BITS 32]

	;;;;;;;;;;;;;;;;;;;;;;;;
	;; Portee des Fonctions
	;;;;;;;;;;;;;;;;;;;;;;;;

	
extern	klib_bochs_print
extern	start_main		; Fonction d'initialisation en C
extern  gdt_desc		; Descripteur de la GDT en C
extern  idt_desc		; Descripteur de l'IDT en C
extern  main			; RhinOS Main en C
extern	kernel_start		; Debut du Noyau (issu de l edition des liens)	
extern	kernel_end		; Fin du Noyau (issu de l edition des liens)
	
global _start			; Point d'entree pour le lien
	



	jmp _start

	
        ;;;;;;;;;;;;;;;;;;;;;
	;; Multiboot header
        ;;;;;;;;;;;;;;;;;;;;;

	align 4

	dd	MULTIBOOT_MAGIC
	dd	MULTIBOOT_FLAGS
	dd	-(MULTIBOOT_MAGIC+MULTIBOOT_FLAGS)
	

	;;;;;;;;;
	;; Code
	;;;;;;;;;

	
_start:
	push	ebx
	push 	eax
	call	start_main	; Appelle start_main
	add 	esp,4
	
	lgdt	[gdt_desc]	; Charge la nouvelle GDT
    	lidt	[idt_desc]	; Charge l IDT

	mov	ax,TSS_SELECTOR ; Charge le TSS
	ltr	ax
	
	jmp	next		; Vide les caches processeurs

next:
	mov     ax,DS_SELECTOR
	mov     ds,ax  	 	; Reinitialisation
	mov     ax,ES_SELECTOR	;
	mov     es,ax   	; des registres
	mov	fs,ax		;
	mov	gs,ax		; de
	mov     ax,SS_SELECTOR	;
	mov     ss,ax   	; segments
	mov	esp,kstack_top	;
 
	jmp	CS_SELECTOR:main


	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;; Declaration des Donnees
	;;;;;;;;;;;;;;;;;;;;;;;;;;;



	pmodemsg 	db	'Protected Mode enabled !',13,10,0


	;;
	;; Multiboot
	;; 

	MULTIBOOT_MAGIC	equ	0x1BADB002
	MULTIBOOT_FLAGS	equ	0x3 ; Modules aligne sur 4K + Memoire - Video Mode - Bin Headers
	
	;; 
	;; Segment Selector
	;;

	CS_SELECTOR	equ	8  ; CS  = 00000001  0  00   = (byte) 8
	DS_SELECTOR	equ	16 ; DS  = 00000010  0  00   = (byte) 16
	ES_SELECTOR	equ	16 ; ES  = 00000010  0  00   = (byte) 16
	SS_SELECTOR	equ	16 ; SS  = 00000010  0  00   = (byte) 16

	;;
	;; Tss Selector
	;; 

	
	TSS_SELECTOR	equ 	40 ; TSS = 0000000000101  0  00   =  40 */
	
	;;
	;; Donnees de la pile noyau
	;;

	KERN_STACK_SIZE	equ	1024 ; Pile de 1024 octets


	;;
	;; La pile noyau
	;;

kstack:
	times	KERN_STACK_SIZE	db 0
kstack_top:
	