#ifndef ELF_H
#define ELF_H

#include <types.h>


/*************
 * Constantes
 *************/

/* Identification */

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

/* Type */

#define ET_NONE       0
#define ET_REL        1
#define ET_EXEC       2
#define ET_DYN        3
#define ET_CORE       4
#define ET_LOPROC     0xFF00
#define ET_HIPROC     0xFFFF

/* Architecture */

#define EM_NONE       0
#define EM_M32        1
#define EM_SPARC      2
#define EM_386        3
#define EM_68K        4
#define EM_88K        5
#define EM_860        7
#define EM_MIPS       8

/* Version */

#define EV_NONE       0
#define EV_CURRENT    1


/**************
 * ELF Header
 **************/

PUBLIC struct elf_header
{
  unsigned char e_ident[EI_NIDENT]; /* Identification */
  u16_t e_type;                     /* Type de fichier */
  u16_t e_machine;                  /* Architecture processeur */
  u32_t e_version;                  /* Version du ELF */
  u32_t e_entry;                    /* Point d entree (adresse virtuelle) */
  u32_t e_phoff;                    /* Offset du Program Header */
  u32_t e_shoff;                    /* Offset du Section Header */
  u32_t e_flags;                    /* Flags processeur */
  u16_t e_ehsize;                   /* Taille du ELF Header */
  u16_t e_phentsize;                /* Taille d une entree du Program Header (en octet) */
  u16_t e_phnum;                    /* Nombre d entrees du Program Header */
  u16_t e_shentsize;                /* Taille d une entree du Section Header (en octet) */
  u16_t e_shnum;                    /* Nombre d entrees du Section Header */
  u16_t e_shstrndx;                 /* Index dans le section header de la table des symboles */
}__attribute__ ((packed));


/******************
 * Program Header
 ******************/

PUBLIC struct prog_header
{
  u32_t p_type;                     /* Type de segment */
  u32_t p_offset;                   /* Offset du segment dans le fichier */
  u32_t p_vaddr;                    /* Adresse virtuelle du premier octet */
  u32_t p_paddr;                    /* Adresse physique du premier octet */
  u32_t p_filesz;                   /* Taille en octet du segment dans le fichier */
  u32_t p_memsz;                    /* Taille en octet du segment en memoire */
  u32_t p_flags;                    /* Flags du segment */
  u32_t p_align;                    /* Valeur de l alignement */
}__attribute__ ((packed));


#endif
