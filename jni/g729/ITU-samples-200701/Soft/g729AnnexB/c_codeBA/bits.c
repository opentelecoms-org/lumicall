/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"
#include "octet.h"

/* prototypes for local functions */
static void  int2bin(Word16 value, Word16 no_of_bits, Word16 *bitstream);
static Word16   bin2int(Word16 no_of_bits, Word16 *bitstream);

/*----------------------------------------------------------------------------
 * prm2bits_ld8k -converts encoder parameter vector into vector of serial bits
 * bits2prm_ld8k - converts serial received bits to  encoder parameter vector
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
 *----------------------------------------------------------------------------
 */
void prm2bits_ld8k(
 Word16   prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
  Word16 bits[]           /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = 80 */
)
{
  Word16 i;
  *bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */

  switch(prm[0]){

    /* not transmitted */
  case 0 : {
    *bits = RATE_0;
    break;
  }

  case 1 : {
    *bits++ = RATE_8000;
    for (i = 0; i < PRM_SIZE; i++) {
      int2bin(prm[i+1], bitsno[i], bits);
      bits += bitsno[i];
    }
    break;
  }

  case 2 : {

#ifndef OCTET_TX_MODE
    *bits++ = RATE_SID;
    for (i = 0; i < 4; i++) {
      int2bin(prm[i+1], bitsno2[i], bits);
      bits += bitsno2[i];
    }
#else
    *bits++ = RATE_SID_OCTET;
    for (i = 0; i < 4; i++) {
      int2bin(prm[i+1], bitsno2[i], bits);
      bits += bitsno2[i];
    }
    *bits++ = BIT_0;
#endif

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
 *  bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void bits2prm_ld8k(
 Word16 bits[],          /* input : serial bits (80)                       */
 Word16   prm[]          /* output: decoded parameters (11 parameters)     */
)
{
  Word16 i;
  Word16 nb_bits;

  nb_bits = *bits++;        /* Number of bits in this frame       */

  if(nb_bits == RATE_8000) {
    prm[1] = 1;
    for (i = 0; i < PRM_SIZE; i++) {
      prm[i+2] = bin2int(bitsno[i], bits);
      bits  += bitsno[i];
    }
  }
  else
#ifndef OCTET_TX_MODE
    if(nb_bits == RATE_SID) {
      prm[1] = 2;
      for (i = 0; i < 4; i++) {
        prm[i+2] = bin2int(bitsno2[i], bits);
        bits += bitsno2[i];
      }
    }
#else
  /* the last bit of the SID bit stream under octet mode will be discarded */
  if(nb_bits == RATE_SID_OCTET) {
    prm[1] = 2;
    for (i = 0; i < 4; i++) {
      prm[i+2] = bin2int(bitsno2[i], bits);
      bits += bitsno2[i];
    }
  }
#endif

  else {
    prm[1] = 0;
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

Word16 read_frame(FILE *f_serial, Word16 *parm)
{
  Word16  i;
  Word16  serial[SERIAL_SIZE];          /* Serial stream               */

  if(fread(serial, sizeof(short), 2, f_serial) != 2) {
    return(0);
  }

  if(fread(&serial[2], sizeof(Word16), (size_t)serial[1], f_serial)
     != (size_t)serial[1]) {
    return(0);
  }

  bits2prm_ld8k(&serial[1], parm);

  /* This part was modified for version V1.3 */
  /* for speech and SID frames, the hardware detects frame erasures
     by checking if all bits are set to zero */
  /* for untransmitted frames, the hardware detects frame erasures
     by testing serial[0] */

  parm[0] = 0;           /* No frame erasure */
  if(serial[1] != 0) {
   for (i=0; i < serial[1]; i++)
     if (serial[i+2] == 0 ) parm[0] = 1;  /* frame erased     */
  }
  else if(serial[0] != SYNC_WORD) parm[0] = 1;

  if(parm[1] == 1) {
    /* check parity and put 1 in parm[5] if parity error */
    parm[5] = Check_Parity_Pitch(parm[4], parm[5]);
  }

  return(1);
}
