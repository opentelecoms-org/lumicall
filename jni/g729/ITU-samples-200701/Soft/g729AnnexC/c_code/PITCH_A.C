/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729 Annex A
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

/***********************************************************************/
/*           Long Term (Pitch) Prediction Functions                    */
/***********************************************************************/

#include <math.h>
#include "typedef.h"
#include "ld8a.h"

/* prototypes for local functions */

static FLOAT dot_product(FLOAT x[], FLOAT y[], int lg);

/*----------------------------------------------------------------------*
 * pitch_ol_fast -> compute the open loop pitch lag -> fast version     *
 *----------------------------------------------------------------------*/

int pitch_ol_fast(         /* output: open-loop pitch lag       */
    FLOAT signal[],        /* input : signal to compute pitch   */
                           /*         s[-PIT_MAX : l_frame-1]   */
    int l_frame            /* input : error minimization window */
)
{
    int  i, j;
    int  T1, T2, T3;
    FLOAT  max1, max2, max3;
    FLOAT  *p, *p1, sum;


   /*--------------------------------------------------------------------*
    *  The pitch lag search is divided in three sections.                *
    *  Each section cannot have a pitch multiple.                        *
    *  A maximum is find for each section.                               *
    *  The final lag is selected by taking into account the multiple.    *
    *                                                                    *
    *  First section:  lag delay = 20 to 39                              *
    *  Second section: lag delay = 40 to 79                              *
    *  Third section:  lag delay = 80 to 143                             *
    *--------------------------------------------------------------------*/

    /* First section */

    max1 = FLT_MIN_G729;
    for (i = 20; i < 40; i++) {
        p  = signal;
        p1 = &signal[-i];
        sum = (F)0.0;
        /* Dot product with decimation by 2 */
        for (j=0; j<l_frame; j+=2, p+=2, p1+=2)
            sum += *p * *p1;
        if (sum > max1) { max1 = sum; T1 = i;}
    }

    /* compute energy of maximum1 */

    sum = (F)0.01;                   /* to avoid division by zero */
    p = &signal[-T1];
    for(i=0; i<l_frame; i+=2, p+=2)
        sum += *p * *p;
    sum = (F)1.0/(FLOAT)sqrt(sum);          /* 1/sqrt(energy)    */
    max1 *= sum;                  /* max/sqrt(energy)  */


    /* Second section */

    max2 = FLT_MIN_G729;
    for (i = 40; i < 80; i++) {
        p  = signal;
        p1 = &signal[-i];
        sum = (F)0.0;
        /* Dot product with decimation by 2 */
        for (j=0; j<l_frame; j+=2, p+=2, p1+=2)
            sum += *p * *p1;
        if (sum > max2) { max2 = sum; T2 = i;}
    }

    /* compute energy of maximum2 */

    sum = (F)0.01;                   /* to avoid division by zero */
    p = &signal[-T2];
    for(i=0; i<l_frame; i+=2, p+=2)
        sum += *p * *p;
    sum  = (F)1.0/(FLOAT)sqrt(sum);         /* 1/sqrt(energy)    */
    max2 *= sum;                  /* max/sqrt(energy)  */

    /* Third section */

    max3 = FLT_MIN_G729;
    /* decimation by 2 for the possible delay */
    for (i = 80; i < 143; i+=2) {
        p  = signal;
        p1 = &signal[-i];
        sum = (F)0.0;
        /* Dot product with decimation by 2 */
        for (j=0; j<l_frame; j+=2, p+=2, p1+=2)
            sum += *p * *p1;
        if (sum > max3) { max3 = sum; T3 = i;}
    }

     /* Test around max3 */

     i = T3;
     p  = signal;
     p1 = &signal[-(i+1)];
     sum = (F)0.0;
     for (j=0; j<l_frame; j+=2, p+=2, p1+=2)
         sum += *p * *p1;
     if (sum > max3) { max3 = sum; T3 = i+1;}

     p  = signal;
     p1 = &signal[-(i-1)];
     sum = (F)0.0;
     for (j=0; j<l_frame; j+=2, p+=2, p1+=2)
         sum += *p * *p1;
     if (sum > max3) { max3 = sum; T3 = i-1;}


    /* compute energy of maximum3 */

    sum = (F)0.01;                   /* to avoid division by zero */
    p = &signal[-T3];
    for(i=0; i<l_frame; i+=2, p+=2)
        sum += *p * *p;
    sum  = (F)1.0/(FLOAT)sqrt(sum);         /* 1/sqrt(energy)    */
    max3 *= sum;                  /* max/sqrt(energy)  */

   /*-----------------------*
    * Test for multiple.    *
    *-----------------------*/

    if( abs(T2*2 - T3) < 5)
       max2 += max3 * (F)0.25;
    if( abs(T2*3 - T3) < 7)
       max2 += max3 * (F)0.25;

    if( abs(T1*2 - T2) < 5)
       max1 += max2 * (F)0.20;
    if( abs(T1*3 - T2) < 7)
       max1 += max2 * (F)0.20;

   /*--------------------------------------------------------------------*
    * Compare the 3 sections maxima.                                     *
    *--------------------------------------------------------------------*/

    if( max1 < max2 ) {max1 = max2; T1 = T2;}
    if( max1 < max3 )  T1 = T3;

    return (T1);
}

/*------------------------------------------------------------------*
 * dot_product()                                                    *
 *  dot product between vector x[] and y[] of lenght lg             *
 *------------------------------------------------------------------*/

static FLOAT dot_product(   /* Return the dot product between x[] an y[] */
  FLOAT x[],                /* First vector.                             */
  FLOAT y[],                /* Second vector.                            */
  int lg                    /* Lenght of the product.                    */
)
{
  int i;
  FLOAT sum;

  sum = (F)0.1;
  for(i=0; i<lg; i++)
     sum +=  x[i] * y[i];

  return sum;
}

/*-------------------------------------------------------------------------*
 * pitch_fr3_fast()                                                        *
 *  Find the pitch period in close loop with 1/3 subsample resolution      *
 *  Fast version                                                           *
 *-------------------------------------------------------------------------*/
int pitch_fr3_fast(     /* output: integer part of pitch period        */
  FLOAT exc[],          /* input : excitation buffer                   */
  FLOAT xn[],           /* input : target vector                       */
  FLOAT h[],            /* input : impulse response.                   */
  int l_subfr,          /* input : Length of subframe                  */
  int t0_min,           /* input : minimum value in the searched range */
  int t0_max,           /* input : maximum value in the searched range */
  int i_subfr,          /* input : indicator for first subframe        */
  int *pit_frac         /* output: chosen fraction                     */
)
{
  int  t, t0;
  FLOAT  dn[L_SUBFR];
  FLOAT  exc_tmp[L_SUBFR];
  FLOAT  corr, max;

  /* Compute  correlations of input response h[] with the target vector xn[].*/

  cor_h_x (h, xn, dn);

  /* Find maximum integer delay */

  max = FLT_MIN_G729;
  for(t=t0_min; t<=t0_max; t++)
  {
    corr = dot_product(dn, &exc[-t], l_subfr);
    if(corr > max) {max = corr; t0 = t;}
  }

  /* Test fractions */

  /* Fraction 0 */
  pred_lt_3(exc, t0, 0, l_subfr);
  max = dot_product(dn, exc, l_subfr);
  *pit_frac = 0;

  /* If first subframe and lag > 84 do not search fractional pitch */

  if( (i_subfr == 0) && (t0 > 84) )
    return t0;

  copy(exc, exc_tmp, l_subfr);

  /* Fraction -1/3 */

  pred_lt_3(exc, t0, -1, l_subfr);
  corr = dot_product(dn, exc, l_subfr);
  if(corr > max){
    max = corr;
    *pit_frac = -1;
    copy(exc, exc_tmp, l_subfr);
  }

  /* Fraction +1/3 */

  pred_lt_3(exc, t0, 1, l_subfr);
  corr = dot_product(dn, exc, l_subfr);
  if(corr > max){
    max = corr;
    *pit_frac = 1;
  }
  else
    copy(exc_tmp, exc, l_subfr);

  return t0;
}

/*---------------------------------------------------------------------------*
 * g_pitch  - compute adaptive codebook gain and compute <y1,y1> , -2<xn,y1> *
 *---------------------------------------------------------------------------*/

FLOAT g_pitch(          /* output: pitch gain                        */
  FLOAT xn[],           /* input : target vector                     */
  FLOAT y1[],           /* input : filtered adaptive codebook vector */
  FLOAT g_coeff[],      /* output: <y1,y1> and -2<xn,y1>             */
  int l_subfr           /* input : vector dimension                  */
)
{
    FLOAT xy, yy, gain;
    int   i;

    yy = (F)0.01;
    for (i = 0; i < l_subfr; i++) {
        yy += y1[i] * y1[i];          /* energy of filtered excitation */
    }
    xy = (F)0.0;
    for (i = 0; i < l_subfr; i++) {
        xy += xn[i] * y1[i];
    }

    g_coeff[0] = yy;
    g_coeff[1] = (F)-2.0*xy +(F)0.01;

    /* find pitch gain and bound it by [0,1.2] */

    gain = xy/yy;

    if (gain<(F)0.0)  gain = (F)0.0;
    if (gain>GAIN_PIT_MAX) gain = GAIN_PIT_MAX;

    return gain;
}

/*----------------------------------------------------------------------*
 *    Functions enc_lag3()                                              *
 *              ~~~~~~~~~~                                              *
 *   Encoding of fractional pitch lag with 1/3 resolution.              *
 *----------------------------------------------------------------------*
 * The pitch range for the first subframe is divided as follows:        *
 *   19 1/3  to   84 2/3   resolution 1/3                               *
 *   85      to   143      resolution 1                                 *
 *                                                                      *
 * The period in the first subframe is encoded with 8 bits.             *
 * For the range with fractions:                                        *
 *   index = (T-19)*3 + frac - 1;   where T=[19..85] and frac=[-1,0,1]  *
 * and for the integer only range                                       *
 *   index = (T - 85) + 197;        where T=[86..143]                   *
 *----------------------------------------------------------------------*
 * For the second subframe a resolution of 1/3 is always used, and the  *
 * search range is relative to the lag in the first subframe.           *
 * If t0 is the lag in the first subframe then                          *
 *  t_min=t0-5   and  t_max=t0+4   and  the range is given by           *
 *       t_min - 2/3   to  t_max + 2/3                                  *
 *                                                                      *
 * The period in the 2nd subframe is encoded with 5 bits:               *
 *   index = (T-(t_min-1))*3 + frac - 1;    where T[t_min-1 .. t_max+1] *
 *----------------------------------------------------------------------*/
int  enc_lag3(     /* output: Return index of encoding */
  int  T0,         /* input : Pitch delay              */
  int  T0_frac,    /* input : Fractional pitch delay   */
  int  *T0_min,    /* in/out: Minimum search delay     */
  int  *T0_max,    /* in/out: Maximum search delay     */
  int pit_min,     /* input : Minimum pitch delay      */
  int pit_max,     /* input : Maximum pitch delay      */
  int  i_subfr     /* input : Flag for 1st subframe    */
)
{
  int index;

  if (i_subfr == 0)   /* if 1st subframe */
  {
     /* encode pitch delay (with fraction) */

     if (T0 <= 85)
       index = T0*3 - 58 + T0_frac;
     else
       index = T0 + 112;

     /* find T0_min and T0_max for second subframe */

     *T0_min = T0 - 5;
     if (*T0_min < pit_min) *T0_min = pit_min;
     *T0_max = *T0_min + 9;
     if (*T0_max > pit_max)
     {
         *T0_max = pit_max;
         *T0_min = *T0_max - 9;
     }
  }

  else                    /* second subframe */
  {
     index = T0 - *T0_min;
     index = index*3 + 2 + T0_frac;
  }
  return index;
}
