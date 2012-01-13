/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.2    Last modified: May 1998 */
/* from bits.c G.729 Version 3.3            */

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "ld8k.h"
#include "ld8e.h"
#include "tab_ld8e.h"
#include "tab_ld8k.h"

/* prototypes for local functions */
static void  int2bin(Word16 value, Word16 no_of_bits, Word16 *bitstream);
static Word16   bin2int(Word16 no_of_bits, Word16 *bitstream);

/*----------------------------------------------------------------------------
 * prm2bits_ld8e -converts encoder parameter vector into vector of serial bits
 * bits2prm_ld8e - converts serial received bits to  encoder parameter vector
 *
 * The transmitted parameters in forward mode are:
 *
 *     mode  (including parity)        1+1 bit
 *
 *     LPC:     1st codebook           7+1 bit
 *              2nd codebook           5+5 bit
 *
 *     1st subframe:
 *          pitch period                 8 bit
 *          parity check on 1st period   1 bit
 *          codebook index             7x5 bit
 *          pitch and codebook gains   4+3 bit
 *
 *     2nd subframe:
 *          pitch period (relative)      5 bit
 *          codebook index             7x5 bit
 *          pitch and codebook gains   4+3 bit
 *
 * The transmitted parameters in backward mode are:
 *
 *     mode  (including parity)        1+1 bit
 *
 *     1st subframe:
 *          pitch period                 8 bit
 *          parity check on 1st period   1 bit
 *          codebook index     13+10+7+7+7 bit
 *          pitch and codebook gains   4+3 bit
 *
 *     2nd subframe:
 *          pitch period (relative)      5 bit
 *          codebook index     13+10+7+7+7 bit
 *          pitch and codebook gains   4+3 bit
 *----------------------------------------------------------------------------
 */
void prm2bits_ld8e(
  Word16 prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
  Word16 bits[],          /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = nbbits */
  Word16 rate           /* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps*/
)

{

    Word16 i, *p_bitsno;
    Word16 prm_size;
    Word16 *ptr_bits, *ptr_prm;

    ptr_bits  = bits;
    *ptr_bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */
    *ptr_bits++ = bitrates[rate];        /* bit[1], to be compatible with hardware */

    ptr_prm = prm;
    if( rate == 0) {
        /* case 8 kbps */
        prm_size = PRM_SIZE;
        p_bitsno = bitsno;
    }
    else {
        /* case 11.8 kbps */
        if((*ptr_prm++) == 0) { /* mode parameter */
            *ptr_bits++ = BIT_0;     /* "Backward / Forward" Indication bit */
            *ptr_bits++ = BIT_0;       /* Parity bit */
            prm_size = PRM_SIZE_E_fwd-1;
            p_bitsno = bitsno_E_fwd;
        }
        else {
            *ptr_bits++ = BIT_1;     /* "Backward / Forward" Indication bit */
            *ptr_bits++ = BIT_1;       /* Parity bit */
            prm_size = PRM_SIZE_E_bwd-1;
            p_bitsno = bitsno_E_bwd;
        }
    }

    for (i=0; i < prm_size; i++) {
        int2bin(*ptr_prm, p_bitsno[i], ptr_bits);
        ptr_bits += p_bitsno[i];
        ptr_prm++;
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
 *  bits2prm_ld8e - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void bits2prm_ld8e(
    Word16 bits[],  /* I   serial bits (nb_bits)                          */
    Word16 prm[],   /* O   output: decoded parameters (11 parameters)     */
    Word16 rate             /* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps*/
)

{
    Word16 i, *p_bitsno, prm_size;
    Word16 mode;
    Word16 parity_mode;
    Word16 *ptr_bits, *ptr_prm;

    ptr_bits = bits;
    ptr_prm = prm;
    if( rate == 0) {
        /* case 8 kbps */
        prm_size = PRM_SIZE;
        p_bitsno = bitsno;
    }
    else {
        /* case 11.8 kbps */
        if( *ptr_bits++ == BIT_0) {
            mode = 0;
        }
        else {
            mode = 1;
        }
        if( *ptr_bits++ == BIT_0) {
            parity_mode = 0;
        }
        else {
            parity_mode = 1;
        }

        if (mode != parity_mode) { /* ==> frame erased */
            for (i=0; i<bitrates[rate]; i++) bits[i] = 0;
        }
        *ptr_prm++ = mode;
        if (mode == 0) {
            prm_size = PRM_SIZE_E_fwd-1;
            p_bitsno = bitsno_E_fwd;
        }
        else {
            prm_size = PRM_SIZE_E_bwd-1;
            p_bitsno = bitsno_E_bwd;
        }
    }
    for (i=0; i<prm_size; i++) {
        *ptr_prm++ = bin2int(p_bitsno[i], ptr_bits);
        ptr_bits  += p_bitsno[i];
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
