/*
 * Gestion des exceptions
 *
 */

#include "types.h"
#include "klib.h"
#include "exceptions.h"

/**************************************
 * Gestion des exceptions (generique)
 **************************************/

PUBLIC void excep_handle(u32_t num, u32_t code)
{
  int i;

  for(i=0;i<=num;i++)
    {
      bochs_print(" Exception ");
    }

  return;
}
