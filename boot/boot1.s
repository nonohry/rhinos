	;;
	;; Yabolo - boot1
	;;

[BITS 16]

	;;========================================================================
	;;  Constantes
	;;========================================================================
	
	%assign BOOT1_SEG		0x80

	%assign KERN_LBA_START		3
	%assign	KERN_SIZE		100
	%assign KERN_LBA_END		(KERN_LBA_START+KERN_SIZE-1)
	%assign	KERN_ABS_ADDR		0x1000

	%assign	BOOT1_MMAP_ADDR		0xC00
	%assign	BOOT1_MMAP_SIZE		0x14

	%assign	BOOT1_KB_CMD		0x64
	%assign	BOOT1_KB_DATA		0x60
	%assign	BOOT1_KB_IFULL		0x02
	%assign	BOOT1_KB_OFULL		0x01

	%assign	BOOT1_A20_WRAPADDR	0x80E

	%assign	BOOT1_CS_SELECTOR	0x08
	%assign	BOOT1_XS_SELECTOR	0x10
	%assign BOOT1_ESP		0x90000
	

	;;========================================================================
	;;  Macros
	;;========================================================================

	%macro  PRINT 1 
        	mov     si,%1
		call	print_message
	%endmacro


	%macro	KB_WAIT_INPUT 0
	%%1:	
		in	al,BOOT1_KB_CMD
		test	al,BOOT1_KB_IFULL
		jne	%%1		
	%endmacro
	

	%macro	KB_WAIT_OUTPUT 0
	%%1:	
		in	al,BOOT1_KB_CMD
		test	al,BOOT1_KB_OFULL
		jne	%%1		
	%endmacro
	
	;;========================================================================
	;;  Structures
	;;========================================================================

	struc	boot_info
		kern_start		resd	1
		kern_end		resd	1
		drv_number:		resb	1
		drv_cylinders:		resw	1
		drv_heads:		resb	1
		drv_sectors:		resb	1
		mem_map_addr		resd	1
		mem_map_count		resw	1
		mem_upper		resw	1
		mem_lower		resw	1
		mem_0x0			resw	1
		mem_0x100000		resw	1
		mem_total		resd	1
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
	;; Point d Entree: Chargment du noyau
	;;========================================================================

	
start:
	;; Replace les data segment registers
	cli
	mov	ax,BOOT1_SEG
	mov	ds,ax
	mov	es,ax
	sti

	;; Controle le boot drive
	mov	[boot1_info+drv_number],dl
	xor	ax,ax
	mov	al,dl
	and	al,0x80
	jz	chs_geometry

	;; Verifie le support du LBA
	mov	ah,0x41
	mov	bx,0x55AA
	mov	dl, byte [boot1_info+drv_number]
	int	0x13
	jc	chs_geometry
	cmp	bx,0xAA55
	jne	chs_geometry

	;; Preparation au chargement des secteurs
	mov	si,KERN_LBA_START
	mov	di,KERN_ABS_ADDR
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
	cmp	si,KERN_LBA_END
	jle	lba_load

	;; Saut a la suite
	jmp	next01
	
chs_geometry:	
	
	;; Geometrie du disque via int 0x13,ah=0x08
	xor	di,di
	mov	es,di
	mov	ah,0x08
	int	0x13
	jc 	geometry_error
	inc	dh
	mov	[boot1_info+drv_heads],dh
	mov	[boot1_info+drv_sectors],cl
	and	byte [boot1_info+drv_sectors],0x3F
	shr	cl,0x06
	xor	cl,ch		; xor swapping :)
	xor	ch,cl
	xor	cl,ch
	mov	[boot1_info+drv_cylinders],cx
	inc 	word [boot1_info+drv_cylinders]

	;; Preparation au chargement des secteurs
	mov	si,KERN_LBA_START
	mov	di,KERN_ABS_ADDR
	xor	ah,ah
	mov	dl,byte [boot1_info+drv_number]
	int	0x13
	jc	reset_error
	
chs_load:
	
	;; Conversion LBA vers CHS
	xor	dx,dx
	xor	cx,cx
	mov	ax,si	; Secteur LBA [0,inf[
	mov	cl,[boot1_info+drv_sectors]
	div	cx
	inc	dx
	mov	byte [sector_info+sector],dl
	xor	dx,dx
	xor	cx,cx
	mov	cl,byte [boot1_info+drv_heads]
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
	mov	dl,byte [boot1_info+drv_number]
	int	0x13
	jc 	read_error
	
	;; Increment de la boucle
	push 	si
	PRINT	BOOT1_LOAD_PROGRESS
	pop	si
	inc	si
	add	di,0x200
	cmp	si,KERN_LBA_END
	jle	chs_load
	PRINT	BOOT1_LOAD_OK
	
	
	;;========================================================================
	;; Suite: Decouverte de la memoire
	;;========================================================================

	
next01:
	;; Decouverte via int 0x15, ax=0xE820
	xor	si,si
	xor	ebx,ebx
	mov	es,ebx
	mov	di,BOOT1_MMAP_ADDR
	mov	ecx,BOOT1_MMAP_SIZE
	mov	dword [boot1_info+mem_map_addr],BOOT1_MMAP_ADDR
mem_e820:	
	mov	eax,0xE820
	mov	edx,0x534D4150
	int	0x15
	jc	mem_e801
	cmp	eax,0x534D4150
	jne	mem_e801
	inc	si
	add	di,BOOT1_MMAP_SIZE
	cmp	ebx,0x0
	jne	mem_e820

	;; Sauve les informations
	mov	word [boot1_info+mem_map_count],si
	jmp	next02		

mem_e801:
	mov	dword [boot1_info+mem_map_addr],0x0
	mov	word [boot1_info+mem_map_addr],0x0
	mov	ax,0xE801
	int	0x15
	jc	mem_e881

	;; Sauve les informations	
	mov	word [boot1_info+mem_lower],cx
	mov	word [boot1_info+mem_upper],dx
	jmp	next02
	
mem_e881:
	mov	ax,0xE881
	int	0x15
	jc	mem_e88

	;; Sauve les informations	
	mov	word [boot1_info+mem_lower],cx
	mov	word [boot1_info+mem_upper],dx	
	jmp	next02	
	
mem_e88:
	mov	word [boot1_info+mem_lower],0x0
	mov	word [boot1_info+mem_upper],0x0	
	mov	ax,0xE88
	int	0x15
	jc	memory_error
	mov	word [boot1_info+mem_0x100000],ax

	xor	ax,ax
	int	12
	mov	word [boot1_info+mem_0x0],ax

	
	;;========================================================================
	;; Suite: Activation de la ligne A20
	;;========================================================================
	
next02:
	;; Support du BIOS ?
	mov	ax,0x2403
	int 	0x15
	jc	a20_kb
	cmp	ah,0x0
	jne	a20_kb

	;; Ouverture via le BIOS
	mov	ax,0x2401
	int	0x15
	jc	a20_kb
	cmp	ah,0x0
	je	a20_chk

	;; Activation via le clavier
a20_kb:
	cli
	KB_WAIT_INPUT
	;; Mode Ecriture
        mov     al,0xD1
        out     BOOT1_KB_CMD,al
	KB_WAIT_INPUT
	;; Active la ligne A20
        mov     al,0xDF               
        out     BOOT1_KB_DATA,al
	KB_WAIT_INPUT
	KB_WAIT_OUTPUT
	sti

	;; Check via wrap around du magic number du bootsector
a20_chk:	
	push	es
	mov	ax,0xFFFF
	mov	es,ax
	mov	si,BOOT1_A20_WRAPADDR
	cmp	word [es:si],0xAA55
	pop	es
	je	a20_error
	

	;;========================================================================
	;; Suite: Passage en mode protege
	;;========================================================================

next03:
	;; Adresse physique de la GDT
	mov	dx,ds
	mov	ax,gdt
	call	real2phys
	mov	word [gdt_desc+2],ax
	mov	byte [gdt_desc+4],dl

	;; Adresse physique de boot1_info
	mov	dx,ds
	mov	ax,boot1_info
	call	real2phys
	mov	word [boot1_info_addr],ax
	mov	byte [boot1_info_addr+2],dl
	mov	ebx,dword  [boot1_info_addr]
	
	;; Charge la GDT
	cli
	lgdt	[gdt_desc]
	sti

	;; Active le mode protege
	cli
	mov     eax,cr0
	or      ax,0x1
	mov     cr0,eax
	jmp	next04

next04:
	;; Selecteurs
	mov     ax,BOOT1_XS_SELECTOR
	mov     ds,ax
	mov     es,ax
	mov     ss,ax
	mov     esp,BOOT1_ESP



	
	;;========================================================================
	;; Fin: Saut au noyau
	;;========================================================================
	
	jmp 	BOOT1_CS_SELECTOR:KERN_ABS_ADDR
	

	
	;;========================================================================
	;; Erreurs
	;;========================================================================

reset_error:
	PRINT	BOOT1_RESET_ERROR
	jmp	$
	
geometry_error:
	PRINT	BOOT1_GEO_ERROR
	jmp	$

read_error:
	PRINT	BOOT1_READ_ERROR
	jmp	$

memory_error:	
	PRINT	BOOT1_MEM_ERROR
	jmp	$

a20_error:	
	PRINT	BOOT1_A20_ERROR
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
	;; Calcul une adresse physique depuis une adresse reelle
	;;========================================================================

real2phys:			; Arg: DX=Segment, AX=Offset	
	mov 	cl,0x04
	mov 	ch,dh
	shr 	ch,cl
	shl 	dx,cl
	add 	ax,dx
	adc 	ch,0x00
	mov 	dl,ch
	xor 	dh,dh
	ret			; Adresse dans DX:AX
	

	;;========================================================================
	;;  Chaines
	;;========================================================================

	BOOT1_RESET_ERROR	db	"Reset Error",13,10,0
	BOOT1_GEO_ERROR		db	"Geometry Error",13,10,0
	BOOT1_READ_ERROR	db	"Read Error",13,10,0
	BOOT1_MEM_ERROR		db	"Memory Error",13,10,0
	BOOT1_A20_ERROR		db	"A20 Gate Error",13,10,0
	BOOT1_LOAD_PROGRESS	db	".",0
	BOOT1_LOAD_OK		db	"OK",13,10,0

	;;========================================================================
	;;  GDT 
	;;======================================================================

gdt_desc:
	dw	0x17		; Taille
	dd	0x00	 	; Adresse Physique

gdt:
	null_desc:
		db	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	cs_desc:	
		db 	0xFF,0xFF,0x00,0x00,0x00,0x9A,0xCF,0x00
	xs_desc:
		db 	0xFF,0xFF,0x00,0x00,0x00,0x92,0xCF,0x00
	
	
	;;========================================================================
	;;  Instances de Structures
	;;========================================================================

	
boot1_info:	istruc boot_info
		iend

boot1_info_addr:
		dd	0x00	; Adresse physique

sector_info:	istruc chs_info
		iend

sector_dap:	istruc disk_address_packet
		iend
	
	;;========================================================================
	;; Fin
	;;========================================================================

