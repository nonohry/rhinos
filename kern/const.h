/*
 * Const.h
 * Constantes partagees du noyau
 *
 */


/*========================================================================
 * Memoire
 *========================================================================*/


#define CONST_PAGE_SIZE              4096
#define CONST_PAGE_SHIFT             12         /* 2^12=4096    */
#define CONST_PAGE_NODE_POOL_ADDR    0x100000


#define CONST_CS_SELECTOR            8    /*  CS = 0000000000001  0  00   =  8  */
#define	CONST_DS_SELECTOR            16   /*  DS = 0000000000010  0  00   =  16 */
#define	CONST_ES_SELECTOR	     16   /*  ES = 0000000000010  0  00   =  16 */
#define	CONST_SS_SELECTOR	     16   /*  SS = 0000000000010  0  00   =  16 */
#define CONST_TSS_SELECTOR           24   /* TSS = 0000000000011  0  00   =  24 */
