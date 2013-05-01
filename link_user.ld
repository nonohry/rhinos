OUTPUT_FORMAT("binary")
offset = 0x80000000;
ENTRY(_start)
SECTIONS
{	
	/* Program loaded a 0x80000000 (virtual address) */
	. = offset;

	/* Provide start symbols */
	PROVIDE(user_start = .);
	PROVIDE(_user_start = .);
	
	/* Code and RO data */
	.text : 
	{
		*(.text)
    		*(.rodata)
		PROVIDE(text_end = .);
		PROVIDE(_text_end = .);
		. = ALIGN(4096);
      	}

	/* Initialized data */
	.data offset + SIZEOF(.text) :
  	{
		*(.data)
		PROVIDE(data_end = .);
		PROVIDE(_data_end = .);
		. = ALIGN(4096);
 	}

	/* Non initialized data */
  	.bss offset + SIZEOF(.text) + SIZEOF(.data) :
  	{
		*(.bss)
		PROVIDE(bss_end = .);
		PROVIDE(_bss_end = .);
		. = ALIGN(4096);
	}

	/* Provide end symbols */
	PROVIDE(user_end = .);
	PROVIDE(_user_end = .);
}