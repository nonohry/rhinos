#ifndef TYPES_H
#define TYPES_H

/* Portee des variables */

#define   PUBLIC
#define   PRIVATE  static
#define   EXTERN   extern

/* TRUE & FALSE */

#define   TRUE     1
#define   FALSE    0

/* SUCCESS & FAILURE */

#define   EXIT_SUCCESS   1
#define   EXIT_FAILURE   0

/* NULL */

#define   NULL     ((void*)0)

/* Entiers de 8, 16 et 32 bits */

typedef unsigned char  u8_t;
typedef unsigned short u16_t;
typedef unsigned int   u32_t;
typedef unsigned long long int u64_t;

typedef char  s8_t;
typedef short s16_t;
typedef int   s32_t;
typedef long long int s64_t;


/* Adresses */

typedef unsigned int   physaddr_t;
typedef unsigned int   lineaddr_t;
typedef unsigned int   virtaddr_t;
typedef unsigned int   addr_t;
typedef unsigned int   reg32_t;
typedef unsigned short reg16_t;

/* Contexte */
typedef void (*thread_cpu_func_t)(void*);

#endif
