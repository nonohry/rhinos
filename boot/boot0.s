	;;
	;; Yabolo - boot0
	;;

[BITS 16]

	;;========================================================================
	;;  Constantes
	;;========================================================================
	
	%assign BOOT0_RELOC_SRC		0x7C00
	%assign BOOT0_RELOC_DST 	0x600
	%assign BOOT0_RELOC_SEG		0x60
	%assign	BOOT0_SP		0x90000

	%assign BOOT1_LBA_START		1
	%assign	BOOT1_SIZE		2
	%assign BOOT1_LBA_END		(BOOT1_LBA_START+BOOT1_SIZE-1)
	%assign	BOOT1_ABS_ADDR		0x800
	

	;;========================================================================
	;;  Macros
	;;========================================================================

	%macro  PRINT 1 
        	mov     si,%1
		call	print_message
	%endmacro

	
	;;========================================================================
	;;  Structures
	;;========================================================================

	struc	drive_info
		number:		resb	1
		cylinders:	resw	1
		heads:		resb	1
		sectors:	resb	1
	endstruc

	
	struc	chs_info
		cylinder:	resw	1
		head:		resb	1
		sector:		resb	1
	endstruc


	struc	disk_address_packet
		size:		resb	1
		unused:		resb	1
		count:		resw	1
		buffer:		resd	1
		startlba:	resq	1
	endstruc
	
	
	;;========================================================================
	;; Relocation du bootloader
	;;========================================================================

	
	cli	
	cld
	mov  	si,BOOT0_RELOC_SRC
	mov	di,BOOT0_RELOC_DST
	mov	cx,512
	shr	cx,0x2
	rep movsd
	jmp	0:BOOT0_RELOC_DST+start


	;;========================================================================
	;; Point d Entree Relocalise
	;;========================================================================

	
start:
	;; Replace les data segment registers
	mov	ax,BOOT0_RELOC_SEG
	mov	ds,ax
	mov	es,ax
	mov	sp,BOOT0_SP
	sti
	
	;; Affiche YABOLO
	PRINT 	BOOT0_YABOLO

	;; Controle le boot drive
	mov	[bootdrv_info+number],dl
	xor	ax,ax
	mov	al,dl
	and	al,0x80
	jz	chs_geometry

	;; Verifie le support du LBA
	mov	ah,0x41
	mov	bx,0x55AA
	mov	dl, byte [bootdrv_info+number]
	int	0x13
	jc	chs_geometry
	cmp	bx,0xAA55
	jne	chs_geometry

	;; Preparation au chargement des secteurs
	mov	si,BOOT1_LBA_START
	mov	di,BOOT1_ABS_ADDR
	mov	byte [sector_dap+size],0x10
	mov	byte [sector_dap+unused],0x00
	mov	word [sector_dap+count],0x01
	
lba_load:	

	;; Chargement en LBA
	mov	word [sector_dap+buffer],di
	mov	word [sector_dap+startlba],si
	push	si
	mov	si,sector_dap
	mov	ah,0x42
	int	0x13
	jc	read_error
	pop	si	
	
	;; Increment de la boucle
	inc	si
	add	di,0x200
	cmp	si,BOOT1_LBA_END
	jle	lba_load

	;; Saut final
	jmp	0x0:0x800
	
chs_geometry:	
	
	;; Geometrie du disque via int 0x13,ah=0x08
	xor	di,di
	mov	es,di
	mov	ah,0x08
	int	0x13
	jc 	geometry_error
	inc	dh
	mov	[bootdrv_info+heads],dh
	mov	[bootdrv_info+sectors],cl
	and	byte [bootdrv_info+sectors],0x3F
	shr	cl,0x06
	xor	cl,ch		; xor swapping :)
	xor	ch,cl
	xor	cl,ch
	mov	[bootdrv_info+cylinders],cx
	inc 	word [bootdrv_info+cylinders]

	;; Preparation au chargement des secteurs
	mov	si,BOOT1_LBA_START
	mov	di,BOOT1_ABS_ADDR
	xor	ah,ah
	mov	dl,byte [bootdrv_info+number]
	int	0x13
	jc	reset_error
	
chs_load:
	
	;; Conversion LBA vers CHS
	xor	dx,dx
	xor	cx,cx
	mov	ax,si	; Secteur LBA [0,inf[
	mov	cl,[bootdrv_info+sectors]
	div	cx
	inc	dx
	mov	byte [sector_info+sector],dl
	xor	dx,dx
	xor	cx,cx
	mov	cl,byte [bootdrv_info+heads]
	div 	cx
	mov	word [sector_info+cylinder],ax
	mov	byte [sector_info+head],dl

	;; Chargement du secteur courant
	xor	ax,ax
	mov	es,ax
	mov	bx,di
	mov	ah,0x02
	mov	al,0x01
	mov	cx, word [sector_info+cylinder]
	xor	cl,ch		; xor swapping :)
	xor	ch,cl
	xor	cl,ch
	shl	cl,0x06
	or	cl,byte [sector_info+sector]
	mov	dh,byte [sector_info+head]
	mov	dl,byte [bootdrv_info+number]
	int	0x13
	jc 	read_error
	
	;; Increment de la boucle
	inc	si
	add	di,0x200
	cmp	si,BOOT1_LBA_END
	jle	chs_load

	;; Saut final
	jmp	0x0:0x800


	;;========================================================================
	;; Erreurs
	;;========================================================================

reset_error:
	PRINT	BOOT0_RESET_ERROR
	jmp	$
	
geometry_error:
	PRINT	BOOT0_GEO_ERROR
	jmp	$

read_error:
	PRINT	BOOT0_READ_ERROR
	jmp	$

	
	;;========================================================================
	;; Affiche un message
	;;========================================================================

	
print_message:
        lodsb
        or      al,al
        jz      print_message_end
        mov     ah,0x0E
        int     0x10
        jmp     print_message
print_message_end:
        ret


	;;========================================================================
	;;  Chaines
	;;========================================================================

	BOOT0_YABOLO		db 	"YABOLO",13,10,0
	BOOT0_RESET_ERROR	db	"Reset Error",13,10,0
	BOOT0_GEO_ERROR		db	"Geometry Error",13,10,0
	BOOT0_READ_ERROR	db	"Read Error",13,10,0

	;;========================================================================
	;;  Instances de Structures
	;;========================================================================

	
bootdrv_info:	istruc drive_info
		iend

sector_info:	istruc chs_info
		iend

sector_dap:	istruc disk_address_packet
		iend
	
	;;========================================================================
	;; Fin
	;;========================================================================

	times	510-($-$$) db 0	; Padding pour atteinde 512 octets 
	dw	0xAA55		; BIOS Magic Number
