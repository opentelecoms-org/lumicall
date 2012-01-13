/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.5    Last modified: October 2006
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/


#include <stdio.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8a.h"
#include "tab_ld8a.h"
#include "sid.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"

/* local functions */
static void Qnt_e(Word16 *errlsf,    /* (i)  : error lsf vector             */
                  Word16 *weight,    /* (i)  : weighting vector             */
                  Word16 DIn,        /* (i)  : number of input candidates   */
                  Word16 *qlsf,      /* (o)  : quantized error lsf vector   */
                  Word16 *Pptr,      /* (o)  : predictor index              */
                  Word16 DOut,       /* (i)  : number of quantized vectors  */
                  Word16 *cluster,   /* (o)  : quantizer indices            */
                  Word16 *MS         /* (i)  : size of the quantizers       */
                  );

static void New_ML_search_1(Word16 *d_data,    /* (i) : error vector             */
                            Word16 J,          /* (i) : number of input vectors  */
                            Word16 *new_d_data,/* (o) : output vector            */
                            Word16 K,          /* (i) : number of candidates     */
                            Word16 *best_indx, /* (o) : best indices             */
                            Word16 *ptr_back,  /* (o) : pointer for backtracking */
                            Word16 *PtrTab,    /* (i) : quantizer table          */
                            Word16 MQ          /* (i) : size of quantizer        */
                            );

static void New_ML_search_2(Word16 *d_data,    /* (i) : error vector             */
                            Word16 *weight,    /* (i) : weighting vector         */
                            Word16 J,          /* (i) : number of input vectors  */
                            Word16 *new_d_data,/* (o) : output vector            */
                            Word16 K,          /* (i) : number of candidates     */
                            Word16 *best_indx, /* (o) : best indices             */
                            Word16 *ptr_prd,   /* (i) : pointer for backtracking */
                            Word16 *ptr_back,  /* (o) : pointer for backtracking */
                            Word16 PtrTab[2][16],/* (i) : quantizer table        */
                            Word16 MQ          /* (i) : size of quantizer        */
                            );


/*-----------------------------------------------------------------*
 * Functions lsfq_noise                                            *
 *           ~~~~~~~~~~                                            *
 * Input:                                                          *
 *   lsp[]         : unquantized lsp vector                        *
 *   freq_prev[][] : memory of the lsf predictor                   *
 *                                                                 *
 * Output:                                                         *
 *                                                                 *
 *   lspq[]        : quantized lsp vector                          * 
 *   ana[]         : indices                                       *
 *                                                                 *
 *-----------------------------------------------------------------*/
void lsfq_noise(Word16 *lsp, 
                Word16 *lspq, 
                Word16 freq_prev[MA_NP][M], 
                Word16 *ana
                )
{
  Word16 i, lsf[M], lsfq[M], weight[M], tmpbuf[M];
  Word16 MS[MODE]={32, 16}, Clust[MODE], mode, errlsf[M*MODE];

  /* convert lsp to lsf */
  Lsp_lsf2(lsp, lsf, M);

  /* spacing to ~100Hz */
  if (lsf[0] < L_LIMIT)
    lsf[0] = L_LIMIT;
  for (i=0 ; i < M-1 ; i++)
    if (sub(lsf[i+1], lsf[i]) < 2*GAP3) 
      lsf[i+1] = add(lsf[i], 2*GAP3);
  if (lsf[M-1] > M_LIMIT)
    lsf[M-1] = M_LIMIT;
  if (lsf[M-1] < lsf[M-2]) 
    lsf[M-2] = sub(lsf[M-1], GAP3);

  /* get the lsf weighting */
  Get_wegt(lsf, weight);
  
  /**********************/
  /* quantize the lsf's */
  /**********************/
  
  /* get the prediction error vector */
  for (mode=0; mode<MODE; mode++)
    Lsp_prev_extract(lsf, errlsf+mode*M, noise_fg[mode], freq_prev, 
                     noise_fg_sum_inv[mode]);

  /* quantize the lsf and get the corresponding indices */
  Qnt_e(errlsf, weight, MODE, tmpbuf, &mode, 1, Clust, MS);
  ana[0] = mode;
  ana[1] = Clust[0];
  ana[2] = Clust[1];

  /* guarantee minimum distance of 0.0012 (~10 in Q13) between tmpbuf[j]
     and tmpbuf[j+1] */
  Lsp_expand_1_2(tmpbuf, 10);

  /* compute the quantized lsf vector */
  Lsp_prev_compose(tmpbuf, lsfq, noise_fg[mode], freq_prev, 
                   noise_fg_sum[mode]);
  
  /* update the prediction memory */
  Lsp_prev_update(tmpbuf, freq_prev);
  
  /* lsf stability check */
  Lsp_stability(lsfq);

  /* convert lsf to lsp */
  Lsf_lsp2(lsfq, lspq, M);

}

static void Qnt_e(Word16 *errlsf,    /* (i)  : error lsf vector             */
                  Word16 *weight,    /* (i)  : weighting vector             */
                  Word16 DIn,        /* (i)  : number of input candidates   */
                  Word16 *qlsf,      /* (o)  : quantized error lsf vector   */
                  Word16 *Pptr,      /* (o)  : predictor index              */
                  Word16 DOut,       /* (i)  : number of quantized vectors  */
                  Word16 *cluster,   /* (o)  : quantizer indices            */
                  Word16 *MS         /* (i)  : size of the quantizers       */
)
{
  Word16 d_data[2][R_LSFQ*M], best_indx[2][R_LSFQ];
  Word16 ptr_back[2][R_LSFQ], ptr, i;

  New_ML_search_1(errlsf, DIn, d_data[0], 4, best_indx[0], ptr_back[0], 
                  PtrTab_1, MS[0]);
  New_ML_search_2(d_data[0], weight, 4, d_data[1], DOut, best_indx[1], 
                  ptr_back[0], ptr_back[1], PtrTab_2, MS[1]);
  
  /* backward path for the indices */
  cluster[1] = best_indx[1][0];
  ptr = ptr_back[1][0];
  cluster[0] = best_indx[0][ptr];

  /* this is the pointer to the best predictor */
  *Pptr = ptr_back[0][ptr];
  
  /* generating the quantized vector */
  Copy(lspcb1[PtrTab_1[cluster[0]]], qlsf, M);
  for (i=0; i<M/2; i++)
    qlsf[i] = add(qlsf[i], lspcb2[PtrTab_2[0][cluster[1]]][i]);
  for (i=M/2; i<M; i++)
    qlsf[i] = add(qlsf[i], lspcb2[PtrTab_2[1][cluster[1]]][i]);

}

static void New_ML_search_1(Word16 *d_data,    /* (i) : error vector             */
                            Word16 J,          /* (i) : number of input vectors  */
                            Word16 *new_d_data,/* (o) : output vector            */
                            Word16 K,          /* (i) : number of candidates     */
                            Word16 *best_indx, /* (o) : best indices             */
                            Word16 *ptr_back,  /* (o) : pointer for backtracking */
                            Word16 *PtrTab,    /* (i) : quantizer table          */
                            Word16 MQ          /* (i) : size of quantizer        */
)
{
  Word16 tmp, m, l, p, q, sum[R_LSFQ*R_LSFQ];
  Word16 min[R_LSFQ], min_indx_p[R_LSFQ], min_indx_m[R_LSFQ];
  Word32 acc0;

  for (q=0; q<K; q++)
    min[q] = MAX_16;

  /* compute the errors */
  for (p=0; p<J; p++)
    for (m=0; m<MQ; m++){
      acc0 = 0;
      for (l=0; l<M; l++){
        tmp = sub(d_data[p*M+l], lspcb1[PtrTab[m]][l]);
        acc0 = L_mac(acc0, tmp, tmp);
      }
      sum[p*MQ+m] = extract_h(acc0);
      sum[p*MQ+m] = mult(sum[p*MQ+m], Mp[p]);
    }
      
  /* select the candidates */
  for (q=0; q<K; q++){
	min_indx_p[q] = 0;	/* G.729 maintenance */
	min_indx_m[q] = 0;	/* G.729 maintenance */
    for (p=0; p<J; p++)
      for (m=0; m<MQ; m++)
        if (sub(sum[p*MQ+m], min[q]) < 0){
          min[q] = sum[p*MQ+m];
		  min_indx_p[q] = p;
          min_indx_m[q] = m;          
        }
    
    sum[min_indx_p[q]*MQ+min_indx_m[q]] = MAX_16;
  }

  /* compute the candidates */
  for (q=0; q<K; q++){
    for (l=0; l<M; l++)
      new_d_data[q*M+l] = sub(d_data[min_indx_p[q]*M+l], 
                              lspcb1[PtrTab[min_indx_m[q]]][l]);
    
    ptr_back[q] = min_indx_p[q];
    best_indx[q] = min_indx_m[q];
  }
}

static void New_ML_search_2(Word16 *d_data,    /* (i) : error vector             */
                            Word16 *weight,    /* (i) : weighting vector         */
                            Word16 J,          /* (i) : number of input vectors  */
                            Word16 *new_d_data,/* (o) : output vector            */
                            Word16 K,          /* (i) : number of candidates     */
                            Word16 *best_indx, /* (o) : best indices             */
                            Word16 *ptr_prd,   /* (i) : pointer for backtracking */
                            Word16 *ptr_back,  /* (o) : pointer for backtracking */
                            Word16 PtrTab[2][16],/* (i) : quantizer table        */
                            Word16 MQ          /* (i) : size of quantizer        */
)
{
  Word16 m, l, p, q, sum[R_LSFQ*R_LSFQ];
  Word16 min[R_LSFQ], min_indx_p[R_LSFQ], min_indx_m[R_LSFQ];
  Word16 tmp1, tmp2;
  Word32 acc0;

  for (q=0; q<K; q++)
    min[q] = MAX_16;

  /* compute the errors */
  for (p=0; p<J; p++)
    for (m=0; m<MQ; m++){
      acc0 = 0;
      for (l=0; l<M/2; l++){
        tmp1 = extract_h(L_shl(L_mult(noise_fg_sum[ptr_prd[p]][l], 
                                      noise_fg_sum[ptr_prd[p]][l]), 2));
        tmp1 = mult(tmp1, weight[l]);
        tmp2 = sub(d_data[p*M+l], lspcb2[PtrTab[0][m]][l]);
        tmp1 = extract_h(L_shl(L_mult(tmp1, tmp2), 3));
        acc0 = L_mac(acc0, tmp1, tmp2);
      }

      for (l=M/2; l<M; l++){
        tmp1 = extract_h(L_shl(L_mult(noise_fg_sum[ptr_prd[p]][l], 
                                      noise_fg_sum[ptr_prd[p]][l]), 2));
        tmp1 = mult(tmp1, weight[l]);
        tmp2 = sub(d_data[p*M+l], lspcb2[PtrTab[1][m]][l]);
        tmp1 = extract_h(L_shl(L_mult(tmp1, tmp2), 3));
        acc0 = L_mac(acc0, tmp1, tmp2);
      }
      
      sum[p*MQ+m] = extract_h(acc0);
    }
      
  /* select the candidates */
  for (q=0; q<K; q++){
    min_indx_p[q] = 0;	/* G.729 maintenance */
    min_indx_m[q] = 0;	/* G.729 maintenance */
	for (p=0; p<J; p++)
      for (m=0; m<MQ; m++)
        if (sub(sum[p*MQ+m], min[q]) < 0){
          min[q] = sum[p*MQ+m];
          min_indx_p[q] = p;
          min_indx_m[q] = m;
        }
    
    sum[min_indx_p[q]*MQ+min_indx_m[q]] = MAX_16;
  }

  /* compute the candidates */
  for (q=0; q<K; q++){
    for (l=0; l<M/2; l++)
      new_d_data[q*M+l] = sub(d_data[min_indx_p[q]*M+l], 
                              lspcb2[PtrTab[0][min_indx_m[q]]][l]);
    for (l=M/2; l<M; l++)
      new_d_data[q*M+l] = sub(d_data[min_indx_p[q]*M+l], 
                              lspcb2[PtrTab[1][min_indx_m[q]]][l]);
    
    ptr_back[q] = min_indx_p[q];
    best_indx[q] = min_indx_m[q];
  }
}












