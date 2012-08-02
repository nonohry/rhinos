
	[BITS 32]

	;;========================================================================
	;; Primitives IPC
	;;========================================================================

	
global	ipc_send		; Primitives visibles pour le C
global	ipc_receive
global	ipc_notify
global	ipc_sendrec
	
	
	;;========================================================================
	;; Constantes
	;;========================================================================

	
IPC_SYSCALL_VECTOR	equ	50
IPC_SEND_NUM		equ	1
IPC_RECEIVE_NUM		equ	2
IPC_NOTIFY_NUM		equ	3	
IPC_SENDREC_NUM		equ 	4
IPC_SUCCESS		equ	0
	
	
	;;========================================================================
	;; ipc_send(int to, ipc_message* msg)
	;;========================================================================	

	
ipc_send:
        push    ebp             ; Sauvegarde de EBP
        mov     ebp,esp         ; Mise en place de la base
        push    esi             ; Sauvegarde ESI (Requis par GCC)
        push    edi             ; Sauvegarde EDI (Requis par GCC)
        push    ebx             ; Sauvegarde EBX
        push    ecx             ; Sauvegarde ECX
	push	edx		; Sauvegarde EDX
        mov     ebx,[ebp+8]     ; Recupere le 1er arg dans EBX
        mov     ecx,[ebp+12]    ; Recupere le 2nd arg dans ECX
        mov     edx,IPC_SEND_NUM    ; Numero SYSCALL dans EDX
        int     IPC_SYSCALL_VECTOR  ; Instruction int
        pop     edx             ; Restaure EDX
        pop     ecx             ; Restaure ECX
        pop     ebx             ; Restaure EBX
        pop     edi             ; Restaure EDI
        pop     esi             ; Restaure ESI
        mov     esp,ebp         ; Restaure la pile
        pop     ebp             ; Restaure EBP
        ret

	
	;;========================================================================
	;; ipc_receive(int from, ipc_message* msg)
	;;========================================================================	

	
ipc_receive:
        push    ebp             ; Sauvegarde de EBP
        mov     ebp,esp         ; Mise en place de la base
        push    esi             ; Sauvegarde ESI (Requis par GCC)
        push    edi             ; Sauvegarde EDI (Requis par GCC)
        push    ebx             ; Sauvegarde EBX
        push    ecx             ; Sauvegarde ECX
	push	edx		; Sauvegarde EDX
        mov     ebx,[ebp+8]     ; Recupere le 1er arg dans EBX
        mov     ecx,[ebp+12]    ; Recupere le 2nd arg dans ECX
        mov     edx,IPC_RECEIVE_NUM ; Numero SYSCALL dans EDX
        int     IPC_SYSCALL_VECTOR  ; Instruction int
        pop     edx             ; Restaure EDX
        pop     ecx             ; Restaure ECX
        pop     ebx             ; Restaure EBX
        pop     edi             ; Restaure EDI
        pop     esi             ; Restaure ESI
        mov     esp,ebp         ; Restaure la pile
        pop     ebp             ; Restaure EBP
        ret

	
	;;========================================================================
	;; ipc_notify(int to)
	;;========================================================================	

	
ipc_notify:
        push    ebp             ; Sauvegarde de EBP
        mov     ebp,esp         ; Mise en place de la base
        push    esi             ; Sauvegarde ESI (Requis par GCC)
        push    edi             ; Sauvegarde EDI (Requis par GCC)
        push    ebx             ; Sauvegarde EBX
        push    edx             ; Sauvegarde EDX
        mov     ebx,[ebp+8]     ; Recupere le 1er arg dans EBX
        mov     edx,IPC_NOTIFY_NUM  ; Numero SYSCALL dans EDX
        int     IPC_SYSCALL_VECTOR  ; Instruction int
        pop     edx             ; Restaure EDX
        pop     ebx             ; Restaure EBX
        pop     edi             ; Restaure EDI
        pop     esi             ; Restaure ESI
        mov     esp,ebp         ; Restaure la pile
        pop     ebp             ; Restaure EBP
        ret


	;;========================================================================
	;; ipc_sendrec(int to, ipc_message* msg)
	;;========================================================================	

	
ipc_sendrec:
        push    ebp             ; Sauvegarde de EBP
        mov     ebp,esp         ; Mise en place de la base
        push    esi             ; Sauvegarde ESI (Requis par GCC)
        push    edi             ; Sauvegarde EDI (Requis par GCC)
        push 	dword [ebp+12]	; Appelle ipc_send avec les arguments
        push 	dword [ebp+8]   
	call	ipc_send
	add	esp,8
	cmp 	eax,IPC_SUCCESS
	jne	ipc_sendrec_end
	push 	dword [ebp+12]  ; Appelle ipc_receive avec les arguments
        push 	dword [ebp+8]    
	call	ipc_receive
	add	esp,8
	cmp 	eax,IPC_SUCCESS
	jne	ipc_sendrec_end
        push 	dword [ebp+8]   ; Appelle ipc_notify avec le bon argument
	call	ipc_notify
	add	esp,4
ipc_sendrec_end:	
        pop     edi             ; Restaure EDI
        pop     esi             ; Restaure ESI
        mov     esp,ebp         ; Restaure la pile
        pop     ebp             ; Restaure EBP
        ret	