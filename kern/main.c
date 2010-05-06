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

  /* Initialisation de la dummy task */
  task_init(&proc_table[0], 0xF00000, 1024, 3, (u32_t)&dummy,10);
  task_init(&proc_table[1], 0xE00000, 1024, 3, (u32_t)&dummy,20);
  task_init(&proc_table[2], 0xD00000, 1024, 3, (u32_t)&dummy,30);
  task_init(&proc_table[3], 0xC00000, 1024, 3, (u32_t)&dummy,40);
  proc_current = &proc_table[3];

  /* DEBUG - Affiche la skip list */
  sched_print(&proc_ready);

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
