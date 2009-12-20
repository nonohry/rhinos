/*
 * Code de demarrage en C 
 * Recharge GDT et IDT 
 *
 */

#include "types.h"
#include "prot.h"

/*
 * Declaration des fonctions assembleur
 * definies dans khead.s
 *
 */

EXTERN void bochs_print(char*);

PUBLIC void cstart()
{
  
  bochs_print("Fonction cstart\n");

  pmode_init();

  return;
}
