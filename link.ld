OUTPUT_FORMAT("elf32-i386")
offset = 0x100000;
ENTRY(_start)
SECTIONS
{	
	/* Noyau Charge a 0x100000 */
	. = offset;

	/* Fournit les adresses de debut */
	PROVIDE(kernel_start = .);
	PROVIDE(_kernel_start = .);
	
	/* La section de code et de donnees read-only */
	.text : 
	{
		*(.text)
    		*(.rodata)
		PROVIDE(text_end = .);
		PROVIDE(_text_end = .);
		. = ALIGN(4096);
      	}

	/* La section de donnees initialisees */
	.data offset + SIZEOF(.text) :
  	{
		*(.data)
		PROVIDE(data_end = .);
		PROVIDE(_data_end = .);
		. = ALIGN(4096);
 	}

	/* La section de donnees non initialisees */
  	.bss offset + SIZEOF(.text) + SIZEOF(.data) :
  	{
		*(.bss)
		PROVIDE(bss_end = .);
		PROVIDE(_bss_end = .);
		. = ALIGN(4096);
	}

	/* Fournit l adresse de fin du noyau */
	PROVIDE(kernel_end = .);
	PROVIDE(_kernel_end = .);
}