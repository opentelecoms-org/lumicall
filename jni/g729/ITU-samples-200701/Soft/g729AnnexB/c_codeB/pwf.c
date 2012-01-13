/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Speech Coder ANSI-C Source Code
   Version 3.3    Last modified: December 26, 1995

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"

static Word16     smooth = 1;
static Word16     LarOld[2] = {0, 0};

/************************************************************************/
/*                                                                      */
/*   ADAPTIVE BANDWIDTH EXPANSION FOR THE PERCEPTUAL WEIGHTING FILTER   */
/*                                                                      */
/*                 W(z) = A (z/gamma1) / A(z/gamma2)                    */
/*                                                                      */
/************************************************************************/

void perc_var (
  Word16 *gamma1, /* Bandwidth expansion parameter */
  Word16 *gamma2, /* Bandwidth expansion parameter */
  Word16 *LsfInt, /* Interpolated LSP vector : 1st subframe */
  Word16 *LsfNew, /* New LSP vector : 2nd subframe */
  Word16 *r_c     /* Reflection coefficients */
)
{

  Word32   L_temp;
  Word16   cur_rc;                    /* Q11 */
  Word16   Lar[4];                    /* Q11 */
  Word16  *LarNew;                    /* Q11 */
  Word16  *Lsf;                       /* Q15 */
  Word16   CritLar0, CritLar1;        /* Q11 */
  Word16   temp;
  Word16   d_min;                     /* Q10 */
  Word16   i, k;



  for (k=0; k<M; k++) {
    LsfInt[k] = shl(LsfInt[k], 1);
    LsfNew[k] = shl(LsfNew[k], 1);
  }


  LarNew = &Lar[2];
  /* ---------------------------------------- */
  /* Reflection coefficients ---> Lar         */
  /* Lar(i) = log10( (1+rc) / (1-rc) )        */
  /* Approximated by                          */
  /* x <= SEG1            y = x               */
  /* SEG1 < x <= SEG2     y = A1 x - B1_L     */
  /* SEG2 < x <= SEG3     y = A2 x - B2_L     */
  /* x > SEG3             y = A3 x - B3_L     */
  /* ---------------------------------------- */
  for (i=0; i<2; i++) {

    cur_rc = abs_s(r_c[i]);
    cur_rc = shr(cur_rc, 4);

    if (sub(cur_rc ,SEG1)<= 0) {
        LarNew[i] = cur_rc;
    }
    else {
      if (sub(cur_rc,SEG2)<= 0) {
        cur_rc = shr(cur_rc, 1);
        L_temp = L_mult(cur_rc, A1);
        L_temp = L_sub(L_temp, L_B1);
        L_temp = L_shr(L_temp, 11);
        LarNew[i] = extract_l(L_temp);
      }
      else {
        if (sub(cur_rc ,SEG3)<= 0) {
          cur_rc = shr(cur_rc, 1);
          L_temp = L_mult(cur_rc, A2);
          L_temp = L_sub(L_temp, L_B2);
          L_temp = L_shr(L_temp, 11);
          LarNew[i] = extract_l(L_temp);
        }
        else {
          cur_rc = shr(cur_rc, 1);
          L_temp = L_mult(cur_rc, A3);
          L_temp = L_sub(L_temp, L_B3);
          L_temp = L_shr(L_temp, 11);
          LarNew[i] = extract_l(L_temp);
        }
      }
    }
    if (r_c[i] < 0) {
        LarNew[i] = sub(0, LarNew[i]);

    }
  }

  /* Interpolation of Lar for the 1st subframe */

  temp = add(LarNew[0], LarOld[0]);
  Lar[0] = shr(temp, 1);
  LarOld[0] = LarNew[0];
  temp = add(LarNew[1], LarOld[1]);
  Lar[1] = shr(temp, 1);
  LarOld[1] = LarNew[1];

  for (k=0; k<2; k++) { /* LOOP : gamma2 for 1st to 2nd subframes */

      /* ---------------------------------------------------------- */
      /* First criterion based on the first two Lars                */
      /* smooth == 1  ==>  gamma2 can vary from 0.4 to 0.7          */
      /* smooth == 0  ==>  gamma2 is set to 0.6                     */
      /*                                                            */
      /* Double threshold + hysteresis :                            */
      /* if smooth = 1                                              */
      /*  if (CritLar0 < THRESH_L1) and (CritLar1 > THRESH_H1)      */
      /*                                                 smooth = 0 */
      /* if smooth = 0                                              */
      /*  if (CritLar0 > THRESH_L2) or (CritLar1 < THRESH_H2)       */
      /*                                                 smooth = 1 */
      /* ---------------------------------------------------------- */

      CritLar0 = Lar[2*k];
      CritLar1 = Lar[2*k+1];

      if (smooth != 0) {
        if ((sub(CritLar0,THRESH_L1)<0)&&( sub(CritLar1,THRESH_H1)>0)) {
            smooth = 0;
        }
      }
      else {
        if ( (sub(CritLar0 ,THRESH_L2)>0) || (sub(CritLar1,THRESH_H2) <0) ) {
            smooth = 1;
        }

      }

    if (smooth == 0) {
      /* ------------------------------------------------------ */
      /* Second criterion based on the minimum distance between */
      /*                two successives LSPs                    */
      /*                                                        */
      /*           gamma2[k] = -6.0 * pi * d_min + 1.0          */
      /*                                                        */
      /*       with Lsfs normalized range 0.0 <= val <= 1.0     */
      /* ------------------------------------------------------ */
      gamma1[k] = GAMMA1_0;
      if (k == 0) {
        Lsf = LsfInt;
      }
      else {
        Lsf = LsfNew;
      }
      d_min = sub(Lsf[1], Lsf[0]);
      for (i=1; i<M-1; i++) {
        temp = sub(Lsf[i+1],Lsf[i]);
        if (sub(temp,d_min)<0) {
            d_min = temp;
        }
      }
      temp = mult(ALPHA, d_min);
      temp = sub(BETA, temp);
      temp = shl(temp, 5);
      gamma2[k] = temp;

      if (sub(gamma2[k] , GAMMA2_0_H)>0) {
        gamma2[k] = GAMMA2_0_H;
      }
      if (sub(gamma2[k] ,GAMMA2_0_L)<0) {
        gamma2[k] = GAMMA2_0_L;
      }

    }
    else {
      gamma1[k] = GAMMA1_1;
      gamma2[k] = GAMMA2_1;
    }
  }
  return;
}

