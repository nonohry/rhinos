	;;/**
	;;
	;; 	klib_s.s
	;; 	========
	;;
	;; 	Kernel utilities library - Assembly code
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

global klib_outb
global klib_inb
global klib_msb
global klib_lsb
global klib_load_CR3
global klib_set_pg_cr0
global klib_flush_tlb
global klib_invlpg	
global klib_mem_set
global klib_mem_copy	
global klib_sti
global klib_idle
	
	
	
	;;/**
	;;
	;; 	Function: klib_outb(u16_t port, u8_t value)
	;; 	-------------------------------------------
	;;
	;; 	Output byte `value` on processor `port`
	;;
	;;**/
	

klib_outb:
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
	;; 	Function: klib_inb(u16_t port, u8_t* buf)
	;; 	-------------------------------------------
	;;
	;; 	Retrieve a byte from processor `port` to buffer ̀buf`
	;;
	;;**/
	
klib_inb:
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
	;; 
	;; 	Function: u32_t klib_msb(u32_t n)
	;;	---------------------------------
	;;
	;; 	Return most significant bit
	;;
	;;**/

	

klib_msb:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	mov  	edx,[ebp+8]	; move `n` to EDX
	bsr     eax,edx		; bsr
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret

	

	;;/**
	;; 
	;; 	Function: u32_t klib_lsb(u32_t n)
	;;	---------------------------------
	;;
	;; 	Return less significant bit
	;;
	;;**/	
	
klib_lsb:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	mov  	edx,[ebp+8]	; move `n` to EDX
	bsf     eax,edx		; bsf
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret

	
	;;/**
	;; 
	;; 	Function: void klib_load_CR3(u32_t cr3)
	;;	---------------------------------------
	;;
	;; 	Load the value `cr3` inr CR3 processor register
	;;
	;;**/

	
klib_load_CR3:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	mov  	eax,[ebp+8]	; move `cr3` in EAX	
	mov	cr3,eax		; load CR3
	pop	edi		; Restaure EDI
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret	


	;;/**
	;; 
	;; 	Function: void klib_set_pg_cr0(void)
	;;	------------------------------------
	;;
	;;	Activate pagination through CR0 register
	;;
	;;**/
	
	

klib_set_pg_cr0:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	xor	eax,eax		; Nullify EAX
	mov	eax,cr0		; Get CR0 in EAX
	or	eax, 0x80000000	; Activate PG bit (pagination)
	mov	cr0,eax		; Set CR0 to activate pagination
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret


	;;/**
	;; 
	;; 	Function: void klib_flush_tlb(void)
	;;	-----------------------------------
	;;
	;; 	Flush Translation Lookaside Buffer cache by reloading CR3
	;;
	;;**/
	

klib_flush_tlb:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	xor	eax,eax		; Nullify EAX
	mov	eax,cr3		; Get CR3
	mov	cr3,eax		; Reload CR3
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret
	

	;;/**
	;; 	Function: void klib_invlpg(virtaddr_t addr)
	;;	-------------------------------------------
	;;
	;; 	Flush the tlb entry corresponding to `addr` 
	;;
	;;**/

klib_invlpg:
	push 	ebp
	mov  	ebp,esp
	push	esi
	push	edi
	xor	eax,eax
	mov  	eax,[ebp+8]	; Get the addr
	invlpg	[eax]		; Clear the tlb entry
	pop	edi
	pop	esi
	mov	esp,ebp
	pop	ebp
	ret

	
	;;/**
	;; 	Function: klib_mem_set(u32_t val, addr_t dest, u32_t len)
	;;	---------------------------------------------------------
	;;
	;; 	Classical (quick & dirty) memset function
	;;
	;;**/
	
	
klib_mem_set:
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
	;; 	Function void klib_mem_copy(addr_t src, addr_t dest, u32_t len)
	;;	---------------------------------------------------------------
	;;
	;; 	Classical (quick & dirty) memcopy function
	;;
	;;**/
	
klib_mem_copy:
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
	;; 
	;; 	Function void klib_sti(void)
	;; 	---------------------------
	;;
	;; 	Restore interrupts using `sti`
	;;
	;;**/
	

klib_sti:
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
	
	
	;;/**
	;; 
	;; 	Function void klib_idle(void)
	;;	-----------------------------
	;;
	;; 	Code for idle thread. Just loop halting processor
	;;
	;;**/

	
klib_idle:
	hlt			; rest processor
	jmp	klib_idle
