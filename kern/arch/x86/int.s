	;;/**
	;;
	;; 	int.s
	;; 	=====
	;;
	;; 	Low level interrupt handling
	;;
	;;**/
	
	[BITS 32]



	;;/**
	;;
	;; 	Global
	;; 	------
	;;
	;; 	Global declarations to make ISR visible by C code
	;;
	;;**/
	
	
global	hwint_00
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

global	swint_syscall
	
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


	;;/**
	;;
	;; 	Extern
	;; 	------
	;;
	;; 	C code needed:
	;;
	;;	- irq_handle_flih	: IRQ generic handler
	;;	- excep_handle		: exception generic handler
	;; 	- ctx_postsave	        : helper to save context in case of ring jump
	;; 	- syscall_handle	: syscall generic handler
	;; 
	;;**/
	
extern	irq_handle_flih
extern	excep_handle
extern	ctx_postsave
extern  cur_th
	
extern	syscall_handle


	;;/**
	;; 
	;; 	Constants: segment selector
	;;	---------------------------
	;;
	;;**/

%assign		KERN_CS_SELECTOR		8  ; CS  = 00000001  0  00   = (byte) 8
%assign		KERN_DS_SELECTOR		16 ; DS  = 00000010  0  00   = (byte) 16
%assign		KERN_ES_SELECTOR		16 ; ES  = 00000010  0  00   = (byte) 16
%assign		KERN_SS_SELECTOR		16 ; SS  = 00000010  0  00   = (byte) 16
	
	;;/**
	;; 
	;; 	Constants: IRQ magic numbers
	;;	----------------------------
	;;
	;;**/

%assign		IRQ_EOI			0x20
%assign		IRQ_MASTER		0x20
%assign		IRQ_SLAVE		0xA0

	;;/**
	;; 
	;; 	Constant: Fake Error Code
	;;	-------------------------
	;;
	;; 	Interrupts and some exceptions dont provide an error code.
	;; 	This error code will be pushed on stack in this case to have a generic handler
	;;
	;;**/
	
%assign		FAKE_ERROR		0xFEC
	
	;;/**
	;; 
	;; 	Constants: Offsets
	;; 	------------------
	;; 
	;;	Offset of fields:
	;; 	- ret_addr
	;; 	- cs
	;; 	- esp
	;; 	in struct cpu_info
	;;
	;;**/
	

%assign		THREAD_RET_OFFSET		40
%assign		THREAD_CS_OFFSET		12
%assign		THREAD_ESP_OFFSET		20
	
	;;/**
	;;
	;; 	Constant: interrupt stack size
	;; 	------------------------------
	;;
	;;**/

%assign		INT_STACK_SIZE		1024	
	
	;;/**
	;;
	;; 	Constants: Exceptions Vectors
	;; 	-----------------------------
	;;
	;;**/

%assign		DIVIDE_VECTOR		0
%assign		DEBUG_VECTOR		1
%assign		NMI_VECTOR		2
%assign		BREAKPT_VECTOR		3
%assign		OVERFLOW_VECTOR		4
%assign		BOUND_VECTOR		5
%assign		OPCODE_VECTOR		6
%assign		NOMATH_VECTOR		7
%assign		DFAULT_VECTOR		8
%assign		COSEG_VECTOR		9
%assign		TSS_VECTOR		10
%assign		NOSEG_VECTOR		11
%assign		SSFAULT_VECTOR		12
%assign		GPROT_VECTOR		13
%assign		PFAULT_VECTOR		14
%assign		MFAULT_VECTOR		16
%assign		ALIGN_VECTOR		17
%assign		MACHINE_VECTOR		18


	;;/**
	;;
	;; 	Macro: hwint_generic0
	;; 	---------------------
	;;
	;; 	All the hardware interrupts are treated the same, so we create a macro to repeat the processing.
	;; 	The actions are:
	;;
	;; 	- Push a fake erroc code (hardware interrupt have no error code)
	;; 	- Save the CPU context
	;; 	- Call the C handler with IRQ as a parameter
	;; 	- Acknowledge the master PIC
	;; 	- Restore a CPU context
	;;
	;;**/
	
%macro	hwint_generic0	1
	push	FAKE_ERROR
   	call	save_ctx
	push	%1
 	call	irq_handle_flih
	add	esp,8
	mov	al,IRQ_EOI
	out	IRQ_MASTER,al
	call	restore_ctx
%endmacro


	;;/**
	;;
	;; 	Macro: hwint_generic1
	;; 	---------------------
	;;
	;; 	Same as above, except that acknowledgement is also sent to the slave PIC
	;;
	;;**/

%macro	hwint_generic1	1
	push	FAKE_ERROR
	call	save_ctx
	push	%1
	call	irq_handle_flih
	add	esp,8
	mov	al,IRQ_EOI
	out	IRQ_SLAVE,al
	out	IRQ_MASTER,al
	call	restore_ctx
%endmacro


	
	;;/**
	;;
	;; 	Functions: hwint_xx
	;; 	-------------------
	;;
	;; 	Define the 16 hardware interrupt handlers corresponding to the 16 IRQ vectors
	;; 	All the definition use the  hwint_generic macros
	;;
	;;**/
	

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
	
	
	;;/**
	;;
	;; 	Function: swint_syscall
	;; 	-----------------------
	;;
	;; 	low level handler for software interrupt, which is in fact a syscall.
	;; 	Just save the CPU context and call the C syscall handler before restoring a context
	;;
	;;**/



swint_syscall:
        push    FAKE_ERROR
        call    save_ctx
        call    syscall_handle
        call    restore_ctx
	
	
	;;/**
	;;
	;; 	Function: save_ctx
	;;	------------------
	;;
	;; 	Save an interrupted CPU context in the thread struct cpu_info
	;; 	A difference is made if the thread is userland or not
	;;
	;;**/

save_ctx:

	cld

	;; Save ESP and point it to ret_addr field of context
	mov dword [save_esp],esp
	mov esp, [cur_th]
	add esp,THREAD_RET_OFFSET

	;;  Push to save registers 
	pushad
	o16 push	ds
	o16 push	es
	o16 push	fs
	o16 push	gs

	;; Get on the interrupt stack with kernel segments (CS already set by processor)
	mov	esp, int_stack_top
	mov	ax,KERN_DS_SELECTOR
	mov	ds,ax
	mov	ax,KERN_ES_SELECTOR
	mov	es,ax

	;; Call thread_cpu_postsave to save remaining registers in case of an interrupted kernel thread 
	push	dword [save_esp]
	push 	dword [cur_th]
	call	ctx_postsave
	add	esp,8

	;; Get the save stack to return to caller
	mov	eax,dword [save_esp]
	jmp	[eax]

	
	;;/**
	;;
	;; 	Function: restore_ctx
	;; 	---------------------
	;;
	;; 	Restore the context of cur_th thread
	;; 	It simply pops the registers from the cur_th struct cpu_info
	;;
	;;**/

	
restore_ctx:
	mov 	esp, [cur_th]
	o16 pop gs
	o16 pop fs
	o16 pop	es
	o16 pop	ds
	popad

	;; In case of an interrupted kernel thread, we move back to the thread stack
	;; which contains the parameters for itretd					       
	cmp 	dword [esp+THREAD_CS_OFFSET], KERN_CS_SELECTOR
	jne 	restore_ctx_next
	mov 	esp, dword [esp+THREAD_ESP_OFFSET]
	
restore_ctx_next:
	add 	esp,4		; pop save_ctx return address
	add 	esp,4		; pop error code
	iretd


	
	;;/**
	;;
	;; 	Functions: excep_xx
	;; 	-------------------
	;;
	;; 	Define the exception handlers. In fact, they only push an error code if the exception does not provide one
	;; 	and the exception vector before jumping to excep_next.
	;;
	;;**/

excep_00:
	push	FAKE_ERROR
	push	DIVIDE_VECTOR
	jmp	excep_next

excep_01:
	push	FAKE_ERROR
	push	DEBUG_VECTOR
	jmp	excep_next

excep_02:
	push	FAKE_ERROR
	push	NMI_VECTOR
	jmp	excep_next

excep_03:
	push	FAKE_ERROR
	push	BREAKPT_VECTOR
	jmp	excep_next

excep_04:
	push	FAKE_ERROR
	push	OVERFLOW_VECTOR	
	jmp	excep_next

excep_05:
	push	FAKE_ERROR
	push	BOUND_VECTOR
	jmp	excep_next

excep_06:
	push	FAKE_ERROR
	push	OPCODE_VECTOR	
	jmp	excep_next

excep_07:
	push	FAKE_ERROR
	push	NOMATH_VECTOR
	jmp	excep_next
	
excep_08:
	push	DFAULT_VECTOR
	jmp	excep_next

excep_09:
	push	FAKE_ERROR
	push	COSEG_VECTOR	
	jmp	excep_next	

excep_10:
	push	TSS_VECTOR	
	jmp	excep_next

excep_11:
	push	NOSEG_VECTOR
	jmp	excep_next	

excep_12:
	push	SSFAULT_VECTOR	
	jmp	excep_next

excep_13:
	push	GPROT_VECTOR	
	jmp	excep_next

excep_14:
	push	PFAULT_VECTOR	
	jmp	excep_next
	
excep_16:
	push	FAKE_ERROR
	push	MFAULT_VECTOR
	jmp	excep_next	

excep_17:
	push	ALIGN_VECTOR
	jmp	excep_next

excep_18:
	push	FAKE_ERROR
	push	MACHINE_VECTOR	
	jmp	excep_next	
	
	
	;;/**
	;;
	;; 	Function: excep_next
	;; 	--------------------
	;;
	;; 	Real low level exceptions handler. Actions are:
	;; 	- Save the CPU context
	;; 	- Call the C handler with exception vector and the current thread
	;; 	- Restore a CPU context
	;;
	;;**/
	
excep_next:
	pop	dword [excep_num]
	call	save_ctx
	push	dword [cur_th]
	push	dword [excep_num]
	call	excep_handle
	add	esp,8
	call	restore_ctx



	;;/**
	;;
	;; 	Global variables
	;; 	----------------
	;;
	;; 	- excep_num	: exception vector
	;;	- save_esp	: saved esp during save_ctx
	;;
	;;**/
	
	excep_num	dd	0 
	save_esp	dd	0

	
	;;/**
	;;
	;; 	Global variable: int_stack_top
	;; 	------------------------------
	;;
	;; 	Define the top of the interrupt stack
	;;
	;;**/
	

int_stack:
	times	INT_STACK_SIZE	db 0
int_stack_top:
