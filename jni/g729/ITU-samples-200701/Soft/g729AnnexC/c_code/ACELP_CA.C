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

/*
 File : ACELP_CA.C
 Used for the floating point version of G.729A only
 (not for G.729 main body)
*/

/*---------------------------------------------------------------------------*
 *  Function  ACELP_code_A()                                                 *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~                                                 *
 *   Find Algebraic codebook for G.729A                                      *
 *--------------------------------------------------------------------------*/

/*  L_SUBFR   -> Lenght of subframe.                                        */
/*  NB_POS    -> Number of positios for each pulse.                         */
/*  STEP      -> Step betweem position of the same pulse.                   */
/*  MSIZE     -> Size of vectors for cross-correlation between two pulses.  */

#include "typedef.h"
#include "ld8a.h"

/* local routines definition */

static void cor_h(
     FLOAT *H,          /* (i) :Impulse response of filters */
     FLOAT *rr          /* (o) :Correlations of H[]         */
);
static int d4i40_17_fast(/*(o) : Index of pulses positions.               */
  FLOAT dn[],           /* (i) : Correlations between h[] and Xn[].       */
  FLOAT *rr,            /* (i) : Correlations of impulse response h[].    */
  FLOAT h[],            /* (i) : Impulse response of filters.             */
  FLOAT cod[],          /* (o) : Selected algebraic codeword.             */
  FLOAT y[],            /* (o) : Filtered algebraic codeword.             */
  int  *sign            /* (o) : Signs of 4 pulses.                       */
);

 /*-----------------------------------------------------------------*
  * Main ACELP function.                                            *
  *-----------------------------------------------------------------*/

int ACELP_code_A(       /* (o) :index of pulses positions    */
  FLOAT x[],            /* (i) :Target vector                */
  FLOAT h[],            /* (i) :Inpulse response of filters  */
  int T0,               /* (i) :Pitch lag                    */
  FLOAT pitch_sharp,    /* (i) :Last quantized pitch gain    */
  FLOAT code[],         /* (o) :Innovative codebook          */
  FLOAT y[],            /* (o) :Filtered innovative codebook */
  int *sign             /* (o) :Signs of 4 pulses            */
)
{
  int i, index;
  static FLOAT Dn[L_SUBFR]; /* "static" to avoid stack overflow on PC */
  static FLOAT rr[DIM_RR];  /* "static" to avoid stack overflow on PC */

 /*-----------------------------------------------------------------*
  * Include fixed-gain pitch contribution into impulse resp. h[]    *
  * Find correlations of h[] needed for the codebook search.        *
  *-----------------------------------------------------------------*/

  if (T0 < L_SUBFR)
     for (i = T0; i < L_SUBFR; i++)
        h[i] += pitch_sharp * h[i-T0];

  cor_h(h, rr);

 /*-----------------------------------------------------------------*
  * Compute correlation of target vector with impulse response.     *
  *-----------------------------------------------------------------*/

  cor_h_x(h, x, Dn);

 /*-----------------------------------------------------------------*
  * Find innovative codebook.                                       *
  *-----------------------------------------------------------------*/
  index = d4i40_17_fast(Dn, rr, h, code, y, sign);

 /*-----------------------------------------------------------------*
  * Compute innovation vector gain.                                 *
  * Include fixed-gain pitch contribution into code[].              *
  *-----------------------------------------------------------------*/

  if(T0 < L_SUBFR)
     for (i = T0; i < L_SUBFR; i++)
       code[i] += pitch_sharp*code[i-T0];

  return index;
}


/*--------------------------------------------------------------------------*
 *  Function  cor_h()                                                       *
 *  ~~~~~~~~~~~~~~~~~                                                       *
 * Compute  correlations of h[]  needed for the codebook search.            *
 *--------------------------------------------------------------------------*/

static void cor_h(
  FLOAT *h,         /* (i) :Impulse response of filters */
  FLOAT *rr         /* (o) :Correlations of H[]         */
)
{
  FLOAT *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
  FLOAT *rri0i1, *rri0i2, *rri0i3, *rri0i4;
  FLOAT *rri1i2, *rri1i3, *rri1i4;
  FLOAT *rri2i3, *rri2i4;

  FLOAT *p0, *p1, *p2, *p3, *p4;

  FLOAT *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
  FLOAT cor;
  int i, k, ldec, l_fin_sup, l_fin_inf;

 /*------------------------------------------------------------*
  * Compute rri0i0[], rri1i1[], rri2i2[], rri3i3 and rri4i4[]  *
  *------------------------------------------------------------*/
  /* Init pointers */
  rri0i0 = rr;
  rri1i1 = rri0i0 + NB_POS;
  rri2i2 = rri1i1 + NB_POS;
  rri3i3 = rri2i2 + NB_POS;
  rri4i4 = rri3i3 + NB_POS;
  rri0i1 = rri4i4 + NB_POS;
  rri0i2 = rri0i1 + MSIZE;
  rri0i3 = rri0i2 + MSIZE;
  rri0i4 = rri0i3 + MSIZE;
  rri1i2 = rri0i4 + MSIZE;
  rri1i3 = rri1i2 + MSIZE;
  rri1i4 = rri1i3 + MSIZE;
  rri2i3 = rri1i4 + MSIZE;
  rri2i4 = rri2i3 + MSIZE;

  p0 = rri0i0 + NB_POS-1;   /* Init pointers to last position of rrixix[] */
  p1 = rri1i1 + NB_POS-1;
  p2 = rri2i2 + NB_POS-1;
  p3 = rri3i3 + NB_POS-1;
  p4 = rri4i4 + NB_POS-1;

  ptr_h1 = h;
  cor    = (F)0.0;
  for(i=0;  i<NB_POS; i++)
  {
    cor += *ptr_h1 * *ptr_h1; ptr_h1++;
    *p4-- = cor;

    cor += *ptr_h1 * *ptr_h1; ptr_h1++;
    *p3-- = cor;

    cor += *ptr_h1 * *ptr_h1; ptr_h1++;
    *p2-- = cor;

    cor += *ptr_h1 * *ptr_h1; ptr_h1++;
    *p1-- = cor;

    cor += *ptr_h1 * *ptr_h1; ptr_h1++;
    *p0-- = cor;
  }

 /*-----------------------------------------------------------------*
  * Compute elements of: rri2i3[], rri1i2[], rri0i1[] and rri0i4[]  *
  *-----------------------------------------------------------------*/

  l_fin_sup = MSIZE-1;
  l_fin_inf = l_fin_sup-1;
  ldec = NB_POS+1;

  ptr_hd = h;
  ptr_hf = ptr_hd + 1;

  for(k=0; k<NB_POS; k++) {

          p3 = rri2i3 + l_fin_sup;
          p2 = rri1i2 + l_fin_sup;
          p1 = rri0i1 + l_fin_sup;
          p0 = rri0i4 + l_fin_inf;
          cor = (F)0.0;
          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;

          for(i=k+1; i<NB_POS; i++ ) {

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p3 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p2 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p1 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p0 = cor;

                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          *p3 = cor;

          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          *p2 = cor;

          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          *p1 = cor;

          l_fin_sup -= NB_POS;
          l_fin_inf--;
          ptr_hf += STEP;
  }

 /*---------------------------------------------------------------------*
  * Compute elements of: rri2i4[], rri1i3[], rri0i2[], rri1i4[], rri0i3 *
  *---------------------------------------------------------------------*/

  ptr_hd = h;
  ptr_hf = ptr_hd + 2;
  l_fin_sup = MSIZE-1;
  l_fin_inf = l_fin_sup-1;
  for(k=0; k<NB_POS; k++) {

          p4 = rri2i4 + l_fin_sup;
          p3 = rri1i3 + l_fin_sup;
          p2 = rri0i2 + l_fin_sup;
          p1 = rri1i4 + l_fin_inf;
          p0 = rri0i3 + l_fin_inf;

          cor = (F)0.0;
          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          for(i=k+1; i<NB_POS; i++ ) {

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p4 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p3 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p2 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p1 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p0 = cor;

                  p4 -= ldec;
                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          *p4 = cor;

          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          *p3 = cor;

          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          *p2 = cor;

          l_fin_sup -= NB_POS;
          l_fin_inf--;
          ptr_hf += STEP;
  }

 /*----------------------------------------------------------------------*
  * Compute elements of: rri1i4[], rri0i3[], rri2i4[], rri1i3[], rri0i2  *
  *----------------------------------------------------------------------*/

  ptr_hd = h;
  ptr_hf = ptr_hd + 3;
  l_fin_sup = MSIZE-1;
  l_fin_inf = l_fin_sup-1;
  for(k=0; k<NB_POS; k++) {

          p4 = rri1i4 + l_fin_sup;
          p3 = rri0i3 + l_fin_sup;
          p2 = rri2i4 + l_fin_inf;
          p1 = rri1i3 + l_fin_inf;
          p0 = rri0i2 + l_fin_inf;

          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          cor = (F)0.0;
          for(i=k+1; i<NB_POS; i++ ) {

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p4 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p3 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p2 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p1 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p0 = cor;

                  p4 -= ldec;
                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          *p4 = cor;

          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          *p3 = cor;

          l_fin_sup -= NB_POS;
          l_fin_inf--;
          ptr_hf += STEP;
  }

 /*----------------------------------------------------------------------*
  * Compute elements of: rri0i4[], rri2i3[], rri1i2[], rri0i1[]          *
  *----------------------------------------------------------------------*/

  ptr_hd = h;
  ptr_hf = ptr_hd + 4;
  l_fin_sup = MSIZE-1;
  l_fin_inf = l_fin_sup-1;
  for(k=0; k<NB_POS; k++) {

          p3 = rri0i4 + l_fin_sup;
          p2 = rri2i3 + l_fin_inf;
          p1 = rri1i2 + l_fin_inf;
          p0 = rri0i1 + l_fin_inf;

          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          cor = 0;
          for(i=k+1; i<NB_POS; i++ ) {

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p3 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p2 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p1 = cor;

                  cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                  *p0 = cor;

                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
          *p3 = cor;

          l_fin_sup -= NB_POS;
          l_fin_inf--;
          ptr_hf += STEP;
  }
  return;
}


/*------------------------------------------------------------------------*
 * Function  d4i40_17_fast                                                *
 *           ~~~~~~~~~                                                    *
 * Algebraic codebook for G.729A.                                         *
 *  -> 17 bits; 4 pulses in a frame of 40 samples                         *
 *                                                                        *
 *------------------------------------------------------------------------*
 * The code length is 40, containing 4 nonzero pulses i0, i1, i2, i3.     *
 * Each pulses can have 8 possible positions (positive or negative)       *
 * except i3 that have 16 possible positions.                             *
 *                                                                        *
 * i0 (+-1) : 0, 5, 10, 15, 20, 25, 30, 35                                *
 * i1 (+-1) : 1, 6, 11, 16, 21, 26, 31, 36                                *
 * i2 (+-1) : 2, 7, 12, 17, 22, 27, 32, 37                                *
 * i3 (+-1) : 3, 8, 13, 18, 23, 28, 33, 38                                *
 *            4, 9, 14, 19, 24, 29, 34, 39                                *
 *------------------------------------------------------------------------*/

static int d4i40_17_fast(/*(o) : Index of pulses positions.               */
  FLOAT dn[],           /* (i) : Correlations between h[] and Xn[].       */
  FLOAT rr[],           /* (i) : Correlations of impulse response h[].    */
  FLOAT h[],            /* (i) : Impulse response of filters.             */
  FLOAT cod[],          /* (o) : Selected algebraic codeword.             */
  FLOAT y[],            /* (o) : Filtered algebraic codeword.             */
  int *sign             /* (o) : Signs of 4 pulses.                       */
)
{


  int i0, i1, i2, i3, ip0, ip1, ip2, ip3;
  int i, j, ix, iy, track, trk;
  int prev_i0, i1_offset;
  FLOAT psk, ps, ps0, ps1, ps2, sq, sq2;
  FLOAT alpk, alp, max;
  FLOAT s, alp0, alp1, alp2;
  FLOAT *p0, *p1, *p2, *p3, *p4;

  FLOAT sign_dn[L_SUBFR], sign_dn_inv[L_SUBFR], *psign;
  FLOAT tmp_vect[NB_POS];

  FLOAT *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
  FLOAT *rri0i1, *rri0i2, *rri0i3, *rri0i4;
  FLOAT *rri1i2, *rri1i3, *rri1i4;
  FLOAT *rri2i3, *rri2i4;

  FLOAT  *ptr_rri0i3_i4;
  FLOAT  *ptr_rri1i3_i4;
  FLOAT  *ptr_rri2i3_i4;
  FLOAT  *ptr_rri3i3_i4;

     /* Init pointers */
   rri0i0 = rr;
   rri1i1 = rri0i0 + NB_POS;
   rri2i2 = rri1i1 + NB_POS;
   rri3i3 = rri2i2 + NB_POS;
   rri4i4 = rri3i3 + NB_POS;
   rri0i1 = rri4i4 + NB_POS;
   rri0i2 = rri0i1 + MSIZE;
   rri0i3 = rri0i2 + MSIZE;
   rri0i4 = rri0i3 + MSIZE;
   rri1i2 = rri0i4 + MSIZE;
   rri1i3 = rri1i2 + MSIZE;
   rri1i4 = rri1i3 + MSIZE;
   rri2i3 = rri1i4 + MSIZE;
   rri2i4 = rri2i3 + MSIZE;

 /*-----------------------------------------------------------------------*
  * Chose the sign of the impulse.                                        *
  *-----------------------------------------------------------------------*/

   for (i=0; i<L_SUBFR; i++)
   {
     if (dn[i] >= (F)0.0)
     {
       sign_dn[i] = (F)1.0;
       sign_dn_inv[i] = (F)-1.0;
     }
     else
     {
       sign_dn[i] = (F)-1.0;
       sign_dn_inv[i] = (F)1.0;
       dn[i] = -dn[i];
     }
   }

 /*-------------------------------------------------------------------*
  * Modification of rrixiy[] to take signs into account.              *
  *-------------------------------------------------------------------*/

  p0 = rri0i1;
  p1 = rri0i2;
  p2 = rri0i3;
  p3 = rri0i4;

  for(i0=0; i0<L_SUBFR; i0+=STEP)
  {
    psign = sign_dn;
    if (psign[i0] < 0.0) psign = sign_dn_inv;

    for(i1=1; i1<L_SUBFR; i1+=STEP)
    {
      *p0 *= psign[i1];    p0++;
      *p1 *= psign[i1+1];  p1++;
      *p2 *= psign[i1+2];  p2++;
      *p3 *= psign[i1+3];  p3++;
    }
  }

  p0 = rri1i2;
  p1 = rri1i3;
  p2 = rri1i4;

  for(i1=1; i1<L_SUBFR; i1+=STEP)
  {
    psign = sign_dn;
    if (psign[i1] < 0.0) psign = sign_dn_inv;

    for(i2=2; i2<L_SUBFR; i2+=STEP)
    {
      *p0 *= psign[i2];   p0++;
      *p1 *= psign[i2+1]; p1++;
      *p2 *= psign[i2+2]; p2++;
    }
  }

  p0 = rri2i3;
  p1 = rri2i4;

  for(i2=2; i2<L_SUBFR; i2+=STEP)
  {
    psign = sign_dn;
    if (psign[i2] < 0.0) psign = sign_dn_inv;

    for(i3=3; i3<L_SUBFR; i3+=STEP)
    {
      *p0 *= psign[i3];   p0++;
      *p1 *= psign[i3+1]; p1++;
    }
  }


 /*-------------------------------------------------------------------*
  * Search the optimum positions of the four pulses which maximize    *
  *     square(correlation) / energy                                  *
  *-------------------------------------------------------------------*/

  psk = (F)-1.0;
  alpk = (F)1.0;

  ptr_rri0i3_i4 = rri0i3;
  ptr_rri1i3_i4 = rri1i3;
  ptr_rri2i3_i4 = rri2i3;
  ptr_rri3i3_i4 = rri3i3;

  /* search 2 times: track 3 and 4 */

  for (track=3, trk=0; track<5; track++, trk++)
  {
   /*------------------------------------------------------------------*
    * depth first search 3, phase A: track 2 and 3/4.                  *
    *------------------------------------------------------------------*/

    sq = (F)-1.0;
    alp = (F)1.0;

    /* i0 loop: 2 positions in track 2 */

    prev_i0  = -1;

    for (i=0; i<2; i++)
    {
      max = (F)-1.0;
      /* search "dn[]" maximum position in track 2 */
      for (j=2; j<L_SUBFR; j+=STEP)
      {
        if( ((dn[j] - max) > (F)0.0) && ((prev_i0-j) != 0) )
        {
          max = dn[j];
          i0 = j;
        }
      }
      prev_i0 = i0;

      j = i0 / 5;        /* j = i0/5 */
      p0 = rri2i2 + j;

      ps1 = dn[i0];
      alp1 = 0.5 * *p0;

      /* i1 loop: 8 positions in track 2 */

      p0 = ptr_rri2i3_i4 + (j<<3);
      p1 = ptr_rri3i3_i4;

      for (i1=track; i1<L_SUBFR; i1+=STEP)
      {
        ps2 = ps1 + dn[i1];
        alp2 = alp1 + *p0++ + *p1++ * (F)0.5;
        sq2 = ps2 * ps2;
        s = alp*sq2 - sq * alp2;
        if (s > (F)0.0)
        {
          sq = sq2;
          ps = ps2;
          alp = alp2;
          ix = i0;
          iy = i1;
        }
      }
    }

    i0 = ix;
    i1 = iy;
    i1_offset = (i1/5) <<3;           /* j = 8*(i1/5) */

   /*------------------------------------------------------------------*
    * depth first search 3, phase B: track 0 and 1.                    *
    *------------------------------------------------------------------*/

    ps0 = ps;
    alp0 = alp;

    sq = (F)-1.0;
    alp = (F)1.0;

    /* build vector for next loop to decrease complexity */

    p0 = rri1i2 + i0/5;
    p1 = ptr_rri1i3_i4 + i1/5;
    p2 = rri1i1;
    p3 = tmp_vect;

    for (i3=1; i3<L_SUBFR; i3+=STEP)
    {
      *p3++ = *p0 + *p1 + *p2 * (F)0.5;
      p0 += NB_POS; p1 += NB_POS; p2++;
    }

    /* i2 loop: 8 positions in track 0 */

    p0 = rri0i2 + i0/5;
    p1 = ptr_rri0i3_i4 + i1/5;
    p2 = rri0i0;
    p3 = rri0i1;

    for (i2=0; i2<L_SUBFR; i2+=STEP)
    {
      ps1 = ps0 + dn[i2];

      alp1 = alp0 + *p0 + *p1 + *p2 * (F)0.5;
      p0 += NB_POS; p1 += NB_POS; p2++;

      /* i3 loop: 8 positions in track 1 */

      p4 = tmp_vect;

      for (i3=1; i3<L_SUBFR; i3+=STEP)
      {
        ps2 = ps1 + dn[i3];

        alp2 = alp1 + *p3++ + *p4++;

        sq2 = ps2 * ps2;
        s = alp*sq2 - sq * alp2;
        if (s > 0.0)
        {
          sq = sq2;
          alp = alp2;
          ix = i2;
          iy = i3;
        }
      }
    }

   /*----------------------------------------------------------------*
    * depth first search 3: compare codevector with the best case.   *
    *----------------------------------------------------------------*/

    s = alpk*sq - psk * alp;
    if (s > (F)0.0)
    {
      psk = sq;
      alpk = alp;
      ip2 = i0;
      ip3 = i1;
      ip0 = ix;
      ip1 = iy;
    }

   /*------------------------------------------------------------------*
    * depth first search 4, phase A: track 3 and 0.                    *
    *------------------------------------------------------------------*/

    sq = (F)-1.0;
    alp = (F)1.0;

    /* i0 loop: 2 positions in track 3/4 */

    prev_i0  = -1;

    for (i=0; i<2; i++)
    {
      max = (F)-1.0;
      /* search "dn[]" maximum position in track 3/4 */
      for (j=track; j<L_SUBFR; j+=STEP)
      {
        if( ((dn[j] - max) > 0) && ((prev_i0 -j) != 0) )
        {
          max = dn[j];
          i0 = j;
        }
      }
      prev_i0 = i0;

      j = i0/5;
      p0 = ptr_rri3i3_i4 + j;

      ps1 = dn[i0];
      alp1 = 0.5 * *p0;

      /* i1 loop: 8 positions in track 0 */

      p0 = ptr_rri0i3_i4 + j;
      p1 = rri0i0;

      for (i1=0; i1<L_SUBFR; i1+=STEP)
      {
        ps2 = ps1 + dn[i1];

        alp2 = alp1 + *p0 + *p1 * (F)0.5;
        p0 += NB_POS;
        p1++;

        sq2 = ps2 * ps2;
        s = alp*sq2 - sq * alp2;
        if (s > (F)0.0)
        {
          sq = sq2;
          ps = ps2;
          alp = alp2;
          ix = i0;
          iy = i1;
        }
      }
    }

    i0 = ix;
    i1 = iy;
    i1_offset = (i1/5) <<3;           /* j = 8*(i1/5) */

   /*------------------------------------------------------------------*
    * depth first search 4, phase B: track 1 and 2.                    *
    *------------------------------------------------------------------*/

    ps0 = ps;
    alp0 = alp;

    sq = (F)-1.0;
    alp = (F)1.0;

    /* build vector for next loop to decrease complexity */

    p0 = ptr_rri2i3_i4 + i0/5;
    p1 = rri0i2 + i1_offset;
    p2 = rri2i2;
    p3 = tmp_vect;

    for (i3=2; i3<L_SUBFR; i3+=STEP)
    {
      *p3++ = *p0 + *p1 + *p2 * (F)0.5;
      p0 += NB_POS; p1++; p2++;
    }

    /* i2 loop: 8 positions in track 1 */

    p0 = ptr_rri1i3_i4 + i0/5;
    p1 = rri0i1 + i1_offset;
    p2 = rri1i1;
    p3 = rri1i2;

    for (i2=1; i2<L_SUBFR; i2+=STEP)
    {
      ps1 = ps0 + dn[i2];

      alp1 = alp0 + *p0 + *p1 + *p2 * (F)0.5;
      p0 += NB_POS; p1++; p2++;

      /* i3 loop: 8 positions in track 2 */

      p4 = tmp_vect;

      for (i3=2; i3<L_SUBFR; i3+=STEP)
      {
        ps2 = ps1 + dn[i3];
        alp2 = alp1 + *p3++ + *p4++;
        sq2 = ps2 * ps2;
        s = alp*sq2 - sq * alp2;
        if (s > (F)0.0)
        {
          sq = sq2;
          alp = alp2;
          ix = i2;
          iy = i3;
        }
      }
    }

   /*----------------------------------------------------------------*
    * depth first search 1: compare codevector with the best case.   *
    *----------------------------------------------------------------*/

    s = alpk*sq - psk*alp;
    if (s > (F)0.0)
    {
      psk = sq;
      alpk = alp;
      ip3 = i0;
      ip0 = i1;
      ip1 = ix;
      ip2 = iy;
    }
  ptr_rri0i3_i4 = rri0i4;
  ptr_rri1i3_i4 = rri1i4;
  ptr_rri2i3_i4 = rri2i4;
  ptr_rri3i3_i4 = rri4i4;

  }


 /* Set the sign of impulses */

 i0 = (int)sign_dn[ip0];
 i1 = (int)sign_dn[ip1];
 i2 = (int)sign_dn[ip2];
 i3 = (int)sign_dn[ip3];

 /* Find the codeword corresponding to the selected positions */


 for(i=0; i<L_SUBFR; i++) {
   cod[i] = (F)0.0;
 }

 cod[ip0] = (F)i0;
 cod[ip1] = (F)i1;
 cod[ip2] = (F)i2;
 cod[ip3] = (F)i3;

 /* find the filtered codeword */

 for (i = 0; i < ip0; i++) y[i] = (F)0.0;

 if(i0 > 0)
   for(i=ip0, j=0; i<L_SUBFR; i++, j++) y[i] = h[j];
 else
   for(i=ip0, j=0; i<L_SUBFR; i++, j++) y[i] = -h[j];

 if(i1 > 0)
   for(i=ip1, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] + h[j];
 else
   for(i=ip1, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] - h[j];

 if(i2 > 0)
   for(i=ip2, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] + h[j];
 else
   for(i=ip2, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] - h[j];

 if(i3 > 0)
   for(i=ip3, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] + h[j];
 else
   for(i=ip3, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] - h[j];

 /* find codebook index;  17-bit address */

 i = 0;
 if(i0 > 0) i += 1;
 if(i1 > 0) i += 2;
 if(i2 > 0) i += 4;
 if(i3 > 0) i += 8;
 *sign = i;

 ip0 = ip0/5;
 ip1 = ip1/5;
 ip2 = ip2/5;
 i   = ip3/5;
 j   = i*5;
 j   = ip3 -j -3;
 ip3 = i*2+j;

 i = ip0 + (ip1<<3)+ (ip2<<6)+ (ip3<<9);
 return i;
}
