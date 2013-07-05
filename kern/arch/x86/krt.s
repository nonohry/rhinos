	;;/**
	;;
	;; 	krt.s
	;; 	=======
	;;
	;; 	Kernel Run Time
	;;
	;;**/

	
	[BITS 32]

	;;/**
	;;
	;; 	Extern
	;; 	------
	;;
	;; 	C code needed:
	;;
	;;	- start_main	: C initialization of GDT and IDT
	;;	- gdt_desc	: GDT descriptor
	;;	- idt_desc	: IDT descriptor
	;; 	- main		: Kernel C main routine
	;; 
	;;**/
	
extern	start_main
extern  gdt_desc
extern  idt_desc
extern  main



	;;/**
	;;
	;; 	Global
	;; 	------
	;;
	;; 	_start	: Entry point for link editor
	;;
	;;**/
	
global _start
	



	jmp _start

	
        ;;/**
	;;
	;; 	Structure: Multibot header
	;; 	--------------------------
	;;
	;; **/

	align 4

	dd	MULTIBOOT_MAGIC
	dd	MULTIBOOT_FLAGS
	dd	-(MULTIBOOT_MAGIC+MULTIBOOT_FLAGS)
	

	;;/**
	;;
	;; 	Function: _start
	;; 	----------------
	;;
	;; 	Kernel entry point. Call start_main to initilize GDT & IDT.
	;; 	Then load GDT, IDT,  all the required selectors
	;; 	and set up the kernel stack before jumping to C main
	;;
	;;**/

	
_start:
	push	ebx
	push 	eax
	call	start_main
	add 	esp,8
	
	lgdt	[gdt_desc]
    	lidt	[idt_desc]

	mov	ax,TSS_SELECTOR
	ltr	ax
	
	jmp	next		; Clear processor cache

next:
	mov     ax,DS_SELECTOR
	mov     ds,ax  	 
	mov     ax,ES_SELECTOR
	mov     es,ax 
	mov	fs,ax
	mov	gs,ax
	mov     ax,SS_SELECTOR
	mov     ss,ax
	mov	esp,kstack_top
 
	jmp	CS_SELECTOR:main


	
	;;/**
	;;
	;; 	Constants: Multiboot relatives
	;; 	------------------------------
	;;
	;;**/

	MULTIBOOT_MAGIC	equ	0x1BADB002
	MULTIBOOT_FLAGS	equ	0x3 ; Modules aligne sur 4K + Memoire - Video Mode - Bin Headers
	
	;;/**
	;;
	;; 	Constants: Kernel selectors
	;; 	---------------------------
	;;
	;;**/

	CS_SELECTOR	equ	8  ; CS  = 00000001  0  00   = (byte) 8
	DS_SELECTOR	equ	16 ; DS  = 00000010  0  00   = (byte) 16
	ES_SELECTOR	equ	16 ; ES  = 00000010  0  00   = (byte) 16
	SS_SELECTOR	equ	16 ; SS  = 00000010  0  00   = (byte) 16
	TSS_SELECTOR	equ 	40 ; TSS = 0000000000101  0  00   =  40 */
	

	;;/**
	;;
	;; 	Constant: KERN_STACK_SIZE
	;; 	-------------------------
	;;
	;;**/

	KERN_STACK_SIZE	equ	1024 ; Pile de 1024 octets

	
	;;/**
	;;
	;; 	Global variable: kstack_top
	;; 	------------------------------
	;;
	;; 	Define the top of the kernel stack
	;;
	;;**/
	

kstack:
	times	KERN_STACK_SIZE	db 0
kstack_top:
