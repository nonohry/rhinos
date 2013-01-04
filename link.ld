OUTPUT_FORMAT("elf32-i386")
offset = 0x100000;
ENTRY(_start)
SECTIONS
{	
	/* Noyau Charge a 0x100000 */
	. = offset;

	/* Fournit les adresses de debut */
	PROVIDE(__CONST_KERN_START = .);
	PROVIDE(___CONST_KERN_START = .);
	
	/* La section de code et de donnees read-only */
	.text : 
	{
		*(.text)
    		*(.rodata)
		PROVIDE(__CONST_TEXT_END = .);
		PROVIDE(___CONST_TEXT_END = .);
		. = ALIGN(4096);
      	}

	/* La section de donnees initialisees */
	.data offset + SIZEOF(.text) :
  	{
		*(.data)
		PROVIDE(__CONST_DATA_END = .);
		PROVIDE(___CONST_DATA_END = .);
		. = ALIGN(4096);
 	}

	/* La section de donnees non initialisees */
  	.bss offset + SIZEOF(.text) + SIZEOF(.data) :
  	{
		*(.bss)
		PROVIDE(__CONST_BSS_END = .);
		PROVIDE(___CONST_BSS_END = .);
		. = ALIGN(4096);
	}

	/* Fournit l adresse de fin du noyau */
	PROVIDE(__CONST_KERN_END = .);
	PROVIDE(___CONST_KERN_END = .);
}