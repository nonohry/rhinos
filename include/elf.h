/**

   elf.h
   =====

   Constantes et structures for elf manipulation

**/

#ifndef ELF_H
#define ELF_H


/**

   Includes
   --------

   - types.h

**/


#include <types.h>


/**

   Constantes: ELF Header Identification
   -------------------------------------

**/


#define EI_NIDENT     16
#define EI_MAG0       0
#define EI_MAG1       1
#define EI_MAG2       2
#define EI_MAG3       3
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_PAD        7

#define ELFMAG0       0x7F
#define ELFMAG1       'E'
#define ELFMAG2       'L'
#define ELFMAG3       'F'

#define ELFCLASSNONE  0
#define ELFCLASS32    1
#define ELFCLASS64    2

#define ELFDATANONE   0
#define ELFDATA2LSB   1
#define ELFDATA2MSB   2


/**

   Constantes: ELF Header Type 
   ---------------------------

**/

#define ET_NONE       0
#define ET_REL        1
#define ET_EXEC       2
#define ET_DYN        3
#define ET_CORE       4
#define ET_LOPROC     0xFF00
#define ET_HIPROC     0xFFFF


/**

   Constantes: ELF Header Architecture 
   -----------------------------------

**/

#define EM_NONE       0
#define EM_M32        1
#define EM_SPARC      2
#define EM_386        3
#define EM_68K        4
#define EM_88K        5
#define EM_860        7
#define EM_MIPS       8


/**

   Constantes: ELF Header Version 
   ------------------------------

**/

#define EV_NONE       0
#define EV_CURRENT    1


/**
   
   Constantes: Program Header Type 
   -------------------------------

**/

#define PT_NULL       0
#define PT_LOAD       1
#define PT_DYNAMIC    2
#define PT_INTERP     3
#define PT_NOTE       4
#define PT_SHLIB      5
#define PT_PHDR       6
#define PT_LOPROC     0x70000000
#define PL_HIPROC     0x7FFFFFFF


/**
 
  Constantes: Program Header Flags 
  --------------------------------

**/

#define PF_R            0x4
#define PF_W            0x2
#define PF_X            0x1
#define PF_MASKOS       0x0ff00000
#define PF_MASKPROC     0xf0000000


/**

   Structure: struct elf_header
   ----------------------------

   Describe an elf header. Members are:

   - e_ident[EI_NIDENT]         : identification
   - e_type                     : file type
   - e_machine                  : processor architecture
   - e_version                  : version 
   - e_entry                    : entry point (virtual addr)
   - e_phoff                    : program header offset 
   - e_shoff                    : section header offset 
   - e_flags                    : processor flags 
   - e_ehsize                   : ELF header size 
   - e_phentsize                : program header entry size (in bytes) 
   - e_phnum                    : program header entries number
   - e_shentsize                : section header entry size (in bytes) 
   - e_shnum                    : section header entries number
   - e_shstrndx                 : symbol table index 

**/

PUBLIC struct elf_header
{
  unsigned char e_ident[EI_NIDENT];
  u16_t e_type;
  u16_t e_machine;
  u32_t e_version;
  u32_t e_entry;
  u32_t e_phoff;
  u32_t e_shoff;
  u32_t e_flags;
  u16_t e_ehsize;
  u16_t e_phentsize;
  u16_t e_phnum;
  u16_t e_shentsize;
  u16_t e_shnum;
  u16_t e_shstrndx;
}__attribute__ ((packed));


/**

   Structure: struct prog_header
   -----------------------------

   Describe an ELF program header. Members are:
   
     - p_type                     : segment type 
     - p_offset                   : segment offset
     - p_vaddr                    : firts byte virtual address
     - p_paddr                    : first byte physical address
     - p_filesz                   : in file segment size (in bytes)
     - p_memsz                    : in memory segment size (in byte)
     - p_flags                    : flags
     - p_align                    : alignement 

**/


PUBLIC struct prog_header
{
  u32_t p_type;
  u32_t p_offset;
  u32_t p_vaddr;
  u32_t p_paddr;
  u32_t p_filesz;
  u32_t p_memsz;
  u32_t p_flags;
  u32_t p_align;
}__attribute__ ((packed));


#endif
