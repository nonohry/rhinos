/*
 * Code de demarrage en C 
 * Recharge GDT et IDT 
 *
 */

#include "types.h"

/*
 * Declaration des fonctions assembleur
 * definies dans lowlvl.s
 *
 */

extern void _bochs_print(char*);


void cstart()
{
  
  _bochs_print("Protected Mode enabled !\n");

}
