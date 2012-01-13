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
 File : PRED_LT3.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/

#include "typedef.h"
#include "version.h"
#ifdef VER_G729A
 #include "ld8a.h"
 #include "tab_ld8a.h"
#else
 #include "ld8k.h"
 #include "tab_ld8k.h"
#endif

/*-------------------------------------------------------------------*
 * Function  Pred_lt_3()                                             *
 *           ~~~~~~~~~~~                                             *
 *-------------------------------------------------------------------*
 * Compute the result of long term prediction with fractional        *
 * interpolation of resolution 1/3.                                  *
 *                                                                   *
 * On return exc[0..L_subfr-1] contains the interpolated signal      *
 *   (adaptive codebook excitation)                                  *
 *-------------------------------------------------------------------*/

void pred_lt_3(         /* Compute adaptive codebook                       */
  FLOAT exc[],          /* in/out: excitation vector, exc[0:l_sub-1] = out */
  int t0,               /* input : pitch lag                               */
  int frac,             /* input : Fraction of pitch lag (-1, 0, 1)  / 3   */
  int l_subfr           /* input : length of subframe.                     */
)
{

  int   i, j, k;
  FLOAT s, *x0, *x1, *x2, *c1, *c2;


  x0 = &exc[-t0];

  frac = -frac;
  if (frac < 0) {
    frac += UP_SAMP;
    x0--;
  }

  for (j=0; j<l_subfr; j++)
  {
    x1 = x0++;
    x2 = x0;
    c1 = &inter_3l[frac];
    c2 = &inter_3l[UP_SAMP-frac];

    s = (F)0.0;
    for(i=0, k=0; i< L_INTER10; i++, k+=UP_SAMP)
      s+= x1[-i] * c1[k] + x2[i] * c2[k];

    exc[j] = s;
  }

  return;
}
