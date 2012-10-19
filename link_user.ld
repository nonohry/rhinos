OUTPUT_FORMAT("binary")
offset = 0x80000000;
ENTRY(_start)
SECTIONS
{	
	/* Programme Charge a 0x80000000 (adresse virtuelle) */
	. = offset;

	/* Fournit les adresses de debut */
	PROVIDE(user_start = .);
	PROVIDE(_user_start = .);
	
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
	PROVIDE(user_end = .);
	PROVIDE(_user_end = .);
}