/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
                         Version 1.1 of October 1999
*/
/*
 File : bwfwfunc.c
 */
/* from bwfwfunc.c G.729 Annex E Version 1.2  Last modified: May 1998 */

#include <stdio.h>
#include <math.h>

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "tabld8cp.h"

#define LOG2_Q11 617
 /* -------------------------------------------------------------------- */
 /*                          TST_BWD_DOMINANT                            */
 /* -------------------------------------------------------------------- */
 /* Test if backward mode is dominant :                                  */
 /* Nbre of Backward frames > 4 * Nbre of Forward frames                 */
 /* It is used for the choice of the perceptual weighting filter         */
 /* Backward dominant : music or stationnary background noise :          */
 /*         Quantized filter (30 coef.) + strong weighting               */
 /* No backward dominant => Non Stationnary signal (mainly speech) :     */
 /*         Unquantized filter (10 coef.) + normal weighting             */
 /* -------------------------------------------------------------------- */
static Word16  count_bwd2 = 0, count_fwd2 = 0;

void tst_bwd_dominant(
    Word16 *bwd_dominant, /* O   Dominant backward mode indication */
    Word16 mode)          /* I   Selected LPC mode */

{

    Word16 tmp, count_all;

    if (mode == 0) count_fwd2++;
    else count_bwd2++;

    count_all = add(count_bwd2, count_fwd2);

    if (count_all == 100) {
        count_all = shr(count_all, 1);
        count_bwd2 = shr(count_bwd2, 1);
        count_fwd2 = shr(count_fwd2, 1);
    }

    *bwd_dominant = 0;
    if (count_all >= 10) {
        tmp = shl(count_fwd2, 2);
        if (count_bwd2 > tmp) *bwd_dominant = 1;
    }

    return;
}


void Int_bwd(Word16 *a_bwd, /* I/O   LPC backward filter */
                  Word16 *prev_filter, /* I previous frame filter */
                  Word16 *C_int     /* I/O interpolation coefficient */
)
{
    Word16  i, tmp1, tmp2, tmp;
    Word32  tmp_L;
    Word16 *pa_bwd;
    Word16 C_int_loc;

    pa_bwd = a_bwd + M_BWDP1;

    /* Calculate the interpolated filters  */
    /* ----------------------------------  */
    C_int_loc = sub(*C_int, 410);
    if( C_int_loc < 0) C_int_loc = 0;

    tmp = sub(4096, C_int_loc);

    for (i=0; i<M_BWDP1; i++) {
        tmp_L = L_mult(pa_bwd[i], tmp);
        tmp_L = L_shr(tmp_L, 13);
        tmp1 = extract_l(tmp_L);
        tmp_L = L_mult(prev_filter[i], C_int_loc);
        tmp_L = L_shr(tmp_L, 13);
        tmp2 = extract_l(tmp_L);
        pa_bwd[i] = add(tmp1, tmp2);
    }

    for (i=0; i<M_BWDP1; i++) {
        tmp1 = shr(pa_bwd[i], 1);
        tmp2 = shr(prev_filter[i], 1);
        a_bwd[i] = add(tmp1, tmp2);
    }

    *C_int = C_int_loc;
    return;
}

/* ---------------------------------------------------------------------- */
/*                                 ENER_DB                                */
/*                                                                        */
/*                        COMPUTATION OF THE ENERGY                       */
/*                                                                        */
/* ---------------------------------------------------------------------- */
Word16 ener_dB(Word16 *synth, Word16 L) {

    Word16  i, j;
    Word16  log_ener, index;
    Word16  res_ener;
    Word32  energy_L = 1;

    for (i=0; i<L; i++) {
        energy_L = L_mac(energy_L, synth[i], synth[i]);
    }

    energy_L = L_shr(energy_L, 1);

    i = 0;
    while (energy_L > 32) {
        energy_L = L_shr(energy_L, 1);
        i++;
    }
    res_ener = extract_l(energy_L);

    log_ener = LOG2_Q11;
    for (j=1; j<i; j++) log_ener = add(log_ener, LOG2_Q11);
    index = sub(res_ener, 16);
    if (index >= 0) log_ener = add(tab_log[index], log_ener);
    else log_ener = 1;

    return(log_ener);
}
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*                              HYBRID WINDOW                              */
/*                              (as in G728)                               */
/*                                                                         */
/*               1st part (0..35) is sine(i*c) where c = 0.047783          */
/*               2nd part (36....) is b*exp((i-35)*log(a))                 */
/*               where b=sine(35*c)                                        */
/*               and a=0.9928337491 is such as a^^40 = 0.75                */
/*               a^^(2*L_FRAME) = 0.75^^4 = 0.31640625                     */
/*                                                                         */
/* ------------------------------------------------------------------------ */

/*------------------------------------------------------------------------------*/
/* Compute the autocorrelation of speech using hybrid window    (as in G.728)   */
/* Fixed Point Version                                                          */
/*------------------------------------------------------------------------------*/
void autocorr_hyb_window(
    Word16  *x,         /* (in)     synthetized speech signal */
    Word32  *r_bwd,       /* (out)    Autocorrelations    */
    Word32 *rexp        /* (in/out) */
)

{
    Word32  tmp_L;
    Word16  rexp_l, rexp_h;
    Word16  i, n, k;
    Word16   w_s[L_ANA_BWD];

    for (n=L_ANA_BWD_M1, k=0; n>=0; n--, k++) w_s[n] = mult(x[n], hw[k]);

    for (i=0; i<=M_BWD; i++) {
        tmp_L = 0;
        for (n=M_BWD; n<N1; n++) tmp_L = L_mac(tmp_L, w_s[n], w_s[n-i]);
        tmp_L = L_shr(tmp_L, 1);
        L_Extract(rexp[i], &rexp_h, &rexp_l);
        rexp[i] = Mpy_32_16(rexp_h, rexp_l, W_FACT);
        rexp[i] = L_add(rexp[i], tmp_L);
    }

    for (i=0; i<=M_BWD; i++) {
        tmp_L = L_shl(rexp[i], 1);
        for (n=N1; n<L_ANA_BWD; n++) tmp_L = L_mac(tmp_L, w_s[n], w_s[n-i]);
        r_bwd[i] = L_shr(tmp_L, 1);
    }
    return;

} /* end of autocorr_hyb_windowing() */


/* ---------------------------------------------------------------------- */
/*                                                                        */
/*                             LAG_WINDOW_BWD                             */
/*                                                                        */
/*      Fixed-point Lag Windowing for backward analysis correlations      */
/*           (values of lag_h_bwd and lag_l_bwd in tab_ld8k.h)            */
/*                                                                        */
/* ---------------------------------------------------------------------- */
void Lag_window_bwd(Word32 *r_bwd, Word16 *r_h, Word16 *r_l)
{

    Word16  i, norm;
    Word32 sum;

    for (i=1; i<= M_BWD; i++) {
        L_Extract(r_bwd[i], &r_h[i], &r_l[i]);
        r_bwd[i] = Mpy_32(r_h[i], r_l[i], lag_h_bwd[i-1], lag_l_bwd[i-1]);
    }
    sum = L_add(r_bwd[0], 1);

    norm = norm_l(sum);
    sum  = L_shl(sum, norm);
    L_Extract(sum, &r_h[0], &r_l[0]);     /* Put in DPF format (see oper_32b) */

    for (i = 1; i <=M_BWD; i++) {
        sum = L_add(r_bwd[i], 1);
        sum = L_shl(sum, norm);
        L_Extract(sum, &r_h[i], &r_l[i]);
    }
    return;
}

