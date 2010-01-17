/*
 * RhinOS Main
 *
 */


/*************
 * Includes 
 *************/

#include "types.h"
#include "klib.h"
#include "i8259.h"


/**************************
 * Dummy irq (debug only)
 **************************/

PUBLIC u8_t irq_dummy()
{
  bochs_print("Dummy\n");
  return TRUE;
}

PUBLIC u8_t irq_dummyy()
{
  static int tic = 0;
  static int sec = 0;
  tic++;
  if (tic % 100 == 0) {
    sec++;
    tic = 0;
    bochs_print("clock\n");
  }

  return TRUE;
}


/***********************
 * Fonction principale 
 ***********************/

PUBLIC void main()
{
  bochs_print("Fonction main\n");

  // Dummy Keyboard (debug)

  struct irq_chaine toto;
  struct irq_chaine tata;
  struct irq_chaine titi;
  struct irq_chaine tutu;

  irq_add_handler(0,irq_dummyy,&toto);
  irq_add_handler(1,irq_dummy,&titi);
  irq_add_handler(1,irq_dummy,&tata);
  irq_add_handler(1,irq_dummy,&tutu);

  // Fin Dummy

  while(1)
    {
    }

  return;
}
