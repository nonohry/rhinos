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


	;;========================================================================
	;; klib_flush_tlb(void)
	;;========================================================================
	

klib_flush_tlb:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	push	esi		; Sauvegarde ESI (Requis par GCC)
	push	edi		; Sauvegarde EDI (Requis par GCC)
	xor	eax,eax		; Nullifie EAX
	mov	eax,cr3		; Recupere CR3
	mov	cr3,eax		; Recharge CR3
	pop	edi		; Restaure EDI
	pop	esi		; Restaure ESI
	mov	esp,ebp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret
	

	;;========================================================================
	;; klib_invlpg(virtaddr_t)
	;;========================================================================
	

klib_invlpg:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	push	esi		; Sauvegarde ESI (Requis par GCC)
	push	edi		; Sauvegarde EDI (Requis par GCC)
	xor	eax,eax		; Nullifie EAX
	mov  	eax,[ebp+8]	; Recupere le parametre
	invlpg	[eax]		; Invalide le tlb
	pop	edi		; Restaure EDI
	pop	esi		; Restaure ESI
	mov	esp,ebp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret

	
	;;========================================================================
	;; klib_mem_set(u32_t val, addr_t dest, u32_t len)
	;;========================================================================
	
	
klib_mem_set:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	push	esi		; Sauvegarde ESI (Requis par GCC)
	push	edi		; Sauvegarde EDI (Requis par GCC)
	push	ecx		; Sauvegarde ECX
	cld			; Fixe le sens du decompte
	mov  	eax,[ebp+8]	; Recupere la valeur
	mov	edi,[ebp+12]	; Recupere l addresse de destination
	mov	ecx,[ebp+16] 	; Recupere la taille
	shr	ecx,0x2		; Divise la taille par 4
	rep stosd		; Set !
	pop	ecx		; Restaure ECX
	pop	edi		; Restaure EDI
	pop	esi		; Restaure ESI
	mov	esp,ebp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret

	
	;;========================================================================
	;; klib_mem_copy(addr_t src, addr_t dest, u32_t len)
	;;========================================================================

	
klib_mem_copy:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	push	esi		; Sauvegarde ESI (Requis par GCC)
	push	edi		; Sauvegarde EDI (Requis par GCC)
	push	ecx		; Sauvegarde ECX
	cld			; Fixe le sens du decompte
	mov  	esi,[ebp+8]	; Recupere l addresse source
	mov	edi,[ebp+12]	; Recupere l addresse de destination
	mov	ecx,[ebp+16] 	; Recupere la taille
	shr	ecx,0x2		; Divise la taille par 4
	rep movsd		; Copie !
	pop	ecx		; Restaure ECX
	pop	edi		; Restaure EDI
	pop	esi		; Restaure ESI
	mov	esp,ebp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret


	;;========================================================================
	;; klib_sti(void)
	;;========================================================================
	

klib_sti:
	push 	ebp         	; Sauvegarde de EBP
	mov  	ebp,esp 	; Mise en place de la base
	push	esi		; Sauvegarde ESI (Requis par GCC)
	push	edi		; Sauvegarde EDI (Requis par GCC)
	sti			; Restaure les interruptions
	pop	edi		; Restaure EDI
	pop	esi		; Restaure ESI
	mov	esp,ebp		; Restaure la pile
	pop	ebp		; Restaure EBP
	ret
	
	
	;;========================================================================
	;; Idle thread
	;;========================================================================

	
klib_idle:
	hlt			; Repose le processeur
	jmp	klib_idle
	
	
	;;========================================================================
	;; Donnees
	;;========================================================================

	
BASE	dd	0		; Base de decomposition
