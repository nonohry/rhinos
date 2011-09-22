/*
 * Gestion du contexte cpu
 *
 */


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "klib.h"
#include "context_cpu.h"


/*========================================================================
 * Pre traitement de la sauvegarde du contexte
 *========================================================================*/

PUBLIC void context_cpu_presave(reg32_t ss, reg32_t* esp)
{
  /* esp+1 pour sauter l'adresse de retour de [hwint|excep]_save */
  bochs_print("ss=0x%x et esp=0x%x (valeur: 0x%x)\n",ss,(reg32_t)(esp+1), *(esp+1));
  while(1){};
  return;
}
