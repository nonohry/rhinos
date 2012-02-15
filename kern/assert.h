/*
 * Assert.h
 * Macros assertions
 *
 */

#ifndef ASSERTION_H
#define ASSERTION_H

#include "klib.h"

/* Convertit une expression en chaine */
#define TO_STRING(__X)  #__X


/* Verifie une assertion */
#define ASSERT(__expr)							\
  {									\
    if ( !(__expr) )							\
      {									\
	klib_bochs_print(" ");					\
      }									\
  }


/* Verifie une assertion et retourne */
#define ASSERT_RETURN(__expr, __ret)					\
  {									\
    if ( !(__expr) )							\
      {									\
	return  __ret;							\
      }									\
  }


/* Verifie une assertion et retourne void */
#define ASSERT_RETURN_VOID(__expr)     ASSERT_RETURN(__expr,)


/* Verifie une assertion et bloque */
#define ASSERT_FATAL(__expr)						\
  {									\
    if ( !(__expr) )							\
      {									\
	klib_bochs_print("Fatal assertion [ ");				\
	klib_bochs_print(TO_STRING(__expr));					\
	klib_bochs_print(" ] failed ! file ");				\
	klib_bochs_print(__FILE__);						\
	klib_bochs_print(", line %d\n",__LINE__);				\
	while(1){};							\
      }									\
  }


#endif
