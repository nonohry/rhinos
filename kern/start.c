/*
 * Code de demarrage en C 
 * Recharge GDT et IDT 
 *
 */

#include "types.h"
#include "klib.h"
#include "prot.h"


PUBLIC void cstart()
{
  
  bochs_print("Fonction cstart\n");

  pmode_init();

  return;

}
