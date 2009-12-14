[BITS 32]

	
	mov	dx,0xe9
	mov	ax,'P'
	out	dx,ax
	mov	ax,'m'
	out	dx,ax
	mov	ax,'o'
	out	dx,ax
	mov	ax,'d'
	out	dx,ax
	mov	ax,'e'
	out	dx,ax
	
start:
	
	jmp	start