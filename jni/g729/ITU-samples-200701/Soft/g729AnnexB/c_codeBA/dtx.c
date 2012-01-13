/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

/* DTX and Comfort Noise Generator - Encoder part */

#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8a.h"
#include "oper_32b.h"
#include "tab_ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"
#include "sid.h"

/* Static Variables */
static Word16 lspSid_q[M] ;
static Word16 pastCoeff[MP1];
static Word16 RCoeff[MP1];
static Word16 sh_RCoeff;
static Word16 Acf[SIZ_ACF];
static Word16 sh_Acf[NB_CURACF];
static Word16 sumAcf[SIZ_SUMACF];
static Word16 sh_sumAcf[NB_SUMACF];
static Word16 ener[NB_GAIN];
static Word16 sh_ener[NB_GAIN];
static Word16 fr_cur;
static Word16 cur_gain;
static Word16 nb_ener;
static Word16 sid_gain;
static Word16 flag_chang;
static Word16 prev_energy;
static Word16 count_fr0;

/* Local functions */
static void Calc_pastfilt(Word16 *Coeff);
static void Calc_RCoeff(Word16 *Coeff, Word16 *RCoeff, Word16 *sh_RCoeff);
static Word16 Cmp_filt(Word16 *RCoeff, Word16 sh_RCoeff, Word16 *acf,
                                        Word16 alpha, Word16 Fracthresh);
static void Calc_sum_acf(Word16 *acf, Word16 *sh_acf,
                    Word16 *sum, Word16 *sh_sum, Word16 nb);
static void Update_sumAcf(void);

/*-----------------------------------------------------------*
 * procedure Init_Cod_cng:                                   *
 *           ~~~~~~~~~~~~                                    *
 *   Initialize variables used for dtx at the encoder        *
 *-----------------------------------------------------------*/
void Init_Cod_cng(void)
{
  Word16 i;

  for(i=0; i<SIZ_SUMACF; i++) sumAcf[i] = 0;
  for(i=0; i<NB_SUMACF; i++) sh_sumAcf[i] = 40;

  for(i=0; i<SIZ_ACF; i++) Acf[i] = 0;
  for(i=0; i<NB_CURACF; i++) sh_Acf[i] = 40;

  for(i=0; i<NB_GAIN; i++) sh_ener[i] = 40;
  for(i=0; i<NB_GAIN; i++) ener[i] = 0;

  cur_gain = 0;
  fr_cur = 0;
  flag_chang = 0;

  return;
}


/*-----------------------------------------------------------*
 * procedure Cod_cng:                                        *
 *           ~~~~~~~~                                        *
 *   computes DTX decision                                   *
 *   encodes SID frames                                      *
 *   computes CNG excitation for encoder update              *
 *-----------------------------------------------------------*/
void Cod_cng(
  Word16 *exc,          /* (i/o) : excitation array                     */
  Word16 pastVad,       /* (i)   : previous VAD decision                */
  Word16 *lsp_old_q,    /* (i/o) : previous quantized lsp               */
  Word16 *Aq,           /* (o)   : set of interpolated LPC coefficients */
  Word16 *ana,          /* (o)   : coded SID parameters                 */
  Word16 freq_prev[MA_NP][M],
                        /* (i/o) : previous LPS for quantization        */
  Word16 *seed          /* (i/o) : random generator seed                */
)
{

  Word16 i;

  Word16 curAcf[MP1];
  Word16 bid[M], zero[MP1];
  Word16 curCoeff[MP1];
  Word16 lsp_new[M];
  Word16 *lpcCoeff;
  Word16 cur_igain;
  Word16 energyq, temp;

  /* Update Ener and sh_ener */
  for(i = NB_GAIN-1; i>=1; i--) {
    ener[i] = ener[i-1];
    sh_ener[i] = sh_ener[i-1];
  }

  /* Compute current Acfs */
  Calc_sum_acf(Acf, sh_Acf, curAcf, &sh_ener[0], NB_CURACF);

  /* Compute LPC coefficients and residual energy */
  if(curAcf[0] == 0) {
    ener[0] = 0;                /* should not happen */
  }
  else {
    Set_zero(zero, MP1);
    Levinson(curAcf, zero, curCoeff, bid, &ener[0]);
  }

  /* if first frame of silence => SID frame */
  if(pastVad != 0) {
    ana[0] = 2;
    count_fr0 = 0;
    nb_ener = 1;
    Qua_Sidgain(ener, sh_ener, nb_ener, &energyq, &cur_igain);

  }
  else {
    nb_ener = add(nb_ener, 1);
    if(sub(nb_ener, NB_GAIN) > 0) nb_ener = NB_GAIN;
    Qua_Sidgain(ener, sh_ener, nb_ener, &energyq, &cur_igain);
      
    /* Compute stationarity of current filter   */
    /* versus reference filter                  */
    if(Cmp_filt(RCoeff, sh_RCoeff, curAcf, ener[0], FRAC_THRESH1) != 0) {
      flag_chang = 1;
    }
      
    /* compare energy difference between current frame and last frame */
    temp = abs_s(sub(prev_energy, energyq));
    temp = sub(temp, 2);
    if (temp > 0) flag_chang = 1;
      
    count_fr0 = add(count_fr0, 1);
    if(sub(count_fr0, FR_SID_MIN) < 0) {
      ana[0] = 0;               /* no transmission */
    }
    else {
      if(flag_chang != 0) {
        ana[0] = 2;             /* transmit SID frame */
      }
      else{
        ana[0] = 0;
      }
        
      count_fr0 = FR_SID_MIN;   /* to avoid overflow */
    }
  }


  if(sub(ana[0], 2) == 0) {
      
    /* Reset frame count and change flag */
    count_fr0 = 0;
    flag_chang = 0;
      
    /* Compute past average filter */
    Calc_pastfilt(pastCoeff);
    Calc_RCoeff(pastCoeff, RCoeff, &sh_RCoeff);

    /* Compute stationarity of current filter   */
    /* versus past average filter               */


    /* if stationary */
    /* transmit average filter => new ref. filter */
    if(Cmp_filt(RCoeff, sh_RCoeff, curAcf, ener[0], FRAC_THRESH2) == 0) {
      lpcCoeff = pastCoeff;
    }

    /* else */
    /* transmit current filter => new ref. filter */
    else {
      lpcCoeff = curCoeff;
      Calc_RCoeff(curCoeff, RCoeff, &sh_RCoeff);
    }

    /* Compute SID frame codes */

    Az_lsp(lpcCoeff, lsp_new, lsp_old_q); /* From A(z) to lsp */

    /* LSP quantization */
    lsfq_noise(lsp_new, lspSid_q, freq_prev, &ana[1]);

    prev_energy = energyq;
    ana[4] = cur_igain;
    sid_gain = tab_Sidgain[cur_igain];


  } /* end of SID frame case */

  /* Compute new excitation */
  if(pastVad != 0) {
    cur_gain = sid_gain;
  }
  else {
    cur_gain = mult_r(cur_gain, A_GAIN0);
    cur_gain = add(cur_gain, mult_r(sid_gain, A_GAIN1));
  }

  Calc_exc_rand(cur_gain, exc, seed, FLAG_COD);

  Int_qlpc(lsp_old_q, lspSid_q, Aq);
  for(i=0; i<M; i++) {
    lsp_old_q[i]   = lspSid_q[i];
  }

  /* Update sumAcf if fr_cur = 0 */
  if(fr_cur == 0) {
    Update_sumAcf();
  }

  return;
}

/*-----------------------------------------------------------*
 * procedure Update_cng:                                     *
 *           ~~~~~~~~~~                                      *
 *   Updates autocorrelation arrays                          *
 *   used for DTX/CNG                                        *
 *   If Vad=1 : updating of array sumAcf                     *
 *-----------------------------------------------------------*/
void Update_cng(
  Word16 *r_h,      /* (i) :   MSB of frame autocorrelation        */
  Word16 exp_r,     /* (i) :   scaling factor associated           */
  Word16 Vad        /* (i) :   current Vad decision                */
)
{
  Word16 i;
  Word16 *ptr1, *ptr2;

  /* Update Acf and shAcf */
  ptr1 = Acf + SIZ_ACF - 1;
  ptr2 = ptr1 - MP1;
  for(i=0; i<(SIZ_ACF-MP1); i++) {
    *ptr1-- = *ptr2--;
  }
  for(i=NB_CURACF-1; i>=1; i--) {
    sh_Acf[i] = sh_Acf[i-1];
  }

  /* Save current Acf */
  sh_Acf[0] = negate(add(16, exp_r));
  for(i=0; i<MP1; i++) {
    Acf[i] = r_h[i];
  }

  fr_cur = add(fr_cur, 1);
  if(sub(fr_cur, NB_CURACF) == 0) {
    fr_cur = 0;
    if(Vad != 0) {
      Update_sumAcf();
    }
  }

  return;
}


/*-----------------------------------------------------------*
 *         Local procedures                                  *
 *         ~~~~~~~~~~~~~~~~                                  *
 *-----------------------------------------------------------*/

/* Compute scaled autocorr of LPC coefficients used for Itakura distance */
/*************************************************************************/
static void Calc_RCoeff(Word16 *Coeff, Word16 *RCoeff, Word16 *sh_RCoeff)
{
  Word16 i, j;
  Word16 sh1;
  Word32 L_acc;
  
  /* RCoeff[0] = SUM(j=0->M) Coeff[j] ** 2 */
  L_acc = 0L;
  for(j=0; j <= M; j++) {
    L_acc = L_mac(L_acc, Coeff[j], Coeff[j]);
  }
  
  /* Compute exponent RCoeff */
  sh1 = norm_l(L_acc);
  L_acc = L_shl(L_acc, sh1);
  RCoeff[0] = round(L_acc);
  
  /* RCoeff[i] = SUM(j=0->M-i) Coeff[j] * Coeff[j+i] */
  for(i=1; i<=M; i++) {
    L_acc = 0L;
    for(j=0; j<=M-i; j++) {
      L_acc = L_mac(L_acc, Coeff[j], Coeff[j+i]);
    }
    L_acc = L_shl(L_acc, sh1);
    RCoeff[i] = round(L_acc);
  }
  *sh_RCoeff = sh1;
  return;
}

/* Compute Itakura distance and compare to threshold */
/*****************************************************/
static Word16 Cmp_filt(Word16 *RCoeff, Word16 sh_RCoeff, Word16 *acf,
                                        Word16 alpha, Word16 FracThresh)
{
  Word32 L_temp0, L_temp1;
  Word16 temp1, temp2, sh[2], ind;
  Word16 i;
  Word16 diff, flag;
  extern Flag Overflow;

  sh[0] = 0;
  sh[1] = 0;
  ind = 1;
  flag = 0;
  do {
    Overflow = 0;
    temp1 = shr(RCoeff[0], sh[0]);
    temp2 = shr(acf[0], sh[1]);
    L_temp0 = L_shr(L_mult(temp1, temp2),1);
    for(i=1; i <= M; i++) {
      temp1 = shr(RCoeff[i], sh[0]);
      temp2 = shr(acf[i], sh[1]);
      L_temp0 = L_mac(L_temp0, temp1, temp2);
    }
    if(Overflow != 0) {
      sh[(int)ind] = add(sh[(int)ind], 1);
      ind = sub(1, ind);
    }
    else flag = 1;
  } while (flag == 0);
  
  
  temp1 = mult_r(alpha, FracThresh);
  L_temp1 = L_add(L_deposit_l(temp1), L_deposit_l(alpha));
  temp1 = add(sh_RCoeff, 9);  /* 9 = Lpc_justif. * 2 - 16 + 1 */
  temp2 = add(sh[0], sh[1]);
  temp1 = sub(temp1, temp2);
  L_temp1 = L_shl(L_temp1, temp1);
  
  L_temp0 = L_sub(L_temp0, L_temp1);
  if(L_temp0 > 0L) diff = 1;
  else diff = 0;

  return(diff);
}

/* Compute past average filter */
/*******************************/
static void Calc_pastfilt(Word16 *Coeff)
{
  Word16 i;
  Word16 s_sumAcf[MP1];
  Word16 bid[M], zero[MP1];
  Word16 temp;
  
  Calc_sum_acf(sumAcf, sh_sumAcf, s_sumAcf, &temp, NB_SUMACF);
  
  if(s_sumAcf[0] == 0L) {
    Coeff[0] = 4096;
    for(i=1; i<=M; i++) Coeff[i] = 0;
    return;
  }

  Set_zero(zero, MP1);
  Levinson(s_sumAcf, zero, Coeff, bid, &temp);
  return;
}

/* Update sumAcf */
/*****************/
static void Update_sumAcf(void)
{
  Word16 *ptr1, *ptr2;
  Word16 i;

  /*** Move sumAcf ***/
  ptr1 = sumAcf + SIZ_SUMACF - 1;
  ptr2 = ptr1 - MP1;
  for(i=0; i<(SIZ_SUMACF-MP1); i++) {
    *ptr1-- = *ptr2--;
  }
  for(i=NB_SUMACF-1; i>=1; i--) {
    sh_sumAcf[i] = sh_sumAcf[i-1];
  }

  /* Compute new sumAcf */
  Calc_sum_acf(Acf, sh_Acf, sumAcf, sh_sumAcf, NB_CURACF);
  return;
}

/* Compute sum of acfs (curAcf, sumAcf or s_sumAcf) */
/****************************************************/
static void Calc_sum_acf(Word16 *acf, Word16 *sh_acf,
                         Word16 *sum, Word16 *sh_sum, Word16 nb)
{

  Word16 *ptr1;
  Word32 L_temp, L_tab[MP1];
  Word16 sh0, temp;
  Word16 i, j;
  
  /* Compute sum = sum of nb acfs */
  /* Find sh_acf minimum */
  sh0 = sh_acf[0];
  for(i=1; i<nb; i++) {
    if(sub(sh_acf[i], sh0) < 0) sh0 = sh_acf[i];
  }
  sh0 = add(sh0, 14);           /* 2 bits of margin */

  for(j=0; j<MP1; j++) {
    L_tab[j] = 0L;
  }
  ptr1 = acf;
  for(i=0; i<nb; i++) {
    temp = sub(sh0, sh_acf[i]);
    for(j=0; j<MP1; j++) {
      L_temp = L_deposit_l(*ptr1++);
      L_temp = L_shl(L_temp, temp); /* shift right if temp<0 */
      L_tab[j] = L_add(L_tab[j], L_temp);
    }
  } 
  temp = norm_l(L_tab[0]);
  for(i=0; i<=M; i++) {
    sum[i] = extract_h(L_shl(L_tab[i], temp));
  }
  temp = sub(temp, 16);
  *sh_sum = add(sh0, temp);
  return;
}
