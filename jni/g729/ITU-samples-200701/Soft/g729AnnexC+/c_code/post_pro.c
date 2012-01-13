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
 File : POST_PRO.C
*/
#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"

/*------------------------------------------------------------------------*
 * Function post_process()                                                 *
 *                                                                        *
 * Post-processing of output speech.                                      *
 *   - 2nd order high pass filter with cut off frequency at 100 Hz.       *
 *-----------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 * 2nd order high pass filter with cut off frequency at 100 Hz.           *
 * Designed with SPPACK efi command -40 dB att, 0.25 ri.                  *
 *                                                                        *
 * Algorithm:                                                             *
 *                                                                        *
 *  y[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2]                          *
 *                   + a[1]*y[i-1] + a[2]*y[i-2];                         *
 *                                                                        *
 *     b[3] = {0.93980581E+00, -0.18795834E+01,  0.93980581E+00};         *
 *     a[3] = {0.10000000E+01, +0.19330735E+01, -0.93589199E+00};         *
 *-----------------------------------------------------------------------*/

static FLOAT x0, x1;         /* high-pass fir memory          */
static FLOAT y1, y2;         /* high-pass iir memory          */

void init_post_process( void
)
{
  x0 = x1 = (F)0.0;
  y2 = y1 = (F)0.0;
  return;
}

void post_process(
   FLOAT signal[],      /* (i/o)  : signal                     */
   int lg               /* (i)    : lenght of signal           */
)
{
  int i;
  FLOAT x2;
  FLOAT y0;

  for(i=0; i<lg; i++)
  {
    x2 = x1;
    x1 = x0;
    x0 = signal[i];

    y0 = y1*a100[1] + y2*a100[2] + x0*b100[0] + x1*b100[1] + x2*b100[2];

    signal[i] = y0;
    y2 = y1;
    y1 = y0;
  }

  return;
}

