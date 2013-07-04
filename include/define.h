/**

   define.h
   ========

   Common definitions

**/


#ifndef DEFINE_H
#define DEFINE_H


/**

   Scopes 
   ------

   Define 3 scopes:
   
   - PUBLIC  : universal scope
   - PRIVATE : in source file scope
   - EXTERN  : external scope


**/

#define   PUBLIC
#define   PRIVATE  static
#define   EXTERN   extern


/**

   Boolean values
   --------------

   Define 2 values:
    - TRUE  (self explanatory)
    - FALSE (self explanatory)

**/

#define   TRUE     1
#define   FALSE    0


/**

   Return values
   -------------

   Define 2 generic value for returning functions:
   
   - EXIT_SUCCESS : No error
   - EXIT_FAILURE : Something wrong ...

**/

#define   EXIT_SUCCESS   1
#define   EXIT_FAILURE   0


/**

   NULL 
   ----

   Define NULL as nil void pointer

**/

#define   NULL     ((void*)0)


#endif
