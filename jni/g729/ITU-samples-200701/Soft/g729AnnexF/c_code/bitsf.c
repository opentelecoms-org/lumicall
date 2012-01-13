/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex F  - Reference C code for fixed point
                         implementation of G.729 Annex F
                         (integration of Annexes B and D)
                         Version 1.1 of October 1999
*/
/*
 File : bitsf.c
 */

/* from bitsd.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from bits.c G.729 Annex B Version 1.3  Last modified: August 1997 */
/* from bits.c G.729 Version 3.3            */


/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "ld8f.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "tabld8kd.h"
#include "tab_ld8k.h"
#include "octet.h"

/* prototypes for local functions */
static void  int2bin(Word16 value, Word16 no_of_bits, Word16 *bitstream);
static Word16   bin2int(Word16 no_of_bits, Word16 *bitstream);
/*----------------------------------------------------------------------------
* prm2bits_ld8f -converts encoder parameter vector into vector of serial bits
* bits2prm_ld8f - converts serial received bits to  encoder parameter vector
*----------------------------------------------------------------------------
*
* G.729 main Recommendation: 8 kb/s
*
* The transmitted parameters are:
*
*     LPC:     1st codebook           7+1 bit
*              2nd codebook           5+5 bit
*
*     1st subframe:
*          pitch period                 8 bit
*          parity check on 1st period   1 bit
*          codebook index1 (positions) 13 bit
*          codebook index2 (signs)      4 bit
*          pitch and codebook gains   4+3 bit
*
*     2nd subframe:
*          pitch period (relative)      5 bit
*          codebook index1 (positions) 13 bit
*          codebook index2 (signs)      4 bit
*          pitch and codebook gains   4+3 bit
*
*----------------------------------------------------------------------------
*
* G.729 Annex D: 6.4 kb/s
*
* The transmitted parameters are:
*
*     LPC:     1st codebook           7+1 bit
*              2nd codebook           5+5 bit
*
*     1st subframe:
*          pitch period                 8 bit
*          codebook index1 (positions)  9 bit
*          codebook index2 (signs)      2 bit
*          pitch and codebook gains   3+3 bit
*
*     2nd subframe:
*          pitch period (relative)      4 bit
*          codebook index1 (positions)  9 bit
*          codebook index2 (signs)      2 bit
*          pitch and codebook gains   3+3 bit
*
*----------------------------------------------------------------------------
*/

void prm2bits_ld8f(
    Word16 prm[],            /* input : encoded parameters  */
    Word16 bits[]           /* output: serial bits         */
)
{
    Word16 i;
    Word16 *ptr_bits;
    
    ptr_bits  = bits;
    *ptr_bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */
        
        
    switch(prm[0]){
                
                
        case 0 : {        /* DTX: no bits transmitted */
            *ptr_bits = RATE_0;
            break;
        }

        case 1 : {        /* SID: 15 bits (or 16 bits if OCTET mode) */

#ifndef OCTET_TX_MODE
            *ptr_bits++ = RATE_SID;
            for (i = 0; i < PRM_SIZE_SID; i++) {
                int2bin(prm[i+1], bitsno2[i], ptr_bits);
                ptr_bits += bitsno2[i];
            }
#else
            *ptr_bits++ = RATE_SID_OCTET;
            for (i = 0; i < PRM_SIZE_SID; i++) {
                int2bin(prm[i+1], bitsno2[i], ptr_bits);
                ptr_bits += bitsno2[i];
            }
            *ptr_bits++ = BIT_0;
#endif

            break;
         }

        case 2 : {
            *ptr_bits++ = RATE_6400;
            for (i = 0; i < PRM_SIZE_6K; i++) {
                int2bin(prm[i+1], bitsno_6k[i], ptr_bits);
                ptr_bits += bitsno_6k[i];
            }
            break;
         }

        case 3 : {
            *ptr_bits++ = RATE_8000;
            for (i = 0; i < PRM_SIZE; i++) {
                int2bin(prm[i+1], bitsno[i], ptr_bits);
                ptr_bits += bitsno[i];
            }
            break;
         }
            
        default : {
            printf("Unrecognized frame type\n");
            exit(-1);
      }
        
    }
    
    return;
}


/*----------------------------------------------------------------------------
 * int2bin convert integer to binary and write the bits bitstream array
 *----------------------------------------------------------------------------
 */
static void int2bin(
 Word16 value,             /* input : decimal value */
 Word16 no_of_bits,        /* input : number of bits to use */
 Word16 *bitstream       /* output: bitstream  */
)
{
    Word16 *pt_bitstream;
    Word16   i, bit;
    
    pt_bitstream = bitstream + no_of_bits;
    
    for (i = 0; i < no_of_bits; i++)
    {
        bit = value & (Word16)0x0001;      /* get lsb */
        if (bit == 0)
            *--pt_bitstream = BIT_0;
        else
            *--pt_bitstream = BIT_1;
        value >>= 1;
    }
}

/*----------------------------------------------------------------------------
 *  bits2prm_ld8f - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void bits2prm_ld8f(
    Word16 bits[],  /* I   serial bits (nb_bits)                          */
    Word16 prm[]   /* O   output: decoded parameters (11 parameters)     */
)

{
    Word16 i;
    Word16 nb_bits;

    nb_bits = *bits++;        /* Number of bits in this frame       */

    switch(nb_bits){
        case RATE_0 : {        /* DTX: no bits transmitted */
            prm[1] = 0;
            break;
         }
                
#ifndef OCTET_TX_MODE
        case RATE_SID : {        /* SID: 15 bits (not in OCTET mode) */
            prm[1] = 1;
            for (i = 0; i < PRM_SIZE_SID; i++) {
                prm[i+2] = bin2int(bitsno2[i], bits);
                bits += bitsno2[i];
            }
            break;
        }
#else
            /* the last bit of the SID bit stream under octet mode will be discarded */
        case RATE_SID_OCTET : {        /* SID: 16 bits (OCTET mode) */
            prm[1] = 1;
            for (i = 0; i < PRM_SIZE_SID; i++) {
                prm[i+2] = bin2int(bitsno2[i], bits);
                bits += bitsno2[i];
            }
            break;
        }
#endif
        case RATE_6400 : {        /* G729 Annex D: 64 bits*/
            prm[1] = 2;
            for (i = 0; i < PRM_SIZE_6K; i++) {
                prm[i+2] = bin2int(bitsno_6k[i], bits);
                bits += bitsno_6k[i];
            }
            break;
        }
                
        case RATE_8000 : {        /* G729: 80 bits*/
            prm[1] = 3;
            for (i = 0; i < PRM_SIZE; i++) {
                prm[i+2] = bin2int(bitsno[i], bits);
                bits += bitsno[i];
            }
            break;
         }

        default : {
            printf("Unrecognized frame type: uncorrect value of serial_size\n");
            exit(-1);
        }
            
    }
        
    return;
}

/*----------------------------------------------------------------------------
 * bin2int - read specified bits from bit array  and convert to integer value
 *----------------------------------------------------------------------------
 */
static Word16 bin2int(            /* output: decimal value of bit pattern */
 Word16 no_of_bits,        /* input : number of bits to read */
 Word16 *bitstream       /* input : array containing bits */
)
{
    Word16   value, i;
    Word16 bit;
    
    value = 0;
    for (i = 0; i < no_of_bits; i++)
    {
        value <<= 1;
        bit = *bitstream++;
        if (bit == BIT_1)  value += 1;
    }
    return(value);
}
