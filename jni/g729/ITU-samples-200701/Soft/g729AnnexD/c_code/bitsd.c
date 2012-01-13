/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX D   Version 1.1    Last modified: March 1998 */

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/
#include "typedef.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "tab_ld8k.h"
#include "tabld8kd.h"
#include "basic_op.h"
#include <stdio.h>

/* prototypes for local functions */
static void  int2bin(Word16 value, Word16 no_of_bits, Word16 *bitstream);
static Word16   bin2int(Word16 no_of_bits, Word16 *bitstream);

/*----------------------------------------------------------------------------
 * prm2bits_ld8kD -converts encoder parameter vector into vector of serial bits
 * bits2prm_ld8kD - converts serial received bits to  encoder parameter vector
 *
 * The transmitted parameters are:
 *
 *                                       8kbps    6.4kbps
 *     LPC:     1st codebook           7+1 bit    7+1 bit
 *              2nd codebook           5+5 bit    5+5 bit
 *
 *     1st subframe:
 *          pitch period                 8 bit      8 bit
 *          parity check on 1st period   1 bit      0 bit
 *          codebook index1 (positions) 13 bit      9 bit
 *          codebook index2 (signs)      4 bit      2 bit
 *          pitch and codebook gains   4+3 bit    3+3 bit
 *
 *     2nd subframe:
 *          pitch period (relative)      5 bit      4 bit
 *          codebook index1 (positions) 13 bit      9 bit
 *          codebook index2 (signs)      4 bit      2 bit
 *          pitch and codebook gains   4+3 bit    3+3 bit
 *----------------------------------------------------------------------------
 */
void prm2bits_ld8kD(
 Word16 prm[],      /* input : encoded parameters  (PRM_SIZE parameters)  */
 Word16 bits[]      /* output: serial bits (SERIAL_SIZE(_6K)) bits[0] = bfi
                               bits[1] = 80 or 64 */
)
{
   Word16 i;

   if (sub(CODEC_MODE, 1) == 0) {
      *bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */
      *bits++ = SIZE_WORD_6K;  /* bit[1], to be compatible with hardware */
 
      for (i = 0; i < PRM_SIZE_6K; i++) {
         int2bin(prm[i], bitsno_6k[i], bits);
         bits += bitsno_6k[i];
      }
   }
   else if (sub(CODEC_MODE, 2) == 0) {
      *bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */
      *bits++ = SIZE_WORD;     /* bit[1], to be compatible with hardware */
 
      for (i = 0; i < PRM_SIZE; i++) {
         int2bin(prm[i], bitsno[i], bits);
         bits += bitsno[i];
      }
   }
   else {
      fprintf(stderr, "CODEC mode invalid\n");
      exit(-1);
   }

   return;
}

/*----------------------------------------------------------------------------
 * int2bin convert integer to binary and write the bits bitstream array
 *----------------------------------------------------------------------------
 */
static void int2bin(
 Word16 value,           /* input : decimal value         */
 Word16 no_of_bits,      /* input : number of bits to use */
 Word16 *bitstream       /* output: bitstream             */
)
{
   Word16 *pt_bitstream;
   Word16 i, bit;

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
 *  bits2prm_ld8kD - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void bits2prm_ld8kD(
 Word16 bits[],          /* input : serial bits (80 or 64)              */
 Word16 prm[]            /* output: decoded parameters (11 parameters)  */
)
{
   Word16 i;

   if (sub(CODEC_MODE, 1) == 0) {
      for (i = 0; i < PRM_SIZE_6K; i++) {
         prm[i] = bin2int(bitsno_6k[i], bits);
         bits  += bitsno_6k[i];
      }
   }
   else if (sub(CODEC_MODE, 2) == 0) {
      for (i = 0; i < PRM_SIZE; i++) {
         prm[i] = bin2int(bitsno[i], bits);
         bits  += bitsno[i];
      }
   }
   else {
      fprintf(stderr, "CODEC mode invalid\n");
      exit(-1);
   }
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
