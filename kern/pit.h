#ifndef PIT_H
#define PIT_H

#include <types.h>

/* Constantes */

#define PIT_MAX_FREQ     1193182    /* Frequence maximal du PIT */
#define PIT_FREQ         100        /* Frequence a 100Hz */

/* I/O Ports */

#define PIT_COUNTER0     0x40
#define PIT_COUNTER1     0x41
#define PIT_COUNTER2     0x42
#define PIT_CWREG        0x43

/* Mot de Controle */

#define PIT_MODE2        0x34   /* 0x34=00110100b, Mode 2 pour le compteur 0 */
#define PIT_MODE3        0x36   /* 0x36=00110110b, Mode 3 pour le compteur 0 */

/* Counter Latch */

#define PIT_LATCH        0x00   /* 00000000b Counter Latch pour le compteur 0 */


/* Prototypes */

PUBLIC void clock_init();
PUBLIC u16_t clock_read();

#endif
