/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/


/*
 File : DTX.C
*/

/* DTX and Comfort Noise Generator - Encoder part */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"
#include "sid.h"

/* Static Variables */
static FLOAT lspSid_q[M] ;
static FLOAT pastCoeff[MP1];
static FLOAT RCoeff[MP1];
static FLOAT Acf[SIZ_ACF];
static FLOAT sumAcf[SIZ_SUMACF];
static FLOAT ener[NB_GAIN];
static int fr_cur;
static FLOAT cur_gain;
static int nb_ener;
static FLOAT sid_gain;
static int flag_chang;
static FLOAT prev_energy;
static int count_fr0;

/* Local functions */
static void calc_pastfilt(FLOAT *Coeff, FLOAT old_A[],  FLOAT old_rc[]);
static void calc_RCoeff(FLOAT *Coeff, FLOAT *RCoeff);
static int cmp_filt(FLOAT *RCoeff, FLOAT *acf, FLOAT alpha, FLOAT Thresh);
static void calc_sum_acf(FLOAT *acf, FLOAT *sum, int nb);
static void update_sumAcf(void);

/*-----------------------------------------------------------*
* procedure init_Cod_cng:                                   *
*           ~~~~~~~~~~~~                                    *
*   Initialize variables used for dtx at the encoder        *
*-----------------------------------------------------------*/
void init_cod_cng(void)
{
    int i;
    
    for(i=0; i<SIZ_SUMACF; i++) sumAcf[i] = (F)0.;
    
    for(i=0; i<SIZ_ACF; i++) Acf[i] = (F)0.;
    
    for(i=0; i<NB_GAIN; i++) ener[i] = (F)0.;
    
    cur_gain = 0;
    fr_cur = 0;
    flag_chang = 0;
    
    return;
}

/*-----------------------------------------------------------*
* procedure cod_cng:                                        *
*           ~~~~~~~~                                        *
*   computes DTX decision                                   *
*   encodes SID frames                                      *
*   computes CNG excitation for encoder update              *
*-----------------------------------------------------------*/
void cod_cng(
    FLOAT *exc,          /* (i/o) : excitation array                     */
    int pastVad,         /* (i)   : previous VAD decision                */
    FLOAT *lsp_old_q,    /* (i/o) : previous quantized lsp               */
    FLOAT *old_A,          /* (i/o) : last stable filter LPC coefficients  */
    FLOAT *old_rc,          /* (i/o) : last stable filter Reflection coefficients.*/
    FLOAT *Aq,           /* (o)   : set of interpolated LPC coefficients */
    int *ana,            /* (o)   : coded SID parameters                 */
    FLOAT freq_prev[MA_NP][M], /* (i/o) : previous LPS for quantization        */
    INT16 *seed          /* (i/o) : random generator seed                */
)
{
    int i;
    
    FLOAT curAcf[MP1];
    FLOAT bid[MP1];
    FLOAT curCoeff[MP1];
    FLOAT lsp_new[M];
    FLOAT *lpcCoeff;
    int cur_igain;
    FLOAT energyq;
    
    /* Update Ener */
    for(i = NB_GAIN-1; i>=1; i--) {
        ener[i] = ener[i-1];
    }
    
    /* Compute current Acfs */
    calc_sum_acf(Acf, curAcf, NB_CURACF);
    
    /* Compute LPC coefficients and residual energy */
    if(curAcf[0] == (F)0.) {
        ener[0] = (F)0.;                /* should not happen */
    }
    else {
        ener[0] = levinsone(M, curAcf, curCoeff, bid, old_A, old_rc);
    }

    /* if first frame of silence => SID frame */
    if(pastVad != 0) {
        ana[0] = 1;
        count_fr0 = 0;
        nb_ener = 1;
        qua_Sidgain(ener, nb_ener, &energyq, &cur_igain);
    }
    else {
        nb_ener++;
        if(nb_ener > NB_GAIN) nb_ener = NB_GAIN;
        qua_Sidgain(ener, nb_ener, &energyq, &cur_igain);

        /* Compute stationarity of current filter   */
        /* versus reference filter                  */
        if(cmp_filt(RCoeff, curAcf, ener[0], THRESH1) != 0) {
            flag_chang = 1;
        }
        
        /* compare energy difference between current frame and last frame */
        if( (FLOAT)fabs(prev_energy - energyq) > (F)2.0) flag_chang = 1;
        
        count_fr0++;
        if(count_fr0 < FR_SID_MIN) {
            ana[0] = 0; /* no transmission */
        }
        else {
            if(flag_chang != 0) {
                ana[0] = 1;             /* transmit SID frame */
            }
            else{
                ana[0] = 0;
            }
            count_fr0 = FR_SID_MIN;   /* to avoid overflow */
        }
    }

    if(ana[0] == 1) {

        /* Reset frame count and change flag */
        count_fr0 = 0;
        flag_chang = 0;
        
        /* Compute past average filter */
        calc_pastfilt(pastCoeff, old_A, old_rc);
        calc_RCoeff(pastCoeff, RCoeff);
        
        /* Compute stationarity of current filter   */
        /* versus past average filter               */
        
        /* if stationary */
        /* transmit average filter => new ref. filter */
        if(cmp_filt(RCoeff, curAcf, ener[0], THRESH2) == 0) {
            lpcCoeff = pastCoeff;
        }
        
        /* else */
        /* transmit current filter => new ref. filter */
        else {
            lpcCoeff = curCoeff;
            calc_RCoeff(curCoeff, RCoeff);
        }
        
        /* Compute SID frame codes */
        az_lsp(lpcCoeff, lsp_new, lsp_old_q); /* From A(z) to lsp */
        
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
        cur_gain *= A_GAIN0;
        cur_gain += A_GAIN1 * sid_gain;
    }
    
    calc_exc_rand(cur_gain, exc, seed, FLAG_COD);
    
    int_qlpc(lsp_old_q, lspSid_q, Aq);
    for(i=0; i<M; i++) {
        lsp_old_q[i]   = lspSid_q[i];
    }
    
    /* Update sumAcf if fr_cur = 0 */
    if(fr_cur == 0) {
        update_sumAcf();
    }
    
    return;
}

/*-----------------------------------------------------------*
* procedure update_cng:                                     *
*           ~~~~~~~~~~                                      *
*   updates autocorrelation arrays                          *
*   used for DTX/CNG                                        *
*   If Vad=1 : updating of array sumAcf                     *
*-----------------------------------------------------------*/
void update_cng(
    FLOAT *r,         /* (i) :   frame autocorrelation               */
    int Vad           /* (i) :   current Vad decision                */
)
{
    int i;
    FLOAT *ptr1, *ptr2;
    
    /* Update Acf */
    ptr1 = Acf + SIZ_ACF - 1;
    ptr2 = ptr1 - MP1;
    for(i=0; i<(SIZ_ACF-MP1); i++) {
        *ptr1-- = *ptr2--;
    }
    
    /* Save current Acf */
    for(i=0; i<MP1; i++) {
        Acf[i] = r[i];
    }
    
    fr_cur++;
    if(fr_cur == NB_CURACF) {
        fr_cur = 0;
        if(Vad != 0) {
            update_sumAcf();
        }
    }
    
    return;
}


/*-----------------------------------------------------------*
*         Local procedures                                  *
*         ~~~~~~~~~~~~~~~~                                  *
*-----------------------------------------------------------*/

/* Compute autocorr of LPC coefficients used for Itakura distance */
/******************************************************************/
static void calc_RCoeff(FLOAT *Coeff, FLOAT *RCoeff)
{
    int i, j;
    FLOAT temp;
    
    /* RCoeff[0] = SUM(j=0->M) Coeff[j] ** 2 */
    for(j=0, temp = (F)0.; j <= M; j++) {
        temp += Coeff[j] * Coeff[j];
    }
    RCoeff[0] = temp;
    
    /* RCoeff[i] = SUM(j=0->M-i) Coeff[j] * Coeff[j+i] */
    for(i=1; i<=M; i++) {
        for(j=0, temp=(F)0.; j<=M-i; j++) {
            temp += Coeff[j] * Coeff[j+i];
        }
        RCoeff[i] = (F)2. * temp;
    }
    return;
}

/* Compute Itakura distance and compare to threshold */
/*****************************************************/
static int cmp_filt(FLOAT *RCoeff, FLOAT *acf, FLOAT alpha, FLOAT Thresh)
{
    FLOAT temp1, temp2;
    int i;
    int diff;
    
    temp1 = (F)0.;
    for(i=0; i <= M; i++) {
        temp1 += RCoeff[i] * acf[i];
    }
    
    temp2 = alpha * Thresh;
    if(temp1 > temp2) diff = 1;
    else diff = 0;
    
    return(diff);
}

/* Compute past average filter */
/*******************************/
static void calc_pastfilt(FLOAT *Coeff, FLOAT old_A[],  FLOAT old_rc[])
{
    int i;
    FLOAT s_sumAcf[MP1];
    FLOAT bid[M];

    calc_sum_acf(sumAcf, s_sumAcf, NB_SUMACF);
    
    if(s_sumAcf[0] == (F)0.) {
        Coeff[0] = (F)1.;
        for(i=1; i<=M; i++) Coeff[i] = (F)0.;
        return;
    }

    levinsone(M, s_sumAcf, Coeff, bid, old_A, old_rc);
    return;
}

/* Update sumAcf */
/*****************/
static void update_sumAcf(void)
{
    FLOAT *ptr1, *ptr2;
    int i;
    
    /*** Move sumAcf ***/
    ptr1 = sumAcf + SIZ_SUMACF - 1;
    ptr2 = ptr1 - MP1;
    for(i=0; i<(SIZ_SUMACF-MP1); i++) {
        *ptr1-- = *ptr2--;
    }
    
    /* Compute new sumAcf */
    calc_sum_acf(Acf, sumAcf, NB_CURACF);
    return;
}

/* Compute sum of acfs (curAcf, sumAcf or s_sumAcf) */
/****************************************************/
static void calc_sum_acf(FLOAT *acf, FLOAT *sum, int nb)
{
    
    FLOAT *ptr1;
    int i, j;
    
    for(j=0; j<MP1; j++) {
        sum[j] = (F)0.;
    }
    ptr1 = acf;
    for(i=0; i<nb; i++) {
        for(j=0; j<MP1; j++) {
            sum[j] += (*ptr1++);
        }
    }
    return;
}
