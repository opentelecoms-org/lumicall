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
 File : DE_ACELP.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/
#include "typedef.h"
#include "version.h"
#ifdef VER_G729A
 #include "ld8a.h"
#else
 #include "ld8k.h"
#endif

/*-----------------------------------------------------------*
 *  Function  decod_ACELP()                                  *
 *  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
 *   Algebraic codebook decoder.                             *
 *----------------------------------------------------------*/

void decod_ACELP(
 int sign,              /* input : signs of 4 pulses     */
 int index,             /* input : positions of 4 pulses */
 FLOAT cod[]            /* output: innovative codevector */
)
{
   int pos[4];
   int i, j;

   /* decode the positions of 4 pulses */

   i = index & 7;
   pos[0] = i*5;

   index >>= 3;
   i = index & 7;
   pos[1] = i*5 + 1;

   index >>= 3;
   i = index & 7;
   pos[2] = i*5 + 2;

   index >>= 3;
   j = index & 1;
   index >>= 1;
   i = index & 7;
   pos[3] = i*5 + 3 + j;

   /* find the algebraic codeword */

   for (i = 0; i < L_SUBFR; i++) cod[i] = 0;

   /* decode the signs of 4 pulses */

   for (j=0; j<4; j++)
   {

     i = sign & 1;
     sign >>= 1;

     if (i != 0) {
       cod[pos[j]] = (F)1.0;
     }
     else {
       cod[pos[j]] = (F)-1.0;
     }
   }

   return;
}
