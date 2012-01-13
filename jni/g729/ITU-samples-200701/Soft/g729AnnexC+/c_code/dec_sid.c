/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/


/*
 File : DEC_SID.C
*/

/*
**
**
** Description:     Comfort noise generation
**                  performed at the decoder part
**
*/
/**** Floatting point version ***/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"
#include "tab_dtx.h"

static FLOAT cur_gain;
static FLOAT lspSid[M];
static FLOAT sid_gain;

/*
**
** Function:        init_dec_cng()
**
** Description:     Initialize dec_cng static variables
**
**
*/
void init_dec_cng(void)
{
    int i;
    
    for (i=0; i<M; i++ )
        lspSid[i] = (FLOAT)cos(freq_prev_reset[i]);
    
    sid_gain = tab_Sidgain[0];
    
    return;
}

/*-----------------------------------------------------------*
* procedure dec_cng:                                        *
*           ~~~~~~~~                                        *
*                     Receives frame type                   *
*                     0  :  for untransmitted frames        *
*                     2  :  for SID frames                  *
*                     Decodes SID frames                    *
*                     Computes current frame excitation     *
*                     Computes current frame LSPs
*-----------------------------------------------------------*/
void dec_cng(
    int past_ftyp,     /* (i)   : past frame type                      */
    FLOAT sid_sav,     /* (i)   : energy to recover SID gain           */
    int *parm,         /* (i)   : coded SID parameters                 */
    FLOAT *exc,        /* (i/o) : excitation array                     */
    FLOAT *lsp_old,    /* (i/o) : previous lsp                         */
    FLOAT *A_t,        /* (o)   : set of interpolated LPC coefficients */
    INT16 *seed,       /* (i/o) : random generator seed                */
    FLOAT freq_prev[MA_NP][M] /* (i/o) : previous LPS for quantization        */
)
{
    FLOAT temp;
    int ind;
    
    /* SID Frame */
    /*************/
    if(parm[0] != 0) {
        
        sid_gain = tab_Sidgain[parm[4]];
        
        /* Inverse quantization of the LSP */
        sid_lsfq_decode(&parm[1], lspSid, freq_prev);
        
    }
    
    /* non SID Frame */
    /*****************/
    else {
        
        /* Case of 1st SID frame erased : quantize-decode   */
        /* energy estimate stored in sid_gain         */
        if(past_ftyp > 1) {
            qua_Sidgain(&sid_sav, 0, &temp, &ind);
            sid_gain = tab_Sidgain[ind];
        }
        
    }
    
    if(past_ftyp > 1) {
        cur_gain = sid_gain;
    }
    else {
        cur_gain *= A_GAIN0;
        cur_gain += A_GAIN1 * sid_gain;
    }
    
    calc_exc_rand(cur_gain, exc, seed, FLAG_DEC);
    
    /* Interpolate the Lsp vectors */
    int_qlpc(lsp_old, lspSid, A_t);
    copy(lspSid, lsp_old, M);
    
    return;
}

/*---------------------------------------------------------------------------*
* Function  init_lsfq_noise                                                 *
* ~~~~~~~~~~~~~~~~~~~~~~~~~                                                 *
*                                                                           *
* -> Initialization of variables for the lsf quantization in the SID        *
*                                                                           *
*---------------------------------------------------------------------------*/
void init_lsfq_noise(void)
{
    int i, j;
    
    /* initialize the noise_fg */
    for (i=0; i<4; i++)
        copy(fg[0][i], noise_fg[0][i], M);
    
    for (i=0; i<4; i++)
        for (j=0; j<M; j++){
            noise_fg[1][i][j] = fg[0][i][j]*(F)0.6 + fg[1][i][j]* (F)0.4;
        }

    return;
}

void sid_lsfq_decode(
    int *index,             /* (i) : quantized indices    */
    FLOAT *lspq,              /* (o) : quantized lsp vector */
    FLOAT freq_prev[MA_NP][M] /* (i) : memory of predictor  */
)
{
    int i;
    FLOAT lsfq[M], tmpbuf[M];
    
    /* get the lsf error vector */
    copy(lspcb1[PtrTab_1[index[1]]], tmpbuf, M);
    for (i=0; i<M/2; i++)
        tmpbuf[i] = tmpbuf[i]+ lspcb2[PtrTab_2[0][index[2]]][i];
    for (i=M/2; i<M; i++)
        tmpbuf[i] = tmpbuf[i]+ lspcb2[PtrTab_2[1][index[2]]][i];
    
        /* guarantee minimum distance of 0.0012 between tmpbuf[j]
    and tmpbuf[j+1] */
    lsp_expand_1_2(tmpbuf, (FLOAT)0.0012);
    
    /* compute the quantized lsf vector */
    lsp_prev_compose(tmpbuf, lsfq, noise_fg[index[0]], freq_prev,
        noise_fg_sum[index[0]]);
    
    /* update the prediction memory */
    lsp_prev_update(tmpbuf, freq_prev);
    
    /* lsf stability check */
    lsp_stability(lsfq);
    
    /* convert lsf to lsp */
    for (i=0; i<M; i++ )
        lspq[i] = (FLOAT)cos(lsfq[i]);
    
}

