/*
 * RhinOS Main
 *
 */


/*************
 * Includes 
 *************/

#include <types.h>
#include "start.h"
#include "klib.h"
#include "i82C54.h"
#include "proc.h"


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

  /* Idle Task */
  task_init(&proc_table[PROC_IDLE_INDEX], 
	    PROC_IDLE_INDEX, 
	    bootinfo.idle_offset, 
	    PROC_IDLE_CODE_SIZE,
	    PROC_IDLE_VADDR,
	    bootinfo.idle_offset+PROC_IDLE_CODE_SIZE,
	    PROC_IDLE_DATA_SIZE,
	    PROC_IDLE_VADDR,
	    PROC_IDLE_BSS_SIZE,
	    bootinfo.idle_offset+PROC_IDLE_CODE_SIZE+PROC_IDLE_DATA_SIZE,
	    PROC_IDLE_DATA_SIZE,
	    PROC_IDLE_PRIV, 
	    (u32_t)&idle_task,
	    (u32_t)&idle_task,
	    PROC_IDLE_TICKETS);

  /* Chargement du Memory Manager */
  task_elf((u32_t*)bootinfo.mm_offset,PROC_MM_FLAG);

  /* Initialisation du processus courant */
  proc_current = &proc_table[0];

  /* Gestion des taches */
  task_mgmt();

  /* On ne doit plus arriver ici */
  while(1)
    {
    }

  return;
}
