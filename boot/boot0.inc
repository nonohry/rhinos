	;;
	;; Boot0.inc
	;; Utilitaires pour Boot0.s
	;; 


	;;
	;; Detection du CPU (methode Intel)
	;;

	CPU_SUCCESS	equ 	0x01
	CPU_FAIL	equ	0x00
	
cpu_type:
        pushf                   ; Sauve FLAGS sur la pile
        xor     ax,ax           ; Vide AX
        push    ax              ; Empile AX
        popf                    ; Pop AX dans FLAGS, FLAGS est donc reset

        pushf                   ; Empile le FLAGS reset
        pop     ax              ; Le recupere dans AX
        and     ax,0x0F000      ; AND pour avoir la valeur du bit 15
        cmp     ax,0x0F000      ; Compare cette valeur
        je      cpu_86          ; Saute si le bit est a 1

        mov     ax,0x0F000      ; On va tester de la meme
        push    ax              ; maniere si on peut
        popf                    ; modifier les bits 13-14
        pushf                   ; Si on ne peut pas
        pop     ax              ; alors nous sommes en
        and     ax,0x0F000      ; presence d un
        jz      cpu_286         ; 80286

        popf                    ; Restaure FLAGS
	xor	ax,ax		; Reset AX
	mov	ax, CPU_SUCCESS	; Valeur de retour
        ret                     ; On retourne
cpu_86:				; On a un 8086/8088
cpu_286:			; On a un 30286
cpu_end:
        popf                    ; Restaure FLAGS
	xor	ax,ax		; Reset AX
	mov	ax, CPU_FAIL	; Valeur de retour
	ret			; Retourne
