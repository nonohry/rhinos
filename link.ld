OUTPUT_FORMAT("elf32-i386")
offset = 0x100000;
ENTRY(_start)
SECTIONS
{	
	/* Kernel loaded at 0x100000 */
	. = offset;

	/* Provide start symbols */
	PROVIDE(__CONST_KERN_START = .);
	PROVIDE(___CONST_KERN_START = .);
	
	/* Code and RO data */
	.text : 
	{
		*(.text)
    		*(.rodata)
		PROVIDE(__CONST_TEXT_END = .);
		PROVIDE(___CONST_TEXT_END = .);
		. = ALIGN(4096);
      	}

	/* Initialized data */
	.data offset + SIZEOF(.text) :
  	{
		*(.data)
		PROVIDE(__CONST_DATA_END = .);
		PROVIDE(___CONST_DATA_END = .);
		. = ALIGN(4096);
 	}

	/* Non initialized data */
  	.bss offset + SIZEOF(.text) + SIZEOF(.data) :
  	{
		*(.bss)
		PROVIDE(__CONST_BSS_END = .);
		PROVIDE(___CONST_BSS_END = .);
		. = ALIGN(4096);
	}

	/* Provide end symbols */
	PROVIDE(__CONST_KERN_END = .);
	PROVIDE(___CONST_KERN_END = .);
}
