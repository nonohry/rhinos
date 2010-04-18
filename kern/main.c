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

  struct proc dummy_proc;

  /* Initialisation Horloge */
  clock_init();
  bochs_print("Clock initialized (100Hz)\n");
 
  /* Initialisation de l'ordonnancement */
  sched_init(&proc_ready);

  /* Initialisation de la dummy task */
  task_init(&dummy_proc, 0x9C00, 1024, 1024, 3, (u32_t)&dummy);
  proc_current = &dummy_proc;

  /* Gestion des taches */
  task_mgmt();

  /* On ne doit plus ariver ici */
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
