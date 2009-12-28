/*
 * RhinOS Main
 *
 */

#include "types.h"

/****************************** 
 * Declarations preliminaires 
 *****************************/

EXTERN void bochs_print(char*);

/***********************
 * Fonction principale 
 ***********************/

PUBLIC void main()
{
  bochs_print("Fonction main\n");

  while(1)
    {
    }

  return;
}
