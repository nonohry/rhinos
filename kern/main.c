/*
 * RhinOS Main
 *
 */


/*************
 * Includes 
 *************/

#include "types.h"
#include "klib.h"
#include "i82C54.h"


/***********************
 * Fonction principale 
 ***********************/

PUBLIC void main()
{

  /* Initialisation Horloge */
  clock_init();
  bochs_print("Clock initialized (100Hz)\n");
 
  while(1)
    {
    }

  return;
}
