/*
 * Mode Protege
 * Mise en place des tables
 *
 */

#include "types.h"
#include "prot.h"

/* Declarations preliminaires */

EXTERN void bochs_print(char*);

/* Fonction principale */
 
PUBLIC void pmode_init()
{

  gdt_desc.limit = GDT_SIZE*8;  /* GDT_SIZE descripteurs de 8 octets */
  /* gdt_desc.base = linear(gdt);   Adresse de gdt dans l espace lineaire */

  return;
}
