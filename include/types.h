/**

   types.h
   =======

   Header file for types definition

**/


#ifndef TYPES_H
#define TYPES_H




/**
   
   Base types
   ----------

   Define signed & unsigned 8 16 32 and 64 bits integers 

**/

typedef unsigned char  u8_t;
typedef unsigned short u16_t;
typedef unsigned int   u32_t;
typedef unsigned long long  u64_t;

typedef char  s8_t;
typedef short s16_t;
typedef int   s32_t;
typedef long long int s64_t;


/**
   
   Adresses
   --------

   Define the types of adresses used in RhinOS:

   - physaddr_t : Physical address
   - lineaddr_t : Linear address
   - virtaddr_t : Virtual address
   - addr_t     : Generic address
   - reg32_t    : 32 bits register
   - reg16_t    : 16 bits register
    
**/

typedef unsigned int   physaddr_t;
typedef unsigned int   lineaddr_t;
typedef unsigned int   virtaddr_t;
typedef unsigned int   addr_t;
typedef unsigned int   reg32_t;
typedef unsigned short reg16_t;



#endif
