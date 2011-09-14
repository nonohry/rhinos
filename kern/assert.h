/*
 * Assert.h
 * Macros assertions
 *
 */

#ifndef ASSERTION_H
#define ASSERTION_H

#include "klib.h"

/* Convertit une expression en chaine */
#define TO_STRING(X)  #X


/* Verifie une assertion */
#define ASSERT(__expr)							\
  {									\
    if ( !(__expr) )							\
      {									\
	bochs_print("Assertion [");					\
	bochs_print(TO_STRING(__expr));					\
	bochs_print("] failed ! file ");				\
	bochs_print(__FILE__);						\
	bochs_print(", line %d\n",__LINE__);				\
      }									\
  }


/* Verifie une assertion et retourne */
#define ASSERT_RETURN(__expr, __ret)					\
  {									\
    if ( !(__expr) )							\
      {									\
	bochs_print("Assertion [");					\
	bochs_print(TO_STRING(__expr));					\
	bochs_print("] failed ! file ");				\
	bochs_print(__FILE__);						\
	bochs_print(", line %d\n",__LINE__);				\
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
	bochs_print("Fatal assertion [");				\
	bochs_print(TO_STRING(__expr));					\
	bochs_print("] failed ! file ");				\
	bochs_print(__FILE__);						\
	bochs_print(", line %d\n",__LINE__);				\
	while(1){};							\
      }									\
  }


#endif
