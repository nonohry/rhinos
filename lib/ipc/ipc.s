	;;/**
	;;
	;;	ipc.s
	;;	=====
	;;
	;;	Kernel syscalls wrappers
	;;
	;;**/

	
	[BITS 32]

	;;/**
	;;
	;; 	Globals
	;; 	-------
	;;
	;; 	Make ipc primitive accessible
	;;
	;;**/

	
global	ipc_send
global	ipc_receive
global	ipc_notify
global	ipc_sendrec
	
	
	;;/**
	;; 
	;; 	Constants
	;; 	---------
	;;
	;; 	Syscall numbers
	;;
	;;**/

	
IPC_SYSCALL_VECTOR	equ	50
IPC_SEND_NUM		equ	1
IPC_RECEIVE_NUM		equ	2
IPC_NOTIFY_NUM		equ	3
IPC_SUCCESS		equ	0
	
	
	;;/**
	;; 
	;; 	Function: u8_t ipc_send(int to, ipc_message* msg)
	;;	-------------------------------------------------
	;;
	;; 	Send a message to thread identified by thread id `to`
	;; 
	;; 	Move arguments in register to avoid trashing during context switching
	;; 	Do a int call with appropriate interrupt vector
	;;
	;;**/
	
	
ipc_send:
        push    ebp
        mov     ebp,esp
        push    esi
        push    edi
        push    ebx
        push    ecx
	push	edx
        mov     ebx,[ebp+8]
        mov     ecx,[ebp+12]
        mov     edx,IPC_SEND_NUM
	jmp	$
        int     IPC_SYSCALL_VECTOR
        pop     edx
        pop     ecx
        pop     ebx
        pop     edi
        pop     esi
        mov     esp,ebp
        pop     ebp
        ret

	
	;;/**
	;; 
	;; 	Function: u8_t ipc_receive(int from, ipc_message* msg)
	;; 	------------------------------------------------
	;;
	;; 	Rerceived a message from thread identified by thread id `from` (or ANY for any thread)
	;; 
	;; 	Move arguments in register to avoid trashing during context switching
	;; 	Do a int call with appropriate interrupt vector
	;;
	;;**/

	
ipc_receive:
        push    ebp
        mov     ebp,esp
        push    esi
        push    edi
        push    ebx
        push    ecx
	push	edx
        mov     ebx,[ebp+8]
        mov     ecx,[ebp+12]
        mov     edx,IPC_RECEIVE_NUM
        int     IPC_SYSCALL_VECTOR
        pop     edx
        pop     ecx
        pop     ebx
        pop     edi
        pop     esi
        mov     esp,ebp
        pop     ebp
        ret

	
	;;/**
	;; 
	;; 	Function: u8_t ipc_notify(int to)
	;;	---------------------------------
	;;
	;; 	Notify thread `to` of blocking send end
	;;
	;;**/

	
ipc_notify:
        push    ebp
        mov     ebp,esp
        push    esi
        push    edi
        push    ebx
        push    edx
        mov     ebx,[ebp+8]
        mov     edx,IPC_NOTIFY_NUM
        int     IPC_SYSCALL_VECTOR
        pop     edx
        pop     ebx
        pop     edi
        pop     esi
        mov     esp,ebp
        pop     ebp
        ret


	;;/**
	;;
	;; 	ipc_sendrec(int to, ipc_message* msg) 
	;;	-------------------------------------
	;;
	;; 	Send a messge to thread `to` and wait for a response message
	;;
	;; 	Simply call `ipc_send` the `ipc_receive` if first call succeed
	;; 	then a notify is sent to thread sending response.
	;;
	;;**/

	
ipc_sendrec:
        push    ebp
        mov     ebp,esp
        push    esi
        push    edi
        push 	dword [ebp+12]
        push 	dword [ebp+8]   
	call	ipc_send
	add	esp,8
	cmp 	eax,IPC_SUCCESS
	jne	ipc_sendrec_end
	push 	dword [ebp+12]
        push 	dword [ebp+8]    
	call	ipc_receive
	add	esp,8
	cmp 	eax,IPC_SUCCESS
	jne	ipc_sendrec_end
        push 	dword [ebp+8]
	call	ipc_notify
	add	esp,4
ipc_sendrec_end:	
        pop     edi
        pop     esi
        mov     esp,ebp
        pop     ebp
        ret
	