
	[BITS 32]

	;;========================================================================
	;; Primitives IPC
	;;========================================================================

	
global	ipc_test		; Primitives visibles pour le C

	
	;;========================================================================
	;; Constantes
	;;========================================================================

	
SYSCALL_VECTOR	equ	50

	
	;;========================================================================
	;; ipc_test(int a, int b)
	;;========================================================================	

	
ipc_test:
        push    ebp             ; Sauvegarde de EBP
        mov     ebp,esp         ; Mise en place de la base
        push    esi             ; Sauvegarde ESI (Requis par GCC)
        push    edi             ; Sauvegarde EDI (Requis par GCC)
        push    ebx             ; Sauvegarde EBX
        push    ecx             ; Sauvegarde ECX
	push	edx		; Sauvegarde EDX
        mov     ebx,[ebp+8]     ; Recupere le 1er arg dans EBX
        mov     ecx,[ebp+12]    ; Recupere le 2nd arg dans ECX
        mov     edx,1           ; Numero SYSCALL dans EDX
        int     SYSCALL_VECTOR  ; Instruction int
        pop     edx             ; Restaure EDX
        pop     ecx             ; Restaure ECX
        pop     ebx             ; Restaure EBX
        pop     edi             ; Restaure EDI
        pop     esi             ; Restaure ESI
        mov     esp,ebp         ; Restaure la pile
        pop     ebp             ; Restaure EBP
        ret
	