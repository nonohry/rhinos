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

PUBLIC void cstart(u16_t lower, u16_t upper)
{ 
  pmode_init();

  return;

}
