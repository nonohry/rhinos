OUTPUT_FORMAT("binary")
ENTRY(_start)
offset = 0xC00;
SECTIONS
{
  .text offset : AT(offset) {
    code = .;
    *(.text)
    *(.rodata)
    . = ALIGN(4096);
  }
  .data : AT(offset + (data - code))
  {
    data = .;
    *(.data)
    . = ALIGN(4096);
  }
  .bss : AT(offset + (bss - code))
  {
    bss = .;
    *(.bss)
    . = ALIGN(4096);
  }
  kernel_end = .;
}