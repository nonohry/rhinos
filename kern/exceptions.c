/*
 * Gestion des exceptions
 *
 */

#include <types.h>
#include "klib.h"
#include "exceptions.h"

/*========================================================================
 * Gestion des exceptions (generique)
 *========================================================================*/

PUBLIC void excep_handle(u32_t num)
{
  bochs_print("Exception %d !\n",num);
  while(1){}

  return;
}
