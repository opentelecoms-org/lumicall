/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.4    Last modified: November 2000

   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

/*
**
** File:            "dec_cng.c"
**
** Description:     Comfort noise generation
**                  performed at the decoder part
**
*/
/**** Fixed point version ***/

#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"
#include "basic_op.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"
#include "tab_dtx.h"

static Word16 cur_gain;
static Word16 lspSid[M]={
  31441,  27566,  21458,  13612,   4663,
  -4663, -13612, -21458, -27566, -31441};
static Word16 sid_gain;

/*
**
** Function:        Init_Dec_cng()
**
** Description:     Initialize dec_cng static variables
**
**
*/
void Init_Dec_cng(void)
{
  
  sid_gain = tab_Sidgain[0];

  return;
}

/*-----------------------------------------------------------*
 * procedure Dec_cng:                                        *
 *           ~~~~~~~~                                        *
 *                     Receives frame type                   *
 *                     0  :  for untransmitted frames        *
 *                     2  :  for SID frames                  *
 *                     Decodes SID frames                    *
 *                     Computes current frame excitation     *
 *                     Computes current frame LSPs
 *-----------------------------------------------------------*/
void Dec_cng(
  Word16 past_ftyp,     /* (i)   : past frame type                      */
  Word16 sid_sav,       /* (i)   : energy to recover SID gain           */
  Word16 sh_sid_sav,    /* (i)   : corresponding scaling factor         */
  Word16 *parm,         /* (i)   : coded SID parameters                 */
  Word16 *exc,          /* (i/o) : excitation array                     */
  Word16 *lsp_old,      /* (i/o) : previous lsp                         */
  Word16 *A_t,          /* (o)   : set of interpolated LPC coefficients */
  Word16 *seed,         /* (i/o) : random generator seed                */
  Word16 freq_prev[MA_NP][M]
                        /* (i/o) : previous LPS for quantization        */
)
{
  Word16 temp, ind;
  Word16 dif;

  dif = sub(past_ftyp, 1);
  
  /* SID Frame */
  /*************/
  if(parm[0] != 0) {

    sid_gain = tab_Sidgain[(int)parm[4]];           
    
    /* Inverse quantization of the LSP */
    sid_lsfq_decode(&parm[1], lspSid, freq_prev);
    
  }

  /* non SID Frame */
  /*****************/
  else {
    
    /* Case of 1st SID frame erased : quantize-decode   */
    /* energy estimate stored in sid_gain         */
    if(dif == 0) {
      Qua_Sidgain(&sid_sav, &sh_sid_sav, 0, &temp, &ind);
      sid_gain = tab_Sidgain[(int)ind];
    }
    
  }
  
  if(dif == 0) {
    cur_gain = sid_gain;
  }
  else {
    cur_gain = mult_r(cur_gain, A_GAIN0);
    cur_gain = add(cur_gain, mult_r(sid_gain, A_GAIN1));
  }
 
  Calc_exc_rand(cur_gain, exc, seed, FLAG_DEC);

  /* Interpolate the Lsp vectors */
  Int_qlpc(lsp_old, lspSid, A_t);
  Copy(lspSid, lsp_old, M);
  
  return;
}



/*---------------------------------------------------------------------------*
 * Function  Init_lsfq_noise                                                 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~                                                 *
 *                                                                           *
 * -> Initialization of variables for the lsf quantization in the SID        *
 *                                                                           *
 *---------------------------------------------------------------------------*/
void Init_lsfq_noise(void)
{
  Word16 i, j;
  Word32 acc0;

  /* initialize the noise_fg */
  for (i=0; i<4; i++)
    Copy(fg[0][i], noise_fg[0][i], M);
  
  for (i=0; i<4; i++)
    for (j=0; j<M; j++){
      acc0 = L_mult(fg[0][i][j], 19660);
      acc0 = L_mac(acc0, fg[1][i][j], 13107);
      noise_fg[1][i][j] = extract_h(acc0);
    }
}


void sid_lsfq_decode(Word16 *index,             /* (i) : quantized indices    */
                     Word16 *lspq,              /* (o) : quantized lsp vector */
                     Word16 freq_prev[MA_NP][M] /* (i) : memory of predictor  */
                     )
{
  Word32 acc0;
  Word16 i, j, k, lsfq[M], tmpbuf[M];

  /* get the lsf error vector */
  Copy(lspcb1[PtrTab_1[index[1]]], tmpbuf, M);
  for (i=0; i<M/2; i++)
    tmpbuf[i] = add(tmpbuf[i], lspcb2[PtrTab_2[0][index[2]]][i]);
  for (i=M/2; i<M; i++)
    tmpbuf[i] = add(tmpbuf[i], lspcb2[PtrTab_2[1][index[2]]][i]);

  /* guarantee minimum distance of 0.0012 (~10 in Q13) between tmpbuf[j] 
     and tmpbuf[j+1] */
  for (j=1; j<M; j++){
    acc0 = L_mult(tmpbuf[j-1], 16384);
    acc0 = L_mac(acc0, tmpbuf[j], -16384);
    acc0 = L_mac(acc0, 10, 16384);
    k = extract_h(acc0);

    if (k > 0){
      tmpbuf[j-1] = sub(tmpbuf[j-1], k);
      tmpbuf[j] = add(tmpbuf[j], k);
    }
  }
  
  /* compute the quantized lsf vector */
  Lsp_prev_compose(tmpbuf, lsfq, noise_fg[index[0]], freq_prev, 
                   noise_fg_sum[index[0]]);
  
  /* update the prediction memory */
  Lsp_prev_update(tmpbuf, freq_prev);
  
  /* lsf stability check */
  Lsp_stability(lsfq);

  /* convert lsf to lsp */
  Lsf_lsp2(lsfq, lspq, M);

}





