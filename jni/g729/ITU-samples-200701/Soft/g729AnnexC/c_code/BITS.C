/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.

----------------------------------------------------------------------
*/

/*
 File : BITS.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/

#include "typedef.h"
#include "version.h"
#ifdef VER_G729A
 #include "ld8a.h"
 #include "tab_ld8a.h"
#else
 #include "ld8k.h"
 #include "tab_ld8k.h"
#endif

/* prototypes for local functions */

static void   int2bin(int  value, int  no_of_bits, INT16 *bitstream);
static int    bin2int(int  no_of_bits, INT16 *bitstream);

/*----------------------------------------------------------------------------
 * prm2bits_ld8k -converts encoder parameter vector into vector of serial bits
 * bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *
 * The transmitted parameters for 8000 bits/sec are:
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
 */

void prm2bits_ld8k(
 int  prm[],            /* input : encoded parameters  */
 INT16 bits[]           /* output: serial bits         */
)
{
   int  i;
   *bits++ = SYNC_WORD;     /* At receiver this bit indicates BFI */
   *bits++ = SIZE_WORD;     /* Number of bits in this frame       */

   for (i = 0; i < PRM_SIZE; i++)
   {
      int2bin(prm[i], bitsno[i], bits);
      bits += bitsno[i];
   }

   return;
}

/*----------------------------------------------------------------------------
 * int2bin convert integer to binary and write the bits bitstream array
 *----------------------------------------------------------------------------
 */
static void int2bin(
 int  value,             /* input : decimal value */
 int  no_of_bits,        /* input : number of bits to use */
 INT16 *bitstream        /* output: bitstream  */
)
{
   INT16 *pt_bitstream;
   int    i, bit;

   pt_bitstream = bitstream + no_of_bits;

   for (i = 0; i < no_of_bits; i++)
   {
     bit = value & 0x0001;      /* get lsb */
     if (bit == 0)
         *--pt_bitstream = BIT_0;
     else
         *--pt_bitstream = BIT_1;
     value >>= 1;
   }
   return;
}

/*----------------------------------------------------------------------------
 *  bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void bits2prm_ld8k(
 INT16 bits[],          /* input : serial bits        */
 int  prm[]             /* output: decoded parameters */
)
{
   int  i;
   for (i = 0; i < PRM_SIZE; i++)
   {
      prm[i] = bin2int(bitsno[i], bits);
      bits  += bitsno[i];
   }

   return;
}


/*----------------------------------------------------------------------------
 * bin2int - read specified bits from bit array  and convert to integer value
 *----------------------------------------------------------------------------
 */
static int  bin2int(            /* output: decimal value of bit pattern */
 int  no_of_bits,        /* input : number of bits to read       */
 INT16 *bitstream        /* input : array containing bits        */
)
{
   int    value, i;
   int  bit;

   value = 0;
   for (i = 0; i < no_of_bits; i++)
   {
     value <<= 1;
     bit = *bitstream++;
     if (bit == BIT_1)  value += 1;
   }
   return(value);
}
