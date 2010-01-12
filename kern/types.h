#ifndef TYPES_H
#define TYPES_H

/* Portee des variables */

#define   PUBLIC
#define   PRIVATE  static
#define   EXTERN   extern

/* TRUE & FALSE */

#define   TRUE     1
#define   FALSE    0

/* NULL */

#define   NULL     ((void*)0)

/* Entiers de 8, 16 et 32 bits */

typedef unsigned char  u8_t;
typedef unsigned short u16_t;
typedef unsigned int   u32_t;

/* Alias pour le pointeur d'ISR */

typedef u8_t (*irq_handler_t)();

#endif
