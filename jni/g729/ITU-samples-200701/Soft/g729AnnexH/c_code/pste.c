/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.3    Last modified: September 1999 */
/* from pst.c G.729 Version 3.3            */

/************************************************************************/
/*      Post - filtering : short term + long term                       */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>

/**** Prototypes                                                        */
#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "ld8k.h"
#include "ld8e.h"


/* Prototypes for local functions */
/**********************************/

static void pst_ltpe(Word16 t0, Word16 *ptr_sig_in,
    Word16 *ptr_sig_pst0, Word16 *vo, Word16 gamma_harm);
static void search_del(Word16 t0, Word16 *ptr_sig_in, Word16 *ltpdel,
    Word16 *phase, Word16 *num_gltp, Word16 *den_gltp, Word16 *sh_num_gltp,
    Word16 *sh_den_gltp, Word16 *y_up, Word16 *off_yup);
static void filt_plt( Word16 *s_in, Word16 *s_ltp,
    Word16 *s_out, Word16 gain_plt);
static void compute_ltp_l(Word16 *s_in, Word16 ltpdel, Word16 phase,
    Word16 *y_up, Word16 *num, Word16 *den, Word16 *sh_num, Word16 *sh_den);
static Word16 select_ltp(Word16 num1, Word16 den1, Word16 sh_num1,
    Word16 sh_den1, Word16 num2, Word16 den2, Word16 sh_num2, Word16 sh_den2);
static void calc_st_filte(Word16 *apond2, Word16 *apond1, Word16 *parcor0,
    Word16 *signal_ltp0_ptr, Word16  long_h_st, Word16 m_pst);
static void filt_mu(Word16 *sig_in, Word16 *sig_out, Word16 parcor0);
static void calc_rc0_he(Word16 *h, Word16 *rc0, Word16  long_h_st);
static void scale_st(Word16 *sig_in, Word16 *sig_out, Word16 *gain_prec);

/* Static arrays and variables */
/*******************************/

/* Arrays */
static Word16 apond2[LONG_H_ST_E];    /* s.t. numerator coeff.        */
static Word16 mem_stp[M_BWD];           /* s.t. postfilter memory       */
static Word16 mem_zero[M_BWD];          /* null memory to compute h_st  */
static Word16 res2[SIZ_RES2];       /* A(gamma2) residual           */

/* pointers */
Word16 *res2_ptr;
Word16 *ptr_mem_stp;

/* Variables */
Word16 gain_prec;

/************************************************************************/
/****   Short term postfilter :                                     *****/
/*      Hst(z) = Hst0(z) Hst1(z)                                        */
/*      Hst0(z) = 1/g0 A(gamma2)(z) / A(gamma1)(z)                      */
/*      if {hi} = i.r. filter A(gamma2)/A(gamma1) (truncated)           */
/*      g0 = SUM(|hi|) if > 1                                           */
/*      g0 = 1. else                                                    */
/*      Hst1(z) = 1/(1 - |mu|) (1 + mu z-1)                             */
/*      with mu = k1 * gamma3                                           */
/*      k1 = 1st parcor calculated on {hi}                              */
/*      gamma3 = gamma3_minus if k1<0, gamma3_plus if k1>0              */
/****   Long term postfilter :                                      *****/
/*      harmonic postfilter :   H0(z) = gl * (1 + b * z-p)              */
/*      b = gamma_g * gain_ltp                                          */
/*      gl = 1 / 1 + b                                                  */
/*      computation of delay p on A(gamma2)(z) s(z)                     */
/*      sub optimal search                                              */
/*      1. search around 1st subframe delay (3 integer values)          */
/*      2. search around best integer with fract. delays (1/8)          */
/************************************************************************/

/*----------------------------------------------------------------------------
 * Init_Post_Filter -  Initialize postfilter functions
 *----------------------------------------------------------------------------
 */
void Init_Post_Filter(
    void
)
{
    int i;

    /* Initialize arrays and pointers */

    /* res2 =  A(gamma2) residual */
    for(i=0; i<MEM_RES2; i++) res2[i] = 0;
    res2_ptr = res2 + MEM_RES2;

    /* 1/A(gamma1) memory */
    for(i=0; i<M_BWD; i++) mem_stp[i] = 0;
    ptr_mem_stp = mem_stp + M_BWD - 1;

    /* fill apond2[M+1->long_h_st-1] with zeroes */
    for(i=M_BWDP1; i<LONG_H_ST_E; i++) apond2[i] = 0;

    /* null memory to compute i.r. of A(gamma2)/A(gamma1) */
    for(i=0; i<M_BWD; i++) mem_zero[i] = 0;

    /* for gain adjustment */
    gain_prec = 16384;

    return;
}

/*----------------------------------------------------------------------------
 * Post - adaptive postfilter main function
 *----------------------------------------------------------------------------
 */
void Poste(
 Word16 t0,             /* input : pitch delay given by coder */
 Word16 *signal_ptr,    /* input : input signal (pointer to current subframe */
 Word16 *coeff,         /* input : LPC coefficients for current subframe */
 Word16 *sig_out,       /* output: postfiltered output */
 Word16 *vo,            /* output: voicing decision 0 = uv,  > 0 delay */
 Word16 gamma1,         /* input: short term postfilt. den. weighting factor*/
 Word16 gamma2,         /* input: short term postfilt. num. weighting factor*/
 Word16 gamma_harm,     /* input: long term postfilter weighting factor*/
  Word16  long_h_st,    /* input: impulse response length*/
  Word16 m_pst          /* input:  LPC order */
)
{

    /* Local variables and arrays */
    Word16 apond1[M_BWDP1];             /* s.t. denominator coeff.      */
    Word16 sig_ltp[L_SUBFRP1];      /* H0 output signal             */
    Word16 *sig_ltp_ptr;
    Word16 parcor0;

    /* Compute weighted LPC coefficients */
    Weight_Az(coeff, gamma1, m_pst, apond1);
    Weight_Az(coeff, gamma2, m_pst, apond2);
    Set_zero(&apond2[m_pst+1], (Word16)(M_BWD-m_pst));

    /* Compute A(gamma2) residual */
    Residue(m_pst, apond2, signal_ptr, res2_ptr, L_SUBFR);

    /* Harmonic filtering */
    sig_ltp_ptr = sig_ltp + 1;
    pst_ltpe(t0, res2_ptr, sig_ltp_ptr, vo, gamma_harm);

    /* Save last output of 1/A(gamma1)  */
    /* (from preceding subframe)        */
    sig_ltp[0] = *ptr_mem_stp;

    /* Controls short term pst filter gain and compute parcor0   */
    calc_st_filte(apond2, apond1, &parcor0, sig_ltp_ptr, long_h_st, m_pst);

    /* 1/A(gamma1) filtering, mem_stp is updated */
    Syn_filte(m_pst, apond1, sig_ltp_ptr, sig_ltp_ptr, L_SUBFR,
            &mem_stp[M_BWD-m_pst], 0);
    Copy(&sig_ltp_ptr[L_SUBFR-M_BWD], mem_stp, M_BWD);

    /* Tilt filtering */
    filt_mu(sig_ltp, sig_out, parcor0);

    /* Gain control */
    scale_st(signal_ptr, sig_out, &gain_prec);

    /**** Update for next subframe */
    Copy(&res2[L_SUBFR], &res2[0], MEM_RES2);

    return;
}

/*----------------------------------------------------------------------------
 *  pst_ltp - harmonic postfilter
 *----------------------------------------------------------------------------
 */
static void pst_ltpe(
 Word16 t0,             /* input : pitch delay given by coder */
 Word16 *ptr_sig_in,    /* input : postfilter input filter (residu2) */
 Word16 *ptr_sig_pst0,  /* output: harmonic postfilter output */
 Word16 *vo,             /* output: voicing decision 0 = uv,  > 0 delay */
 Word16 gamma_harm      /* input: harmonic postfilter coefficient */
)
{

/**** Declare variables                                 */
    int i;
    Word16 temp;
    Word16 ltpdel, phase;
    Word16 num_gltp, den_gltp;
    Word16 num2_gltp, den2_gltp;
    Word16 sh_num, sh_den;
    Word16 sh_num2, sh_den2;
    Word16 gain_plt;
    Word16 y_up[SIZ_Y_UP];
    Word16 *ptr_y_up;
    Word16 off_yup;
    Word16 *ptr_sig;
    Word16 sig_cadr[SIZ_RES2], *ptr_sig_cadr;
    Word16 nb_sh_sig;
    Word32 L_temp;

    /* input signal justified on 13 bits */
    ptr_sig = ptr_sig_in - MEM_RES2;

    temp = 0;
    for(i=0; i<SIZ_RES2; i++) {
        temp |= abs_s(ptr_sig[i]);
    }
    nb_sh_sig = sub(3, norm_s(temp));

    for(i=0; i<SIZ_RES2; i++) {   /* nb_sh_sig may be >0, <0 or =0 */
        sig_cadr[i] = shr( ptr_sig[i], nb_sh_sig);
    }

    ptr_sig_cadr = sig_cadr + MEM_RES2;

    /* Sub optimal delay search */
    search_del(t0, ptr_sig_cadr, &ltpdel, &phase, &num_gltp, &den_gltp,
                            &sh_num, &sh_den, y_up, &off_yup);
    *vo = ltpdel;

    if(num_gltp == 0)   {
        Copy(ptr_sig_in, ptr_sig_pst0, L_SUBFR);
    }

    else {
        if(phase == 0) {
            ptr_y_up = ptr_sig_in - ltpdel;
        }

        else {

            /* Filtering with long filter */
            compute_ltp_l(ptr_sig_cadr, ltpdel, phase, ptr_sig_pst0,
                &num2_gltp, &den2_gltp, &sh_num2, &sh_den2);

            if(select_ltp(num_gltp, den_gltp, sh_num, sh_den,
                num2_gltp, den2_gltp, sh_num2, sh_den2) == 1) {

                /* select short filter */
                temp   = sub(phase, 1);
                L_temp = L_mult(temp, L_SUBFRP1);
                L_temp = L_shr(L_temp, 1);
                temp   = extract_l(L_temp);
                temp   = add(temp, off_yup);
                /* ptr_y_up = y_up + (phase-1) * L_SUBFRP1 + off_yup */
                ptr_y_up = y_up + temp;
            }
            else {
                /* select long filter */
                num_gltp = num2_gltp;
                den_gltp = den2_gltp;
                sh_num   = sh_num2;
                sh_den   = sh_den2;
                ptr_y_up = ptr_sig_pst0;
            }

            /* rescale y_up */
            for(i=0; i<L_SUBFR; i++) {  /* nb_sh_sig may be >0, <0 or =0 */
                ptr_y_up[i] = shl(ptr_y_up[i], nb_sh_sig);
            }
        }

        temp = sub(sh_num,sh_den);
        if(temp >= 0) den_gltp = shr(den_gltp, temp);
        else {
            num_gltp = shl(num_gltp, temp); /*  >> (-temp) */
        }

        if(sub(num_gltp ,den_gltp)>=0) {
            /* beta bounded to 1 */
            if (gamma_harm == GAMMA_HARM)   gain_plt = MIN_GPLT;
            else {
                num_gltp = mult_r(den_gltp, gamma_harm);
                num_gltp = shr(num_gltp, 1);
                den_gltp = shr(den_gltp, 1);
                temp     = add(den_gltp, num_gltp);
                gain_plt = div_s(den_gltp, temp);
            }

        }
        else {
            /*gain_plt = den_gltp x 2**15 / (den_gltp + 0.5 num_gltp)  */
            /* shift 1 bit to avoid overflows in add                    */
            if (gamma_harm == GAMMA_HARM) {
                num_gltp = shr(num_gltp, 2);
            }
            else {
                num_gltp = mult_r(num_gltp, gamma_harm);
                num_gltp = shr(num_gltp, 1);
            }
            den_gltp = shr(den_gltp, 1);
            temp     = add(den_gltp, num_gltp);
            gain_plt = div_s(den_gltp, temp);

        }

        /** filtering by H0(z) = harmonic filter **/
        filt_plt(ptr_sig_in, ptr_y_up, ptr_sig_pst0, gain_plt);

    }

    return;
}

/*----------------------------------------------------------------------------
 *  search_del: computes best (shortest) integer LTP delay + fine search
 *----------------------------------------------------------------------------
 */
static void search_del(
 Word16 t0,                /* input : pitch delay given by coder */
 Word16 *ptr_sig_in,       /* input : input signal (with delay line) */
 Word16 *ltpdel,           /* output: delay = *ltpdel - *phase / f_up */
 Word16 *phase,            /* output: phase */
 Word16 *num_gltp,         /* output: 16 bits numerator of LTP gain */
 Word16 *den_gltp,         /* output: 16 bits denominator of LTP gain */
 Word16 *sh_num_gltp,      /* output: justification for num_gltp */
 Word16 *sh_den_gltp,      /* output: justification for den_gltp */
 Word16 *y_up,             /* output: LT delayed signal if fract. delay */
 Word16 *off_yup           /* output: offset in y_up */
)
{

    /* Tables of constants */
    extern Word16 tab_hup_s[SIZ_TAB_HUP_S];

    Word32 L_den0[F_UP_PST-1];
    Word32 L_den1[F_UP_PST-1];
    Word32 *ptr_L_den0, *ptr_L_den1;

    int i, n;
    Word16 *ptr_h;
    Word16 *ptr_sig_past, *ptr_sig_past0;
    Word16 *ptr1, *ptr_y_up;
    Word16 num, den0, den1;
    Word16 den_max, num_max;
    Word32 L_num_int, L_den_int, L_den_max;
    Word16 hi_numsq, hi_numsq_max;
    Word16 lo_numsq, lo_numsq_max;
    Word16 ener;
    Word16 sh_num, sh_den, sh_ener;
    Word16 i_max, lambda, phi, phi_max, ioff;
    Word16 temp;
    Word32 L_temp0, L_temp1;
    Word32 L_acc;
    Word32 L_temp;


    /* Computes energy of current signal */
    /*************************************/

    L_acc = 0L;
    for(i=0; i<L_SUBFR; i++) {
        L_acc = L_mac( L_acc, ptr_sig_in[i] , ptr_sig_in[i]);
    }
    if(L_acc == 0) {
        *num_gltp = 0;
        *den_gltp = 1;
        *ltpdel = 0;
        *phase = 0;
        return;
    }
    sh_ener = sub(16, norm_l(L_acc));
    /* save energy for final decision */
    if(sh_ener > 0) {
        ener = extract_l(L_shr(L_acc, sh_ener));
    }
    else {
        sh_ener = 0;
        ener = extract_l(L_acc);
    }

    /*************************************/
    /* Selects best of 3 integer delays  */
    /* Maximum of 3 numerators around t0 */
    /*************************************/
    lambda = sub(t0,1);
    ptr_sig_past = ptr_sig_in - lambda;

    L_num_int = -1L;

   /* initialization used only to suppress Microsoft Visual C++ warnings */
    i_max = (Word16)0;

    for(i=0; i<3; i++) {
        L_acc = 0L;
        for(n=0; n<L_SUBFR; n++) {
            L_acc = L_mac( L_acc, ptr_sig_in[n] , ptr_sig_past[n]);
        }
        if(L_acc < 0) {
            L_acc = 0L;
        }
        L_temp =L_sub(L_acc ,L_num_int);
        if(L_temp > 0L) {
            L_num_int = L_acc;
            i_max = (Word16)i;
        }
        ptr_sig_past--;
    }

    if(L_num_int == 0) {
        *num_gltp = 0;
        *den_gltp = 1;
        *ltpdel = 0;
        *phase = 0;
        return;
    }

    /* Compute den for i_max */
    lambda = add(lambda, (Word16)i_max);
    ptr_sig_past = ptr_sig_in - lambda;
    L_acc = 0L;
    for(i=0; i<L_SUBFR; i++) {
        temp = *ptr_sig_past++;
        L_acc = L_mac( L_acc, temp, temp);
    }
    if(L_acc == 0L) {
        *num_gltp = 0;
        *den_gltp = 1;
        *ltpdel = 0;
        *phase = 0;
        return;
    }
    L_den_int = L_acc;


    /***********************************/
    /* Select best phase around lambda */
    /***********************************/

    /* Compute y_up & denominators */
    /*******************************/

    ptr_y_up = y_up;
    L_den_max = L_den_int;
    ptr_L_den0 = L_den0;
    ptr_L_den1 = L_den1;
    ptr_h = tab_hup_s;
    temp = sub(lambda, LH_UP_SM1);
    ptr_sig_past0 = ptr_sig_in - temp;

    /* Loop on phase */
    for(phi=1; phi<F_UP_PST; phi++) {

        /* Compute y_up for lambda+1 - phi/F_UP_PST */
        /* and lambda - phi/F_UP_PST                */

        ptr_sig_past = ptr_sig_past0;
        for(n = 0; n<=L_SUBFR; n++) {
            ptr1 = ptr_sig_past++;
            L_acc = 0L;
            for(i=0; i<LH2_S; i++) {
                L_acc = L_mac(L_acc, ptr_h[i], ptr1[-i]);
            }
            ptr_y_up[n] = round(L_acc);
        }

        /* compute den0 (lambda+1) and den1 (lambda) */

        /* part common to den0 and den1 */
        L_acc = 0L;
        for(n=1; n<L_SUBFR; n++) {
            L_acc = L_mac(L_acc, ptr_y_up[n] ,ptr_y_up[n]);
        }
        L_temp0 = L_acc;        /* saved for den1 */

        /* den0 */
        L_acc = L_mac(L_acc, ptr_y_up[0] ,ptr_y_up[0]);
        *ptr_L_den0 = L_acc;

        /* den1 */
        L_acc = L_mac(L_temp0, ptr_y_up[L_SUBFR] ,ptr_y_up[L_SUBFR]);
        *ptr_L_den1 = L_acc;

        if(sub(abs_s(ptr_y_up[0]),abs_s(ptr_y_up[L_SUBFR])) >0) {
            L_temp =L_sub(*ptr_L_den0 ,L_den_max );
            if(L_temp> 0L) {
                L_den_max = *ptr_L_den0;
            }
        }
        else {
            L_temp =L_sub(*ptr_L_den1 ,L_den_max );
            if(L_temp> 0L) {
                L_den_max = *ptr_L_den1;
            }
        }
        ptr_L_den0++;
        ptr_L_den1++;
        ptr_y_up += L_SUBFRP1;
        ptr_h += LH2_S;
    }

    if(L_den_max == 0) {
        *num_gltp = 0;
        *den_gltp = 1;
        *ltpdel = 0;
        *phase = 0;
        return;
    }

    sh_den = sub(16, norm_l(L_den_max));
    /* if sh_den <= 0 :  dynamic between current frame */
    /* and delay line too high                         */
    if(sh_den <= 0) {
        *num_gltp = 0;
        *den_gltp = 1;
        *ltpdel = 0;
        *phase = 0;
        return;
    }

    /* search sh_num to justify correlations   */
    /* sh_num = Max(sh_den, sh_ener)           */
    sh_num = (sub( sh_den , sh_ener)>=0) ? sh_den : sh_ener;

    /* Computation of the numerators                */
    /* and selection of best num*num/den            */
    /* for non null phases                          */

    /* Initialize with null phase */
    L_acc        = L_shr(L_den_int, sh_den);   /* sh_den > 0 */
    den_max      = extract_l(L_acc);
    L_acc        = L_shr(L_num_int, sh_num);   /* sh_num > 0 */
    num_max      = extract_l(L_acc);
    L_acc        = L_mult(num_max, num_max);
    L_Extract(L_acc, &hi_numsq_max, &lo_numsq_max);
    phi_max      = 0;
    ioff         = 1;

    ptr_L_den0   = L_den0;
    ptr_L_den1   = L_den1;
    ptr_y_up     = y_up;

    /* if den_max = 0 : will be selected and declared unvoiced */
    /* if num!=0 & den=0 : will be selected and declared unvoiced */
    /* degenerated seldom cases, switch off LT is OK */

    /* Loop on phase */
    for(phi=1; phi<F_UP_PST; phi++) {

        /* compute num for lambda+1 - phi/F_UP_PST */
        L_acc = 0L;
        for(n = 0; n<L_SUBFR; n++) {
            L_acc = L_mac(L_acc, ptr_sig_in[n] ,ptr_y_up[n]);
        }
        L_acc = L_shr(L_acc, sh_num);       /* sh_num > 0 */
        if(L_acc < 0L) {
            num = 0;
        }
        else {
            num  = extract_l(L_acc);
        }

        /* selection if num**2/den0 max */
        L_acc    = L_mult(num, num);
        L_Extract(L_acc, &hi_numsq, &lo_numsq);
        L_temp0  = Mpy_32_16(hi_numsq, lo_numsq, den_max);
        L_acc    = *ptr_L_den0++;
        L_acc    = L_shr(L_acc, sh_den);        /* sh_den > 0 */
        den0     = extract_l(L_acc);
        L_temp1  = Mpy_32_16(hi_numsq_max, lo_numsq_max, den0);
        L_temp = L_sub(L_temp0, L_temp1);
        if(L_temp>0L) {
            num_max      = num;
            hi_numsq_max = hi_numsq;
            lo_numsq_max = lo_numsq;
            den_max      = den0;
            ioff         = 0;
            phi_max      = phi;
        }

        /* compute num for lambda - phi/F_UP_PST */
        ptr_y_up++;
        L_acc = 0L;
        for(n = 0; n<L_SUBFR; n++) {
            L_acc = L_mac(L_acc, ptr_sig_in[n] ,ptr_y_up[n]);
        }
        L_acc = L_shr(L_acc, sh_num);   /* sh_num > 0 */
        if(L_acc < 0L) {
            num = 0;
        }
        else {
            num  = extract_l(L_acc);
        }

        /* selection if num**2/den1 max */
        L_acc    = L_mult(num, num);
        L_Extract(L_acc, &hi_numsq, &lo_numsq);
        L_temp0  = Mpy_32_16(hi_numsq, lo_numsq, den_max);
        L_acc    = *ptr_L_den1++;
        L_acc    = L_shr(L_acc, sh_den);        /* sh_den > 0 */
        den1     = extract_l(L_acc);
        L_temp1  = Mpy_32_16(hi_numsq_max, lo_numsq_max, den1);
        L_temp = L_sub(L_temp0,L_temp1);
        if(L_temp> 0L) {
            num_max      = num;
            hi_numsq_max = hi_numsq;
            lo_numsq_max = lo_numsq;
            den_max     = den1;
            ioff        = 1;
            phi_max     = phi;
        }

        ptr_y_up += L_SUBFR;
    }

    /***************************************************/
    /*** test if normalized crit0[iopt] > THRESHCRIT ***/
    /***************************************************/
    if((num_max == 0) || (sub(den_max,1) <= 0)) {
        *num_gltp = 0;
        *den_gltp = 1;
        *ltpdel = 0;
        *phase = 0;
        return;
    }

    /* compare num**2               */
    /* to ener * den * 0.5          */
    /* (THRESHCRIT = 0.5)           */
    L_temp1 = L_mult(den_max, ener);
    L_temp0 = L_Comp(hi_numsq_max, lo_numsq_max);

    /* temp = 2 * sh_num - sh_den - sh_ener + 1 */
    /* 16 bits with no overflows  */
    temp = shl(sh_num,1);
    temp = sub(temp, sh_den);
    temp = sub(temp, sh_ener);
    temp = add(temp, 1);
    if(temp < 0) {
        temp    = negate(temp);             /* no overflow */
        L_temp0 = L_shr(L_temp0, temp);
    }
    else {
        if(temp > 0) L_temp1 = L_shr(L_temp1, temp);
    }
    L_temp = L_sub(L_temp0 ,L_temp1);
    if(L_temp >= 0L) {
        temp         = add(lambda, 1);
        *ltpdel      = sub(temp, ioff);
        *off_yup     = ioff;
        *phase       = phi_max;
        *num_gltp    = num_max;
        *den_gltp    = den_max;
        *sh_den_gltp = sh_den;
        *sh_num_gltp = sh_num;
    }
    else {
        *num_gltp = 0;
        *den_gltp = 1;
        *ltpdel = 0;
        *phase = 0;
    }
    return;

}

/*----------------------------------------------------------------------------
 *  filt_plt -  ltp  postfilter
 *----------------------------------------------------------------------------
 */
static void filt_plt(
 Word16 *s_in,       /* input : input signal with past*/
 Word16 *s_ltp,      /* input : filtered signal with gain 1 */
 Word16 *s_out,      /* output: output signal */
 Word16 gain_plt     /* input : filter gain  */
)
{

    /* Local variables */
    int n;
    Word32 L_acc;
    Word16 gain_plt_1;

    gain_plt_1 = sub(32767, gain_plt);
    gain_plt_1 = add(gain_plt_1, 1);        /* 2**15 (1 - g) */

    for(n=0;  n<L_SUBFR; n++) {
        /* s_out(n) = gain_plt x s_in(n) + gain_plt_1 x s_ltp(n)        */
        L_acc    = L_mult(gain_plt, s_in[n]);
        L_acc    = L_mac(L_acc, gain_plt_1, s_ltp[n]);  /* no overflow      */
        s_out[n] = round(L_acc);
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
 Word16 *s_in,       /* input signal with past*/
 Word16 ltpdel,      /* delay factor */
 Word16 phase,       /* phase factor */
 Word16 *y_up,       /* delayed signal */
 Word16 *num,        /* numerator of LTP gain */
 Word16 *den,        /* denominator of LTP gain */
 Word16 *sh_num,     /* justification factor of num */
 Word16 *sh_den      /* justification factor of den */
)
{

/**** Table of constants declared externally            */
    extern Word16 tab_hup_l[SIZ_TAB_HUP_L];

    /* Pointer on table of constants */
    Word16 *ptr_h;

    /* Local variables */
    int n, i;
    Word16 *ptr2;
    Word16 temp;
    Word32 L_acc;

    temp  = sub(phase, 1);
    temp  = shl(temp, L2_LH2_L);
    ptr_h = tab_hup_l + temp;   /* tab_hup_l + LH2_L * (phase-1) */

    temp  = sub(LH_UP_L, ltpdel);
    ptr2  = s_in + temp; ;

    /* Compute y_up */
    for(n = 0; n<L_SUBFR; n++) {
        L_acc = 0L;
        for(i=0; i<LH2_L; i++) {
            L_acc = L_mac(L_acc, ptr_h[i], (*ptr2--));
        }
        y_up[n] = round(L_acc);
        ptr2 += LH2_L_P1;
    }


    /* Compute num */
    L_acc = 0L;
    for(n=0; n<L_SUBFR; n++)    {
        L_acc = L_mac(L_acc, y_up[n], s_in[n]);
    }
    if(L_acc < 0L) {
        *num = 0;
        *sh_num = 0;
    }
    else {
        temp = sub(16, norm_l(L_acc));
        if(temp < 0) {
            temp = 0;
        }
        L_acc = L_shr(L_acc, temp);   /* with temp >= 0 */
        *num = extract_l(L_acc);
        *sh_num = temp;
    }

    /* Compute den */
    L_acc = 0L;
    for(n=0; n<L_SUBFR; n++)    {
        L_acc = L_mac(L_acc, y_up[n], y_up[n]);
    }
    temp = sub(16, norm_l(L_acc));
    if(temp < 0) {
        temp = 0;
    }
    L_acc = L_shr(L_acc, temp);     /* with temp >= 0 */
    *den = extract_l(L_acc);
    *sh_den = temp;

    return;
}

/*----------------------------------------------------------------------------
 *  select_ltp : selects best of (gain1, gain2)
 *  with gain1 = num1 * 2** sh_num1 / den1 * 2** sh_den1
 *  and  gain2 = num2 * 2** sh_num2 / den2 * 2** sh_den2
 *----------------------------------------------------------------------------
 */
static Word16 select_ltp(  /* output : 1 = 1st gain, 2 = 2nd gain */
 Word16 num1,       /* input : numerator of gain1 */
 Word16 den1,       /* input : denominator of gain1 */
 Word16 sh_num1,    /* input : just. factor for num1 */
 Word16 sh_den1,    /* input : just. factor for den1 */
 Word16 num2,       /* input : numerator of gain2 */
 Word16 den2,       /* input : denominator of gain2 */
 Word16 sh_num2,    /* input : just. factor for num2 */
 Word16 sh_den2)    /* input : just. factor for den2 */
{
    Word32 L_temp1, L_temp2;
    Word16 temp1, temp2;
    Word16 hi, lo;
    Word32 L_temp;

    if(den2 == 0) {

        return(1);
    }

    /* compares criteria = num**2/den */
    L_temp1 = L_mult(num1, num1);
    L_Extract(L_temp1, &hi, &lo);
    L_temp1 = Mpy_32_16(hi, lo, den2);

    L_temp2 = L_mult(num2, num2);
    L_Extract(L_temp2, &hi, &lo);
    L_temp2 = Mpy_32_16(hi, lo, den1);

    /* temp1 = sh_den2 + 2 * sh_num1 */
    temp1 = shl(sh_num1, 1);
    temp1 = add(temp1, sh_den2);
    /* temp2 = sh_den1 + 2 * sh_num2; */
    temp2 = shl(sh_num2, 1);
    temp2 = add(temp2, sh_den1);

    if(sub(temp2 ,temp1)>0) {
        temp2 = sub(temp2, temp1);
        L_temp1 = L_shr(L_temp1, temp2);    /* temp2 > 0 */
    }
    else {
        if(sub(temp1 ,temp2) >0){
            temp1 = sub(temp1, temp2);
            L_temp2 = L_shr(L_temp2, temp1);    /* temp1 > 0 */
        }
    }

    L_temp = L_sub(L_temp2,L_temp1);
    if(L_temp>0L) {

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
static void calc_st_filte(
 Word16 *apond2,     /* input : coefficients of numerator */
 Word16 *apond1,     /* input : coefficients of denominator */
 Word16 *parcor0,    /* output: 1st parcor calcul. on composed filter */
 Word16 *sig_ltp_ptr,    /* in/out: input of 1/A(gamma1) : scaled by 1/g0 */
 Word16  long_h_st,     /* input : impulse response length                   */
 Word16 m_pst               /* input : LPC order    */
)
{
    Word16 h[LONG_H_ST_E];
    Word32 L_temp, L_g0;
    Word16 g0, temp;
    int i;

    /* compute i.r. of composed filter apond2 / apond1 */
    Syn_filte(m_pst, apond1, apond2, h, long_h_st, mem_zero, 0);

    /* compute 1st parcor */
    calc_rc0_he(h, parcor0, long_h_st);

    /* compute g0 */
    L_g0 = 0L;
    for(i=0; i<long_h_st; i++) {
        L_temp = L_deposit_l(abs_s(h[i]));
        L_g0   = L_add(L_g0, L_temp);
    }
    g0 = extract_h(L_shl(L_g0, 14));

    /* Scale signal input of 1/A(gamma1) */
    if(sub(g0, 1024)>0) {
        temp = div_s(1024, g0);     /* temp = 2**15 / gain0 */
        for(i=0; i<L_SUBFR; i++) {
            sig_ltp_ptr[i] = mult_r(sig_ltp_ptr[i], temp);
        }
    }

    return;
}

/*----------------------------------------------------------------------------
 * calc_rc0_h - computes 1st parcor from composed filter impulse response
 *----------------------------------------------------------------------------
 */
static void calc_rc0_he(
 Word16 *h,      /* input : impulse response of composed filter */
 Word16 *rc0,     /* output: 1st parcor */
  Word16  long_h_st  /*input : impulse response length */
)
{
    Word16 acf0, acf1;
    Word32 L_acc;
    Word16 temp, sh_acf;
    Word16 *ptrs;
    int i;


    /* computation of the autocorrelation function acf */
    L_acc  = 0L;
    for(i=0; i<long_h_st; i++) L_acc = L_mac(L_acc, h[i], h[i]);
    sh_acf = norm_l(L_acc);
    L_acc  = L_shl(L_acc, sh_acf);
    acf0   = extract_h(L_acc);

    L_acc  = 0L;
    ptrs   = h;
    for(i=0; i<long_h_st-1; i++){
        temp = *ptrs++;
        L_acc = L_mac(L_acc, temp, *ptrs);
    }
    L_acc = L_shl(L_acc, sh_acf);
    acf1  = extract_h(L_acc);

    /* Compute 1st parcor */
    /**********************/
    if( sub(acf0, abs_s(acf1))<0) {
        *rc0 = 0;
        return;
    }
    *rc0 = div_s(abs_s(acf1), acf0);
    if(acf1 > 0) {
        *rc0 = negate(*rc0);
    }

    return;
}


/*----------------------------------------------------------------------------
 * filt_mu - tilt filtering with : (1 + mu z-1) * (1/1-|mu|)
 *   computes y[n] = (1/1-|mu|) (x[n]+mu*x[n-1])
 *----------------------------------------------------------------------------
 */
static void filt_mu(
 Word16 *sig_in,     /* input : input signal (beginning at sample -1) */
 Word16 *sig_out,    /* output: output signal */
 Word16 parcor0      /* input : parcor0 (mu = parcor0 * gamma3) */
)
{
    int n;
    Word16 mu, mu2, ga, temp;
    Word32 L_acc, L_temp, L_fact;
    Word16 fact, sh_fact1;
    Word16 *ptrs;

    if(parcor0 > 0) {
        mu      = mult_r(parcor0, GAMMA3_PLUS);
        /* GAMMA3_PLUS < 0.5 */
        sh_fact1 = 15;                   /* sh_fact + 1 */
        fact     = (Word16)0x4000;       /* 2**sh_fact */
        L_fact   = (Word32)0x00004000L;
    }
    else {
        mu       = mult_r(parcor0, GAMMA3_MINUS);
        /* GAMMA3_MINUS < 0.9375 */
        sh_fact1 = 12;                   /* sh_fact + 1 */
        fact     = (Word16)0x0800;       /* 2**sh_fact */
        L_fact   = (Word32)0x00000800L;
    }

    temp = sub(1, abs_s(mu));
    mu2  = add(32767, temp);    /* 2**15 (1 - |mu|) */
    ga   = div_s(fact, mu2);    /* 2**sh_fact / (1 - |mu|) */

    ptrs = sig_in;     /* points on sig_in(-1) */
    mu   = shr(mu, 1);          /* to avoid overflows   */

    for(n=0; n<L_SUBFR; n++) {
        temp   = *ptrs++;
        L_temp = L_deposit_l(*ptrs);
        L_acc  = L_shl(L_temp, 15);         /* sig_in(n) * 2**15 */
        L_temp = L_mac(L_acc, mu, temp);
        L_temp = L_add(L_temp, 0x00004000L);
        temp   = extract_l(L_shr(L_temp,15));
        /* ga x temp x 2 with rounding */
        L_temp = L_add(L_mult(temp, ga),L_fact);
        L_temp = L_shr(L_temp, sh_fact1); /* mult. temp x ga */
        sig_out[n] = sature(L_temp);
    }
    return;
}

/*----------------------------------------------------------------------------
 *   scale_st  - control of the subframe gain
 *   gain[n] = AGC_FAC * gain[n-1] + (1 - AGC_FAC) g_in/g_out
 *----------------------------------------------------------------------------
 */
static void scale_st(
 Word16 *sig_in,     /* input : postfilter input signal */
 Word16 *sig_out,    /* in/out: postfilter output signal */
 Word16 *gain_prec   /* in/out: last value of gain for subframe */
)
{

    int i;
    Word16 scal_in, scal_out;
    Word32 L_acc, L_temp;
    Word16 s_g_in, s_g_out, temp, sh_g0, g0;
    Word16 gain;

    /* compute input gain */
    L_acc = 0L;
    for(i=0; i<L_SUBFR; i++) {
        L_temp  = L_abs(L_deposit_l(sig_in[i]));
        L_acc   = L_add(L_acc, L_temp);
    }

    if(L_acc == 0L) {
        g0 = 0;
    }
    else {
        scal_in = norm_l(L_acc);
        L_acc   = L_shl(L_acc, scal_in);
        s_g_in  = extract_h(L_acc);    /* normalized */

        /* Compute output gain */
        L_acc = 0L;
        for(i=0; i<L_SUBFR; i++) {
            L_temp  = L_abs(L_deposit_l(sig_out[i]));
            L_acc   = L_add(L_acc, L_temp);
        }
        if(L_acc == 0L) {
            *gain_prec = 0;
            return;
        }
        scal_out = norm_l(L_acc);
        L_acc    = L_shl(L_acc, scal_out);
        s_g_out  = extract_h(L_acc);  /* normalized */

        sh_g0    = add(scal_in, 1);
        sh_g0    = sub(sh_g0, scal_out);    /* scal_in - scal_out + 1 */
        if(sub(s_g_in ,s_g_out)<0) {
            g0 = div_s(s_g_in, s_g_out);    /* s_g_in/s_g_out in Q15 */
        }
        else {
            temp  = sub(s_g_in, s_g_out);   /* sufficient since normalized */
            g0    = shr(div_s(temp, s_g_out), 1);
            g0    = add(g0, (Word16)0x4000);/* s_g_in/s_g_out in Q14 */
            sh_g0 = sub(sh_g0, 1);
        }
        /* L_gain_in/L_gain_out in Q14              */
        /* overflows if L_gain_in > 2 * L_gain_out  */
        g0 = shr(g0, sh_g0);        /* sh_g0 may be >0, <0, or =0 */
        g0 = mult_r(g0, AGC_FAC1);  /* L_gain_in/L_gain_out * AGC_FAC1      */

    }

    /* gain(n) = AGC_FAC gain(n-1) + AGC_FAC1 gain_in/gain_out          */
    /* sig_out(n) = gain(n) sig_out(n)                                  */
    gain = *gain_prec;
    for(i=0; i<L_SUBFR; i++) {
        temp = mult_r(AGC_FAC, gain);
        gain = add(temp, g0);            /* in Q14 */
        L_temp = L_mult(gain, sig_out[i]);
        L_temp = L_shl(L_temp, 1);
        sig_out[i] = round(L_temp);
    }
    *gain_prec = gain;
    return;
}
