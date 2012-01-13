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
 File : P_PARITY.C
*/
#include "typedef.h"
#include "ld8k.h"

/*-----------------------------------------------------*
* parity_pitch - compute parity bit for first 6 MSBs  *
*-----------------------------------------------------*/

int parity_pitch(     /* output: parity bit (XOR of 6 MSB bits)     */
    int pitch_index     /* input : index for which parity is computed */
)
{
    int temp, sum, i, bit;

    temp = pitch_index >> 1;

    sum = 1;
    for (i = 0; i <= 5; i++) {
        temp >>= 1;
        bit = temp & 1;
        sum = sum + bit;
    }
    sum = sum & 1;
    return (sum);
}

/*--------------------------------------------------------------------*
* check_parity_pitch - check parity of index with transmitted parity *
*--------------------------------------------------------------------*/

int check_parity_pitch(  /* output: 0 = no error, 1= error */
    int pitch_index,       /* input : index of parameter     */
    int parity             /* input : parity bit             */
)
{
    int temp, sum, i, bit;
    temp = pitch_index >> 1;

    sum = 1;
    for (i = 0; i <= 5; i++) {
        temp >>= 1;
        bit = temp & 1;
        sum = sum + bit;
    }
    sum += parity;
    sum = sum & 1;
    return (sum);
}
