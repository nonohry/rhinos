	;;/**
	;;
	;; 	x86_lib.s
	;; 	=========
	;;
	;; 	x86 utilities library - Assembly code
	;;
	;;**/




	[BITS 32]


	;;/**
	;;
	;; 	Global
	;; 	------
	;;
	;; 	Global declarations to make routines visible by C code
	;;
	;;**/

	
global x86_outb
global x86_inb
global x86_mem_copy
global x86_mem_set
global x86_load_pd
global x86_get_pf_addr
global x86_sti
	
	;;/**
	;;
	;; 	Function: x86_outb(u16_t port, u8_t value)
	;; 	-------------------------------------------
	;;
	;; 	Output byte `value` on processor `port`
	;;
	;;**/
	

x86_outb:
	push 	ebp		
	mov  	ebp,esp
	push	esi
	push	edi
	mov  	dx,[ebp+8]	; move ̀port` in DX
	mov     al,[ebp+12]     ; move `value` in AL
	out     dx,al		; out
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret

	
	;;/**
	;;
	;; 	Function: x86_inb(u16_t port, u8_t* buf)
	;; 	-------------------------------------------
	;;
	;; 	Retrieve a byte from processor `port` to buffer ̀buf`
	;;
	;;**/
	
x86_inb:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	mov  	dx,[ebp+8]	; move ̀port` in DX
	mov	ebx,[ebp+12]	; move `buf` address in EBX
	in      al,dx		; in
	mov	byte [ebx],al	; put `in` value in EBX
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret


	;;/**
	;; 	Function void x86_mem_copy(addr_t src, addr_t dest, size_t len)
	;;	--------------------------------------------------------------
	;;
	;; 	Classical (quick & dirty) memcopy function
	;;
	;;**/

	
x86_mem_copy:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	push	ecx
	cld
	mov  	esi,[ebp+8]	; Get `src`
	mov	edi,[ebp+12]	; Get `dest`
	mov	ecx,[ebp+16] 	; Get `len`
	shr	ecx,0x2		; len/4
	rep movsd		; Copy !
	pop	ecx
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret

	
;;/**
	;; 	Function: x86_mem_set(u32_t val, addr_t dest, size_t len)
	;;	---------------------------------------------------------
	;;
	;; 	Classical (quick & dirty) memset function
	;;
	;;**/
	
	
x86_mem_set:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	push	ecx
	cld
	mov  	eax,[ebp+8]	; Get `val`
	mov	edi,[ebp+12]	; Get `dest`
	mov	ecx,[ebp+16] 	; Get `len`
	shr	ecx,0x2		; len/4
	rep stosd		; Set !
	pop	ecx
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret


	;;/**
	;; 
	;; 	Function: void x86_load_pd(u32_t pd)
	;;	-------------------------------------
	;;
	;; 	Load the page directory `pd` in CR3 processor register
	;;
	;;**/

	
x86_load_pd:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	mov  	eax,[ebp+8]	; move `pd` in EAX	
	mov	cr3,eax		; load CR3
	pop	edi		; Restaure EDI
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret	

	;;/**
	;; 
	;; 	Function: virtaddr_t x86_get_pf_addr(void)
	;;	------------------------------------------
	;;
	;; 	Get page faulting address from CR2 register
	;;
	;;**/

	
x86_get_pf_addr:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	xor  	eax,eax		; move `pd` in EAX	
	mov	eax, cr2	; load CR3
	pop	edi		; Restaure EDI
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret	

	;;/**
	;; 
	;; 	Function: void x86_sti(void)
	;; 	----------------------------
	;;
	;; 	Restore interrupts using `sti`
	;;
	;;**/
	

x86_sti:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	sti			; Restore  interrupts
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret