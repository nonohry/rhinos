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
#include "proc.h"

/*********
 * DEBUG
 *********/

PUBLIC void dummy();

/**********
 * EXTERN
 **********/

EXTERN void task_mgmt();


/***********************
 * Fonction principale 
 ***********************/

PUBLIC void main()
{
  /* Initialisation Horloge */
  clock_init();
  bochs_print("Clock initialized (100Hz)\n");

  /* Initialisation de l'ordonnancement */
  sched_init(&proc_ready);
  bochs_print("Lottery Scheduling initialized\n");

  /* Initialisation de la dummy task */
  task_init(&proc_table[0], 0xF00000, 1024, 3, (u32_t)&dummy,10);
  proc_current = &proc_table[0];

  /* Gestion des taches */
  task_mgmt();

  /* On ne doit plus arriver ici */
  while(1)
    {
    }

  return;
}


/**************
 * Dummy task
 **************/

PUBLIC void dummy()
{
  while(1)
    {
    }

  return;
}
