/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
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
 File : POSTFIL.C
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/


/************************************************************************/
/*      Post - filtering : short term + long term                       */
/*      Floating point computation                                      */
/************************************************************************/
#include <math.h>
#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"

/* Prototypes for local functions */
static void pst_ltp(int t0, FLOAT *ptr_sig_in, FLOAT *ptr_sig_pst0, int *vo);
static void search_del(int t0, FLOAT *ptr_sig_in, int *ltpdel, int *phase,
            FLOAT *num_gltp, FLOAT *den_gltp, FLOAT *y_up, int *off_yup);
static void filt_plt( FLOAT *s_in, FLOAT *s_ltp, FLOAT *s_out, FLOAT gain_plt);
static void compute_ltp_l(FLOAT *s_in, int ltpdel, int phase, FLOAT *y_up,
 FLOAT *num, FLOAT *den);
static int select_ltp( FLOAT num1, FLOAT den1, FLOAT num2, FLOAT den2);
static void calc_st_filt(FLOAT *apond2, FLOAT *apond1, FLOAT *parcor0,
                                                FLOAT *signal_ltp0_ptr);
static void filt_mu(FLOAT *sig_in, FLOAT *sig_out, FLOAT parcor0);
static void calc_rc0_h(FLOAT *h, FLOAT *rc0);
static void scale_st(FLOAT *sig_in, FLOAT *sig_out, FLOAT *gain_prec);

/* Static arrays and variables */
static FLOAT apond2[LONG_H_ST];           /* s.t. numerator coeff.        */
static FLOAT mem_stp[M];            /* s.t. postfilter memory       */
static FLOAT mem_zero[M];          /* null memory to compute h_st  */
static FLOAT res2[SIZ_RES2];        /* A(gamma2) residual           */

/* Static pointers */
static FLOAT *res2_ptr;
static FLOAT *ptr_mem_stp;

/* Variables */
static FLOAT gain_prec;             /* for gain adjustment          */

/****   Short term postfilter :                                     *****/
/*      Hst(z) = Hst0(z) Hst1(z)                                        */
/*      Hst0(z) = 1/g0 A(gamma2)(z) / A(gamma1)(z)                      */
/*      if {hi} = i.r. filter A(gamma2)/A(gamma1) (truncated)           */
/*      g0 = SUM(|hi|) if > 1                                           */
/*      g0 = 1. else                                                    */
/*      Hst1(z) = 1/(1+ |mu|) (1 + mu z-1)                              */
/*      with mu = 1st parcor calculated on {hi}                         */
/****   Long term postfilter :                                      *****/
/*      harmonic postfilter :   H0(z) = gl * (1 + b * z-p)              */
/*      b = gamma_g * gain_ltp                                          */
/*      gl = 1 / 1 + b                                                  */
/*      copmuation of delay on A(gamma2)(z) s(z)                        */
/*      sub optimal research                                            */
/*      1. search best integer delay                                    */
/*      2. search around integer sub multiples (3 val. / sub mult)      */
/*      3. search around integer with fractionnal delays (1/8)          */
/************************************************************************/

/*----------------------------------------------------------------------------
 * init_pst -  Initialize postfilter functions
 *----------------------------------------------------------------------------
 */
void init_post_filter(
    void
)
{
    int i;

    /* Initialize arrays and pointers */

    /* A(gamma2) residual */
    for(i=0; i<MEM_RES2; i++) res2[i] = (F)0.0;
    res2_ptr = res2 + MEM_RES2;

    /* 1/A(gamma1) memory */
    for(i=0; i<M; i++) mem_stp[i] = (F)0.0;
    ptr_mem_stp = mem_stp + M - 1;

    /* fill apond2[M+1->LONG_H_ST-1] with zeroes */
    for(i=MP1; i<LONG_H_ST; i++) apond2[i] = (F)0.0;

    /* null memory to compute i.r. of A(gamma2)/A(gamma1) */
    for(i=0; i<M; i++) mem_zero[i] = (F)0.0;

    /* for gain adjustment */
    gain_prec =(F)1.;

    return;
}

/*----------------------------------------------------------------------------
 * post - adaptive postfilter main function
 *----------------------------------------------------------------------------
 */
void post(
 int t0,                /* input : pitch delay given by coder */
 FLOAT *signal_ptr,     /* input : input signal (pointer to current subframe */
 FLOAT *coeff,          /* input : LPC coefficients for current subframe */
 FLOAT *sig_out,        /* output: postfiltered output */
 int *vo                /* output: voicing decision 0 = uv,  > 0 delay */
)
{
    FLOAT apond1[MP1];           /* s.t. denominator coeff.      */
    FLOAT sig_ltp[L_SUBFRP1];   /* H0 output signal             */
    FLOAT *sig_ltp_ptr;
    FLOAT parcor0;

    /* Compute weighted LPC coefficients */
    weight_az(coeff, GAMMA1_PST, M, apond1);
    weight_az(coeff, GAMMA2_PST, M, apond2);

    /* Compute A(gamma2) residual */
    residu(apond2, signal_ptr, res2_ptr, L_SUBFR);

    /* Harmonic filtering */
    sig_ltp_ptr = sig_ltp + 1;
    pst_ltp(t0, res2_ptr, sig_ltp_ptr, vo);

    /* Save last output of 1/A(gamma1)  */
    /* (from preceding subframe)        */
    sig_ltp[0] = *ptr_mem_stp;

    /* Control short term pst filter gain and compute parcor0   */
    calc_st_filt(apond2, apond1, &parcor0, sig_ltp_ptr);

    /* 1/A(gamma1) filtering, mem_stp is updated */
    syn_filt(apond1, sig_ltp_ptr, sig_ltp_ptr, L_SUBFR, mem_stp, 1);

    /* (1 + mu z-1) tilt filtering */
    filt_mu(sig_ltp, sig_out, parcor0);

    /* gain control */
    scale_st(signal_ptr, sig_out, &gain_prec);

    /**** Update for next frame */
    copy(&res2[L_SUBFR], &res2[0], MEM_RES2);

    return;
}

/*----------------------------------------------------------------------------
 *  pst_ltp - harmonic postfilter
 *----------------------------------------------------------------------------
 */
static void pst_ltp(
 int t0,                /* input : pitch delay given by coder */
 FLOAT *ptr_sig_in,     /* input : postfilter input filter (residu2) */
 FLOAT *ptr_sig_pst0,   /* output: harmonic postfilter output */
 int *vo                /* output: voicing decision 0 = uv,  > 0 delay */
)
{

/**** Declare variables                                 */
    int ltpdel, phase;
    FLOAT num_gltp, den_gltp;
    FLOAT num2_gltp, den2_gltp;
    FLOAT gain_plt;
    FLOAT y_up[SIZ_Y_UP];
    FLOAT *ptr_y_up;
    int off_yup;

    /* Sub optimal delay search */
    search_del(t0, ptr_sig_in, &ltpdel, &phase, &num_gltp, &den_gltp,
                        y_up, &off_yup);
    *vo = ltpdel;

    if(num_gltp == (F)0.)  {
        copy(ptr_sig_in, ptr_sig_pst0, L_SUBFR);
    }
    else {

        if(phase == 0) {
            ptr_y_up = ptr_sig_in - ltpdel;
        }

        else {
            /* Filtering with long filter */
            compute_ltp_l(ptr_sig_in, ltpdel, phase, ptr_sig_pst0,
                &num2_gltp, &den2_gltp);

            if(select_ltp(num_gltp, den_gltp, num2_gltp, den2_gltp) == 1) {

                /* select short filter */
                ptr_y_up = y_up + ((phase-1) * L_SUBFRP1 + off_yup);
            }
            else {
                /* select long filter */
                num_gltp = num2_gltp;
                den_gltp = den2_gltp;
                ptr_y_up = ptr_sig_pst0;
            }
        }

        if(num_gltp > den_gltp) {
            /* beta bounded to 1 */
            gain_plt = MIN_GPLT;
        }
        else {
            gain_plt = den_gltp / (den_gltp + GAMMA_G * num_gltp);
        }

        /** filtering by H0(z) (harmonic filter) **/
        filt_plt(ptr_sig_in, ptr_y_up, ptr_sig_pst0, gain_plt);

    }

    return;
}

/*----------------------------------------------------------------------------
 *  search_del: computes best (shortest) integer LTP delay + fine search
 *----------------------------------------------------------------------------
 */
static void search_del(
 int t0,                /* input : pitch delay given by coder */
 FLOAT *ptr_sig_in,     /* input : input signal (with delay line) */
 int *ltpdel,           /* output: delay = *ltpdel - *phase / f_up */
 int *phase,            /* output: phase */
 FLOAT *num_gltp,       /* output: numerator of LTP gain */
 FLOAT *den_gltp,       /* output: denominator of LTP gain */
 FLOAT *y_up,           /*       : */
 int *off_yup           /*       : */
)
{

    /* pointers on tables of constants */
    FLOAT *ptr_h;

    /* Variables and local arrays */
    FLOAT tab_den0[F_UP_PST-1], tab_den1[F_UP_PST-1];
    FLOAT *ptr_den0, *ptr_den1;
    FLOAT *ptr_sig_past, *ptr_sig_past0;
    FLOAT *ptr1;

    int i, n, ioff, i_max;
    FLOAT ener, num, numsq, den0, den1;
    FLOAT den_int, num_int;
    FLOAT den_max, num_max, numsq_max;
    int phi_max;
    int lambda, phi;
    FLOAT temp0, temp1;
    FLOAT *ptr_y_up;

    /*****************************************/
    /* Compute current signal energy         */
    /*****************************************/

    ener = (F)0.;
    for(i=0; i<L_SUBFR; i++) {
        ener += ptr_sig_in[i] * ptr_sig_in[i];
    }
    if(ener < (F)0.1) {
        *num_gltp = (F)0.;
        *den_gltp = (F)1.;
        *ltpdel = 0;
        *phase = 0;
        return;
    }

    /*************************************/
    /* Selects best of 3 integer delays  */
    /* Maximum of 3 numerators around t0 */
    /* coder LTP delay                   */
    /*************************************/

    lambda = t0-1;

    ptr_sig_past = ptr_sig_in - lambda;

    num_int = (F)-1.0e30;

   /* initialization used only to suppress Microsoft Visual C++ warnings */
    i_max = 0;
    for(i=0; i<3; i++) {
        num=(F)0.;
        for(n=0; n<L_SUBFR; n++) {
            num += ptr_sig_in[n]* ptr_sig_past[n];
        }
        if(num > num_int) {
            i_max   = i;
            num_int = num;
        }
        ptr_sig_past--;
    }
    if(num_int <= (F)0.) {
        *num_gltp = (F)0.;
        *den_gltp = (F)1.;
        *ltpdel   = 0;
        *phase    = 0;
        return;
    }

    /* Calculates denominator for lambda_max */
    lambda += i_max;
    ptr_sig_past = ptr_sig_in - lambda;
    den_int=(F)0.;
    for(n=0; n<L_SUBFR; n++) {
        den_int += ptr_sig_past[n]* ptr_sig_past[n];
    }
    if(den_int < (F)0.1) {
        *num_gltp = (F)0.;
        *den_gltp = (F)1.;
        *ltpdel   = 0;
        *phase    = 0;
        return;
    }
    /***********************************/
    /* Select best phase around lambda */
    /***********************************/

    /* Compute y_up & denominators */
    /*******************************/
    ptr_y_up = y_up;
    den_max = den_int;
    ptr_den0 = tab_den0;
    ptr_den1 = tab_den1;
    ptr_h = tab_hup_s;
    ptr_sig_past0 = ptr_sig_in + LH_UP_S - 1 - lambda; /* points on lambda_max+1 */

    /* loop on phase  */
    for(phi=1; phi<F_UP_PST; phi++) {

        /* Computes criterion for (lambda_max+1) - phi/F_UP_PST     */
        /* and lambda_max - phi/F_UP_PST                            */
        ptr_sig_past = ptr_sig_past0;
        /* computes y_up[n] */
        for(n = 0; n<=L_SUBFR; n++) {
            ptr1 = ptr_sig_past++;
            temp0 = (F)0.;
            for(i=0; i<LH2_S; i++) {
                temp0 += ptr_h[i] * ptr1[-i];
            }
            ptr_y_up[n] = temp0;
        }

        /* recursive computation of den0 (lambda_max+1) and den1 (lambda_max) */

        /* common part to den0 and den1 */
        temp0 = (F)0.;
        for(n=1; n<L_SUBFR; n++) {
            temp0 += ptr_y_up[n] * ptr_y_up[n];
        }

        /* den0 */
        den0  = temp0 + ptr_y_up[0] * ptr_y_up[0];
        *ptr_den0++ = den0;

        /* den1 */
        den1 = temp0 + ptr_y_up[L_SUBFR] * ptr_y_up[L_SUBFR];
        *ptr_den1++ = den1;

        if(fabs(ptr_y_up[0])>fabs(ptr_y_up[L_SUBFR])) {
            if(den0 > den_max) {
                den_max = den0;
            }
        }
        else {
            if(den1 > den_max) {
                den_max = den1;
            }
        }
        ptr_y_up += L_SUBFRP1;
        ptr_h += LH2_S;
    }
    if(den_max < (F)0.1 ) {
        *num_gltp = (F)0.;
        *den_gltp = (F)1.;
        *ltpdel   = 0;
        *phase    = 0;
        return;
    }
    /* Computation of the numerators                */
    /* and selection of best num*num/den            */
    /* for non null phases                          */

    /* Initialize with null phase */
    num_max      = num_int;
    den_max      = den_int;
    numsq_max   =  num_max * num_max;
    phi_max      = 0;
    ioff         = 1;

    ptr_den0   = tab_den0;
    ptr_den1   = tab_den1;
    ptr_y_up     = y_up;

    /* if den_max = 0 : will be selected and declared unvoiced */
    /* if num!=0 & den=0 : will be selected and declared unvoiced */
    /* degenerated seldom cases, switch off LT is OK */

    /* Loop on phase */
    for(phi=1; phi<F_UP_PST; phi++) {


        /* computes num for lambda_max+1 - phi/F_UP_PST */
        num = (F)0.;
        for(n = 0; n<L_SUBFR; n++) {
            num += ptr_sig_in[n]  * ptr_y_up[n];
        }
        if(num < (F)0.) num = (F)0.;
        numsq = num * num;

        /* selection if num/sqrt(den0) max */
        den0 = *ptr_den0++;
        temp0 = numsq * den_max;
        temp1 = numsq_max * den0;
        if(temp0 > temp1) {
            num_max     = num;
            numsq_max   = numsq;
            den_max     = den0;
            ioff        = 0;
            phi_max     = phi;
        }

        /* computes num for lambda_max - phi/F_UP_PST */
        ptr_y_up++;
        num = (F)0.;
        for(n = 0; n<L_SUBFR; n++) {
            num += ptr_sig_in[n]  * ptr_y_up[n];
        }
        if(num < (F)0.) num = (F)0.;
        numsq = num * num;

        /* selection if num/sqrt(den1) max */
        den1 = *ptr_den1++;
        temp0 = numsq * den_max;
        temp1 = numsq_max * den1;
        if(temp0 > temp1) {
            num_max     = num;
            numsq_max   = numsq;
            den_max     = den1;
            ioff        = 1;
            phi_max     = phi;
        }
        ptr_y_up += L_SUBFR;
    }

    /***************************************************/
    /*** test if normalised crit0[iopt] > THRESCRIT  ***/
    /***************************************************/

    if((num_max == (F)0.) || (den_max <= (F)0.1)) {
        *num_gltp = (F)0.;
        *den_gltp = (F)1.;
        *ltpdel = 0;
        *phase = 0;
        return;
    }

    /* comparison num * num            */
    /* with ener * den x THRESCRIT      */
    temp1 = den_max * ener * THRESCRIT;
    if(numsq_max >= temp1) {
        *ltpdel   = lambda + 1 - ioff;
        *off_yup  = ioff;
        *phase    = phi_max;
        *num_gltp = num_max;
        *den_gltp = den_max;
    }
    else {
        *num_gltp = (F)0.;
        *den_gltp = (F)1.;
        *ltpdel   = 0;
        *phase    = 0;
    }
    return;
}

/*----------------------------------------------------------------------------
 *  filt_plt -  ltp  postfilter
 *----------------------------------------------------------------------------
 */
void filt_plt(
 FLOAT *s_in,       /* input : input signal with past*/
 FLOAT *s_ltp,      /* input : filtered signal with gain 1 */
 FLOAT *s_out,      /* output: output signal */
 FLOAT gain_plt     /* input : filter gain  */
)
{

    /* Local variables */
    int n;
    FLOAT temp;
    FLOAT gain_plt_1;

    gain_plt_1 = (F)1. - gain_plt;

    for(n=0;  n<L_SUBFR; n++) {
        /* s_out(n) = gain_plt x s_in(n) + gain_plt_1 x s_ltp(n)    */
        temp =  gain_plt   * s_in[n];
        temp += gain_plt_1 * s_ltp[n];
        s_out[n] = temp;
    }
    return;
}

/*----------------------------------------------------------------------------
 *  compute_ltp_l : compute delayed signal,
                    num & den of gain for fractional delay
 *                  with long interpolation filter
 *----------------------------------------------------------------------------
 */
static void compute_ltp_l(
 FLOAT *s_in,       /* input signal with past*/
 int ltpdel,      /* delay factor */
 int phase,       /* phase factor */
 FLOAT *y_up,       /* delayed signal */
 FLOAT *num,        /* numerator of LTP gain */
 FLOAT *den        /* denominator of LTP gain */
)
{

    /* Pointer on table of constants */
    FLOAT *ptr_h;

    /* Local variables */
    int n, i;
    FLOAT *ptr2, temp;

    /* Filtering with long filter */
    ptr_h = tab_hup_l + (phase-1) * LH2_L;
    ptr2 = s_in - ltpdel + LH_UP_L;

    /* Compute y_up */
    for(n = 0; n<L_SUBFR; n++) {
        temp = (F)0.;
        for(i=0; i<LH2_L; i++) {
            temp += ptr_h[i] * *ptr2--;
        }
        y_up[n] = temp;
        ptr2 += LH2_L_P1;
    }

    *num = (F)0.;
    /* Compute num */
    for(n = 0; n<L_SUBFR; n++) {
        *num += y_up[n]* s_in[n];
    }
    if(*num < (F)0.0) *num = (F)0.0;

    *den = (F)0.;
    /* Compute den */
    for(n = 0; n<L_SUBFR; n++) {
        *den += y_up[n]* y_up[n];
    }

    return;
}
/*----------------------------------------------------------------------------
 *  select_ltp : selects best of (gain1, gain2)
 *  with gain1 = num1 / den1
 *  and  gain2 = num2 / den2
 *----------------------------------------------------------------------------
 */
static int select_ltp(  /* output : 1 = 1st gain, 2 = 2nd gain */
 FLOAT num1,       /* input : numerator of gain1 */
 FLOAT den1,       /* input : denominator of gain1 */
 FLOAT num2,       /* input : numerator of gain2 */
 FLOAT den2        /* input : denominator of gain2 */
)
{
    if(den2 == (F)0.) {
        return(1);
    }
    if(num2 * num2 * den1> num1 * num1 * den2) {
        return(2);
    }
    else {
        return(1);
    }
}



/*----------------------------------------------------------------------------
 *   calc_st_filt -  computes impulse response of A(gamma2) / A(gamma1)
 *   controls gain : computation of energy impulse response as
 *                    SUMn  (abs (h[n])) and computes parcor0
 *----------------------------------------------------------------------------
 */
static void calc_st_filt(
 FLOAT *apond2,     /* input : coefficients of numerator */
 FLOAT *apond1,     /* input : coefficients of denominator */
 FLOAT *parcor0,    /* output: 1st parcor calcul. on composed filter */
 FLOAT *sig_ltp_ptr    /* in/out: input of 1/A(gamma1) : scaled by 1/g0 */
)
{
    FLOAT h[LONG_H_ST];
    FLOAT g0, temp;
    int i;

    /* computes impulse response of  apond1 / apond2 */
    syn_filt(apond1, apond2, h, LONG_H_ST, mem_zero, 0);

    /* computes 1st parcor */
    calc_rc0_h(h, parcor0);

    /* computes gain g0 */
    g0 = (F)0.;
    for(i=0; i<LONG_H_ST; i++) {
        g0 += (FLOAT)fabs(h[i]);
    }

    /* Scale signal input of 1/A(gamma1) */
    if(g0 > (F)1.) {
        temp = (F)1./g0;
        for(i=0; i<L_SUBFR; i++) {
            sig_ltp_ptr[i] = sig_ltp_ptr[i] * temp;
        }
    }

    return;
}

/*----------------------------------------------------------------------------
 * calc_rc0_h - computes 1st parcor from composed filter impulse response
 *----------------------------------------------------------------------------
 */
static void calc_rc0_h(
 FLOAT *h,      /* input : impulse response of composed filter */
 FLOAT *rc0     /* output: 1st parcor */
)
{
    FLOAT acf0, acf1;
    FLOAT temp, temp2;
    FLOAT *ptrs;
    int i;

    /* computation of the autocorrelation function acf */
    temp = (F)0.;
    for(i=0;i<LONG_H_ST;i++){
        temp += h[i] * h[i];
    }
    acf0 = temp;

    temp = (F)0.;
    ptrs = h;
    for(i=0;i<LONG_H_ST-1;i++){
        temp2 = *ptrs++;
        temp += temp2 * (*ptrs);
    }
    acf1 = temp;

    /* Initialisation of the calculation */
    if( acf0 == (F)0.) {
        *rc0 = (F)0.;
        return;
    }

    /* Compute 1st parcor */
    /**********************/
    if(acf0 < fabs(acf1) ) {
        *rc0 = (F)0.0;
        return;
    }
    *rc0 = - acf1 / acf0;

    return;
}

/*----------------------------------------------------------------------------
 * filt_mu - tilt filtering with : (1 + mu z-1) * (1/1-|mu|)
 *   computes y[n] = (1/1-|mu|) (x[n]+mu*x[n-1])
 *----------------------------------------------------------------------------
 */
static void filt_mu(
 FLOAT *sig_in,     /* input : input signal (beginning at sample -1) */
 FLOAT *sig_out,    /* output: output signal */
 FLOAT parcor0      /* input : parcor0 (mu = parcor0 * gamma3) */
)
{
    int n;
    FLOAT mu, ga, temp;
    FLOAT *ptrs;

    if(parcor0 > (F)0.) {
        mu = parcor0 * GAMMA3_PLUS;
    }
    else {
        mu = parcor0 * GAMMA3_MINUS;
    }
    ga = (F)1. / ((F)1. - (FLOAT)fabs(mu));

    ptrs = sig_in;      /* points on sig_in(-1) */
    for(n=0; n<L_SUBFR; n++) {
        temp = mu * (*ptrs++);
        temp += (*ptrs);
        sig_out[n] = ga * temp;
    }
    return;
}

/*----------------------------------------------------------------------------
 *   scale_st  - control of the subframe gain
 *   gain[n] = AGC_FAC * gain[n-1] + (1 - AGC_FAC) g_in/g_out
 *----------------------------------------------------------------------------
 */
void scale_st(
 FLOAT *sig_in,     /* input : postfilter input signal */
 FLOAT *sig_out,    /* in/out: postfilter output signal */
 FLOAT *gain_prec   /* in/out: last value of gain for subframe */
)
{
    int i;
    FLOAT gain_in, gain_out;
    FLOAT g0, gain;

    /* compute input gain */
    gain_in = (F)0.;
    for(i=0; i<L_SUBFR; i++) {
        gain_in += (FLOAT)fabs(sig_in[i]);
    }
    if(gain_in == (F)0.) {
        g0 = (F)0.;
    }
    else {

        /* Compute output gain */
        gain_out = (F)0.;
        for(i=0; i<L_SUBFR; i++) {
            gain_out += (FLOAT)fabs(sig_out[i]);
        }
        if(gain_out == (F)0.) {
            *gain_prec = (F)0.;
            return;
        }

        g0 = gain_in/ gain_out;
        g0 *= AGC_FAC1;
    }

    /* compute gain(n) = AGC_FAC gain(n-1) + (1-AGC_FAC)gain_in/gain_out */
    /* sig_out(n) = gain(n) sig_out(n)                                   */
    gain = *gain_prec;
    for(i=0; i<L_SUBFR; i++) {
        gain *= AGC_FAC;
        gain += g0;
        sig_out[i] *= gain;
    }
    *gain_prec = gain;
    return;
}
