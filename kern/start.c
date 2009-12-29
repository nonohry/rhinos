/*
 * Code de demarrage en C 
 * Recharge GDT et IDT 
 *
 */


/*************
 * Includes 
 *************/

#include "types.h"
#include "klib.h"
#include "prot.h"

/*******************
 * Fonction cstart
 *******************/

PUBLIC void cstart()
{
  
  pmode_init();

  return;

}
