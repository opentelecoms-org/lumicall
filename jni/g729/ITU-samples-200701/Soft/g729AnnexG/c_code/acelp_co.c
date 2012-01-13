/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 3.3    Last modified: December 26, 1995 */

/*---------------------------------------------------------------------------*
 *  Function  ACELP_CODEBOOK()                                               *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~                                               *
 *   Find Algebraic codebook.                                                *
 *--------------------------------------------------------------------------*/

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
static void Cor_h(
     Word16 *H,         /* (i) Q12 :Impulse response of filters */
     Word16 *rr         /* (o)     :Correlations of H[]         */
);
static void Cor_h_X(
     Word16 h[],        /* (i) Q12 :Impulse response of filters      */
     Word16 X[],        /* (i) Q12 :Target vector                    */
     Word16 D[]         /* (o)     :Correlations between h[] and D[] */
                        /*          Normalized to 13 bits            */
);
static Word16 D4i40_17(        /* (o)    : Index of pulses positions.               */
  Word16 Dn[],          /* (i)    : Correlations between h[] and Xn[].       */
  Word16 rr[],          /* (i)    : Correlations of impulse response h[].    */
  Word16 h[],           /* (i) Q12: Impulse response of filters.             */
  Word16 cod[],         /* (o) Q13: Selected algebraic codeword.             */
  Word16 y[],           /* (o) Q12: Filtered algebraic codeword.             */
  Word16 *sign,         /* (o)    : Signs of 4 pulses.                       */
  Word16 i_subfr        /* (i)    : subframe flag                            */
);

Word16  ACELP_Codebook(  /* (o)     :index of pulses positions    */
  Word16 x[],            /* (i)     :Target vector                */
  Word16 h[],            /* (i) Q12 :Impulse response of filters  */
  Word16 T0,             /* (i)     :Pitch lag                    */
  Word16 pitch_sharp,    /* (i) Q14 :Last quantized pitch gain    */
  Word16 i_subfr,        /* (i)     :Indicator of 1st subframe,   */
  Word16 code[],         /* (o) Q13 :Innovative codebook          */
  Word16 y[],            /* (o) Q12 :Filtered innovative codebook */
  Word16 *sign           /* (o)     :Signs of 4 pulses            */
)
{
  Word16 i, index, sharp;
  Word16 Dn[L_SUBFR];
  Word16 rr[DIM_RR];

 /*-----------------------------------------------------------------*
  * Include fixed-gain pitch contribution into impulse resp. h[]    *
  * Find correlations of h[] needed for the codebook search.        *
  *-----------------------------------------------------------------*/

  sharp = shl(pitch_sharp, 1);          /* From Q14 to Q15 */
  if (sub(T0, L_SUBFR)<0)
     for (i = T0; i < L_SUBFR; i++){    /* h[i] += pitch_sharp*h[i-T0] */
       h[i] = add(h[i], mult(h[i-T0], sharp));
     }

  Cor_h(h, rr);

 /*-----------------------------------------------------------------*
  * Compute correlation of target vector with impulse response.     *
  *-----------------------------------------------------------------*/

  Cor_h_X(h, x, Dn);

 /*-----------------------------------------------------------------*
  * Find innovative codebook.                                       *
  *-----------------------------------------------------------------*/

  index = D4i40_17(Dn, rr, h, code, y, sign, i_subfr);

 /*-----------------------------------------------------------------*
  * Compute innovation vector gain.                                 *
  * Include fixed-gain pitch contribution into code[].              *
  *-----------------------------------------------------------------*/

  if(sub(T0 ,L_SUBFR) <0)
     for (i = T0; i < L_SUBFR; i++) {  /* code[i] += pitch_sharp*code[i-T0] */
       code[i] = add(code[i], mult(code[i-T0], sharp));
     }

  return index;
}

/*--------------------------------------------------------------------------*
 *  Function  Corr_h_X()                                                    *
 *  ~~~~~~~~~~~~~~~~~~~~                                                    *
 * Compute  correlations of input response h[] with the target vector X[].  *
 *--------------------------------------------------------------------------*/

static void Cor_h_X(
     Word16 h[],        /* (i) Q12 :Impulse response of filters      */
     Word16 X[],        /* (i)     :Target vector                    */
     Word16 D[]         /* (o)     :Correlations between h[] and D[] */
                        /*          Normalized to 13 bits            */
)
{
   Word16 i, j;
   Word32 s, max, L_temp;
   Word32 y32[L_SUBFR];

   /* first keep the result on 32 bits and find absolute maximum */

   max = 0;

   for (i = 0; i < L_SUBFR; i++)
   {
     s = 0;
     for (j = i; j <  L_SUBFR; j++)
       s = L_mac(s, X[j], h[j-i]);

     y32[i] = s;

     s = L_abs(s);
     L_temp =L_sub(s,max);
     if(L_temp>0L) {
        max = s;
     }
   }

   /* Find the number of right shifts to do on y32[]  */
   /* so that maximum is on 13 bits                   */

   j = norm_l(max);
   if( sub(j,16) > 0) {
    j = 16;
   }

   j = sub(18, j);

   for(i=0; i<L_SUBFR; i++) {
     D[i] = extract_l( L_shr(y32[i], j) );
   }

   return;

}


/*--------------------------------------------------------------------------*
 *  Function  Cor_h()                                                       *
 *  ~~~~~~~~~~~~~~~~~                                                       *
 * Compute  correlations of h[]  needed for the codebook search.            *
 *--------------------------------------------------------------------------*/

static void Cor_h(
     Word16 *H,         /* (i) Q12 :Impulse response of filters */
     Word16 *rr         /* (o)     :Correlations of H[]         */
)
{
  Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
  Word16 *rri0i1, *rri0i2, *rri0i3, *rri0i4;
  Word16 *rri1i2, *rri1i3, *rri1i4;
  Word16 *rri2i3, *rri2i4;

  Word16 *p0, *p1, *p2, *p3, *p4;

  Word16 *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
  Word32 cor;
  Word16 i, k, ldec, l_fin_sup, l_fin_inf;
  Word16 h[L_SUBFR];
  Word32 L_tmp;

 /* Scaling h[] for maximum precision */

  cor = 0;
  for(i=0; i<L_SUBFR; i++)
    cor = L_mac(cor, H[i], H[i]);

  L_tmp = L_sub(extract_h(cor),32000);
  if(L_tmp>0L )
  {
    for(i=0; i<L_SUBFR; i++) {
      h[i] = shr(H[i], 1);}
  }
  else
  {
    k = norm_l(cor);
    k = shr(k, 1);

    for(i=0; i<L_SUBFR; i++) {
      h[i] = shl(H[i], k); }
  }


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

 /*------------------------------------------------------------*
  * Compute rri0i0[], rri1i1[], rri2i2[], rri3i3 and rri4i4[]  *
  *------------------------------------------------------------*/

  p0 = rri0i0 + NB_POS-1;   /* Init pointers to last position of rrixix[] */
  p1 = rri1i1 + NB_POS-1;
  p2 = rri2i2 + NB_POS-1;
  p3 = rri3i3 + NB_POS-1;
  p4 = rri4i4 + NB_POS-1;

  ptr_h1 = h;
  cor    = 0;
  for(i=0;  i<NB_POS; i++)
  {
    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p4-- = extract_h(cor);

    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p3-- = extract_h(cor);

    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p2-- = extract_h(cor);

    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p1-- = extract_h(cor);

    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p0-- = extract_h(cor);
  }

 /*-----------------------------------------------------------------*
  * Compute elements of: rri2i3[], rri1i2[], rri0i1[] and rri0i4[]  *
  *-----------------------------------------------------------------*/

  l_fin_sup = MSIZE-1;
  l_fin_inf = l_fin_sup-(Word16)1;
  ldec = NB_POS+1;

  ptr_hd = h;
  ptr_hf = ptr_hd + 1;

  for(k=0; k<NB_POS; k++) {

          p3 = rri2i3 + l_fin_sup;
          p2 = rri1i2 + l_fin_sup;
          p1 = rri0i1 + l_fin_sup;
          p0 = rri0i4 + l_fin_inf;
          cor = 0;
          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;

          for(i=k+(Word16)1; i<NB_POS; i++ ) {

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p3 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p2 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p1 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p0 = extract_h(cor);

                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p3 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p2 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p1 = extract_h(cor);

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
  l_fin_inf = l_fin_sup-(Word16)1;
  for(k=0; k<NB_POS; k++) {

          p4 = rri2i4 + l_fin_sup;
          p3 = rri1i3 + l_fin_sup;
          p2 = rri0i2 + l_fin_sup;
          p1 = rri1i4 + l_fin_inf;
          p0 = rri0i3 + l_fin_inf;

          cor = 0;
          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          for(i=k+(Word16)1; i<NB_POS; i++ ) {

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p4 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p3 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p2 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p1 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p0 = extract_h(cor);

                  p4 -= ldec;
                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p4 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p3 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p2 = extract_h(cor);


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
  l_fin_inf = l_fin_sup-(Word16)1;
  for(k=0; k<NB_POS; k++) {

          p4 = rri1i4 + l_fin_sup;
          p3 = rri0i3 + l_fin_sup;
          p2 = rri2i4 + l_fin_inf;
          p1 = rri1i3 + l_fin_inf;
          p0 = rri0i2 + l_fin_inf;

          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          cor = 0;
          for(i=k+(Word16)1; i<NB_POS; i++ ) {

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p4 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p3 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p2 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p1 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p0 = extract_h(cor);

                  p4 -= ldec;
                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p4 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p3 = extract_h(cor);

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
  l_fin_inf = l_fin_sup-(Word16)1;
  for(k=0; k<NB_POS; k++) {

          p3 = rri0i4 + l_fin_sup;
          p2 = rri2i3 + l_fin_inf;
          p1 = rri1i2 + l_fin_inf;
          p0 = rri0i1 + l_fin_inf;

          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          cor = 0;
          for(i=k+(Word16)1; i<NB_POS; i++ ) {

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p3 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p2 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p1 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p0 = extract_h(cor);

                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p3 = extract_h(cor);

          l_fin_sup -= NB_POS;
          l_fin_inf--;
          ptr_hf += STEP;
  }
  return;
}


/*------------------------------------------------------------------------*
 * Function  D4i40_17                                                     *
 *           ~~~~~~~~~                                                    *
 * Algebraic codebook for ITU 8 kb/s.                                      *
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

static Word16 extra;

static Word16 D4i40_17( /* (o)    : Index of pulses positions.               */
  Word16 Dn[],          /* (i)    : Correlations between h[] and Xn[].       */
  Word16 rr[],          /* (i)    : Correlations of impulse response h[].    */
  Word16 h[],           /* (i) Q12: Impulse response of filters.             */
  Word16 cod[],         /* (o) Q13: Selected algebraic codeword.             */
  Word16 y[],           /* (o) Q12: Filtered algebraic codeword.             */
  Word16 *sign,         /* (o)    : Signs of 4 pulses.                       */
  Word16 i_subfr        /* (i)    : subframe flag                            */
)
{
   Word16  i0, i1, i2, i3, ip0, ip1, ip2, ip3;
   Word16  i, j, time;
   Word16  ps0, ps1, ps2, ps3, alp, alp0;
   Word32  alp1, alp2, alp3, L32;
   Word16  ps3c, psc, alpha;
   Word16  average, max0, max1, max2, thres;
   Word32  L_temp;

   Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
   Word16 *rri0i1, *rri0i2, *rri0i3, *rri0i4;
   Word16 *rri1i2, *rri1i3, *rri1i4;
   Word16 *rri2i3, *rri2i4;

   Word16 *ptr_ri0i0, *ptr_ri1i1, *ptr_ri2i2, *ptr_ri3i3, *ptr_ri4i4;
   Word16 *ptr_ri0i1, *ptr_ri0i2, *ptr_ri0i3, *ptr_ri0i4;
   Word16 *ptr_ri1i2, *ptr_ri1i3, *ptr_ri1i4;
   Word16 *ptr_ri2i3, *ptr_ri2i4;

   Word16  p_sign[L_SUBFR];

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
  * Reset max_time for 1st subframe.                                      *
  *-----------------------------------------------------------------------*/

   if (i_subfr == 0){ extra = 30; }

 /*-----------------------------------------------------------------------*
  * Chose the sign of the impulse.                                        *
  *-----------------------------------------------------------------------*/

   for (i=0; i<L_SUBFR; i++)
   {
     if( Dn[i] >= 0)
     {
       p_sign[i] = 0x7fff;
     }
     else
     {
       p_sign[i] = (Word16)0x8000;
       Dn[i] = negate(Dn[i]);
     }
   }

 /*-------------------------------------------------------------------*
  * - Compute the search threshold after three pulses                 *
  *-------------------------------------------------------------------*/

   /* Find maximum of Dn[i0]+Dn[i1]+Dn[i2] */

   max0 = Dn[0];
   max1 = Dn[1];
   max2 = Dn[2];

   for (i = 5; i < L_SUBFR; i+=STEP)
   {
     if (sub(Dn[i]  , max0) > 0){ max0 = Dn[i];   }
     if (sub(Dn[i+1], max1) > 0){ max1 = Dn[i+1]; }
     if (sub(Dn[i+2], max2) > 0){ max2 = Dn[i+2]; }
   }
   max0 = add(max0, max1);
   max0 = add(max0, max2);

   /* Find average of Dn[i0]+Dn[i1]+Dn[i2] */

   L32 = 0;
   for (i = 0; i < L_SUBFR; i+=STEP)
   {
     L32 = L_mac(L32, Dn[i], 1);
     L32 = L_mac(L32, Dn[i+1], 1);
     L32 = L_mac(L32, Dn[i+2], 1);
   }
   average =extract_l( L_shr(L32, 4));   /* 1/8 of sum */

   /* thres = average + (max0-average)*THRESHFCB; */

   thres = sub(max0, average);
   thres = mult(thres, THRESHFCB);
   thres = add(thres, average);

  /*-------------------------------------------------------------------*
   * Modification of rrixiy[] to take signs into account.              *
   *-------------------------------------------------------------------*/

  ptr_ri0i1 = rri0i1;
  ptr_ri0i2 = rri0i2;
  ptr_ri0i3 = rri0i3;
  ptr_ri0i4 = rri0i4;

   for(i0=0; i0<L_SUBFR; i0+=STEP)
   {
     for(i1=1; i1<L_SUBFR; i1+=STEP)
     {
       *ptr_ri0i1 = mult(*ptr_ri0i1, mult(p_sign[i0], p_sign[i1]));
        ptr_ri0i1++;
       *ptr_ri0i2 = mult(*ptr_ri0i2, mult(p_sign[i0], p_sign[i1+1]));
        ptr_ri0i2++;
       *ptr_ri0i3 = mult(*ptr_ri0i3, mult(p_sign[i0], p_sign[i1+2]));
        ptr_ri0i3++;
       *ptr_ri0i4 = mult(*ptr_ri0i4, mult(p_sign[i0], p_sign[i1+3]));
        ptr_ri0i4++;

     }
   }

   ptr_ri1i2 = rri1i2;
   ptr_ri1i3 = rri1i3;
   ptr_ri1i4 = rri1i4;

   for(i1=1; i1<L_SUBFR; i1+=STEP)
   {
      for(i2=2; i2<L_SUBFR; i2+=STEP)
      {
        *ptr_ri1i2 = mult(*ptr_ri1i2, mult(p_sign[i1], p_sign[i2]));
         ptr_ri1i2++;
        *ptr_ri1i3 = mult(*ptr_ri1i3, mult(p_sign[i1], p_sign[i2+1]));
         ptr_ri1i3++;
        *ptr_ri1i4 = mult(*ptr_ri1i4, mult(p_sign[i1], p_sign[i2+2]));
         ptr_ri1i4++;

      }
   }

   ptr_ri2i3 = rri2i3;
   ptr_ri2i4 = rri2i4;

   for(i2=2; i2<L_SUBFR; i2+=STEP)
   {
      for(i3=3; i3<L_SUBFR; i3+=STEP)
      {
        *ptr_ri2i3 = mult(*ptr_ri2i3, mult(p_sign[i2], p_sign[i3]));
         ptr_ri2i3++;
        *ptr_ri2i4 = mult(*ptr_ri2i4, mult(p_sign[i2], p_sign[i3+1]));
         ptr_ri2i4++;

      }
   }

 /*-------------------------------------------------------------------*
  * Search the optimum positions of the four  pulses which maximize   *
  *     square(correlation) / energy                                  *
  * The search is performed in four  nested loops. At each loop, one  *
  * pulse contribution is added to the correlation and energy.        *
  *                                                                   *
  * The fourth loop is entered only if the correlation due to the     *
  *  contribution of the first three pulses exceeds the preset        *
  *  threshold.                                                       *
  *-------------------------------------------------------------------*/

 /* Default values */

 ip0    = 0;
 ip1    = 1;
 ip2    = 2;
 ip3    = 3;
 psc    = 0;
 alpha  = MAX_16;
 time   = add(MAX_TIME, extra);


 /* Four loops to search innovation code. */

 ptr_ri0i0 = rri0i0;    /* Init. pointers that depend on first loop */
 ptr_ri0i1 = rri0i1;
 ptr_ri0i2 = rri0i2;
 ptr_ri0i3 = rri0i3;
 ptr_ri0i4 = rri0i4;

 for (i0 = 0; i0 < L_SUBFR; i0 += STEP)        /* first pulse loop  */
 {
   ps0  = Dn[i0];
   alp0 = *ptr_ri0i0++;

   ptr_ri1i1 = rri1i1;    /* Init. pointers that depend on second loop */
   ptr_ri1i2 = rri1i2;
   ptr_ri1i3 = rri1i3;
   ptr_ri1i4 = rri1i4;

   for (i1 = 1; i1 < L_SUBFR; i1 += STEP)      /* second pulse loop */
   {
     ps1  = add(ps0, Dn[i1]);

     /* alp1 = alp0 + *ptr_ri1i1++ + 2.0 * ( *ptr_ri0i1++); */

     alp1 = L_mult(alp0, 1);
     alp1 = L_mac(alp1, *ptr_ri1i1++, 1);
     alp1 = L_mac(alp1, *ptr_ri0i1++, 2);

     ptr_ri2i2 = rri2i2;     /* Init. pointers that depend on third loop */
     ptr_ri2i3 = rri2i3;
     ptr_ri2i4 = rri2i4;

     for (i2 = 2; i2 < L_SUBFR; i2 += STEP)    /* third pulse loop */
      {
       ps2  = add(ps1, Dn[i2]);

       /* alp2 = alp1 + *ptr_ri2i2++ + 2.0 * (*ptr_ri0i2++ + *ptr_ri1i2++); */

       alp2 = L_mac(alp1, *ptr_ri2i2++, 1);
       alp2 = L_mac(alp2, *ptr_ri0i2++, 2);
       alp2 = L_mac(alp2, *ptr_ri1i2++, 2);

       /* Test threshold */

       if ( sub(ps2, thres) > 0)
       {

         ptr_ri3i3 = rri3i3;    /* Init. pointers that depend on 4th loop */


         for (i3 = 3; i3 < L_SUBFR; i3 += STEP)      /* 4th pulse loop */
         {
           ps3 = add(ps2, Dn[i3]);

           /* alp3 = alp2 + *ptr_ri3i3++                                */
           /*       + 2.0*( *ptr_ri0i3++ + *ptr_ri1i3++ + *ptr_ri2i3++); */

           alp3 = L_mac(alp2, *ptr_ri3i3++, 1);
           alp3 = L_mac(alp3, *ptr_ri0i3++, 2);
           alp3 = L_mac(alp3, *ptr_ri1i3++, 2);
           alp3 = L_mac(alp3, *ptr_ri2i3++, 2);
           alp  = extract_l(L_shr(alp3, 5));

           ps3c = mult(ps3, ps3);
           L_temp = L_mult(ps3c, alpha);
           L_temp = L_msu(L_temp, psc, alp);
           if( L_temp > 0L )
           {
             psc = ps3c;
             alpha = alp;
             ip0 = i0;
             ip1 = i1;
             ip2 = i2;
             ip3 = i3;
           }
         }  /*  end of for i3 = */
         ptr_ri0i3 -= NB_POS;
         ptr_ri1i3 -= NB_POS;

         ptr_ri4i4 = rri4i4;    /* Init. pointers that depend on 4th loop */


         for (i3 = 4; i3 < L_SUBFR; i3 += STEP)      /* 4th pulse loop */
         {
           ps3 = add(ps2, Dn[i3]);

           /* alp3 = alp2 + *ptr_ri4i4++                                */
           /*       + 2.0*( *ptr_ri0i4++ + *ptr_ri1i4++ + *ptr_ri2i4++); */

           alp3 = L_mac(alp2, *ptr_ri4i4++, 1);
           alp3 = L_mac(alp3, *ptr_ri0i4++, 2);
           alp3 = L_mac(alp3, *ptr_ri1i4++, 2);
           alp3 = L_mac(alp3, *ptr_ri2i4++, 2);
           alp  = extract_l(L_shr(alp3, 5));

           ps3c = mult(ps3, ps3);
           L_temp = L_mult(ps3c, alpha);
           L_temp = L_msu(L_temp, psc, alp);
           if( L_temp > 0L )
           {
             psc = ps3c;
             alpha = alp;
             ip0 = i0;
             ip1 = i1;
             ip2 = i2;
             ip3 = i3;
           }
         }  /*  end of for i3 = */
         ptr_ri0i4 -= NB_POS;
         ptr_ri1i4 -= NB_POS;

         time = sub(time, 1);
         if(time <= 0 ) goto end_search;     /* Maximum time finish */

       }  /* end of if >thres */
       else
       {
         ptr_ri2i3 += NB_POS;
         ptr_ri2i4 += NB_POS;
       }

     } /* end of for i2 = */

     ptr_ri0i2 -= NB_POS;
     ptr_ri1i3 += NB_POS;
     ptr_ri1i4 += NB_POS;

   } /* end of for i1 = */

   ptr_ri0i2 += NB_POS;
   ptr_ri0i3 += NB_POS;
   ptr_ri0i4 += NB_POS;

 } /* end of for i0 = */

end_search:

 extra = time;

 /* Set the sign of impulses */

 i0 = p_sign[ip0];
 i1 = p_sign[ip1];
 i2 = p_sign[ip2];
 i3 = p_sign[ip3];

 /* Find the codeword corresponding to the selected positions */

 for(i=0; i<L_SUBFR; i++) {cod[i] = 0; }

 cod[ip0] = shr(i0, 2);         /* From Q15 to Q13 */
 cod[ip1] = shr(i1, 2);
 cod[ip2] = shr(i2, 2);
 cod[ip3] = shr(i3, 2);

 /* find the filtered codeword */

 for (i = 0; i < L_SUBFR; i++) {y[i] = 0;  }

 if(i0 > 0)
   for(i=ip0, j=0; i<L_SUBFR; i++, j++) {
       y[i] = add(y[i], h[j]); }
 else
   for(i=ip0, j=0; i<L_SUBFR; i++, j++) {
       y[i] = sub(y[i], h[j]); }

 if(i1 > 0)
   for(i=ip1, j=0; i<L_SUBFR; i++, j++) {
       y[i] = add(y[i], h[j]); }
 else
   for(i=ip1, j=0; i<L_SUBFR; i++, j++) {
       y[i] = sub(y[i], h[j]); }

 if(i2 > 0)
   for(i=ip2, j=0; i<L_SUBFR; i++, j++) {
       y[i] = add(y[i], h[j]); }
 else
   for(i=ip2, j=0; i<L_SUBFR; i++, j++) {
       y[i] = sub(y[i], h[j]); }

 if(i3 > 0)
   for(i=ip3, j=0; i<L_SUBFR; i++, j++) {
       y[i] = add(y[i], h[j]); }
 else
   for(i=ip3, j=0; i<L_SUBFR; i++, j++) {
       y[i] = sub(y[i], h[j]); }

 /* find codebook index;  17-bit address */

 i = 0;
 if(i0 > 0) i = add(i, 1);
 if(i1 > 0) i = add(i, 2);
 if(i2 > 0) i = add(i, 4);
 if(i3 > 0) i = add(i, 8);
 *sign = i;

 ip0 = mult(ip0, 6554);         /* ip0/5 */
 ip1 = mult(ip1, 6554);         /* ip1/5 */
 ip2 = mult(ip2, 6554);         /* ip2/5 */
 i   = mult(ip3, 6554);         /* ip3/5 */
 j   = add(i, shl(i, 2));       /* j = i*5 */
 j   = sub(ip3, add(j, 3));     /* j= ip3%5 -3 */
 ip3 = add(shl(i, 1), j);

 i = add(ip0, shl(ip1, 3));
 i = add(i  , shl(ip2, 6));
 i = add(i  , shl(ip3, 9));


 return i;
}
