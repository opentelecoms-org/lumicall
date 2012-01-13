/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.2    Last modified: May 1998 */

/* -------------------------------------------------------------------- */
/*   Coder_ld8e and Init_Coder_ld8e.                        */
/*                                                                      */
/*   (C) Original Copyright 1995, AT&T, France Telecom, NTT,            */
/*   Universite de Sherbrooke.                                          */
/*   All rights reserved.                                               */
/*                                                                      */
/*                                                                      */
/*   Modified to perform higher bit-rate extension of G729 at 11.8 kb/s */
/*                      Bit rate : 11.8 kb/s                            */
/*                                                                      */
/*   (C) Copyright 1997 : France Telecom, Universite de Sherbrooke.     */
/*   All rights reserved.                                               */
/*                                                                      */
/* -------------------------------------------------------------------- */


/*-----------------------------------------------------------------*
 *   Functions Coder_ld8e and Init_Coder_ld8e                      *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8e.h"
#include "tab_ld8k.h"
#include "oper_32b.h"


static void update_exc_err(Word16 gain_pit, Word16 t0);
static Word16 test_err(Word16 t0, Word16 t0_frac);

/*-----------------------------------------------------------*
 *    Coder constant parameters (defined in "ld8k.h")        *
 *-----------------------------------------------------------*
 *   L_WINDOW    : LPC analysis window size.                 *
 *   L_NEXT      : Samples of next frame needed for autocor. *
 *   L_FRAME     : Frame size.                               *
 *   L_SUBFR     : Sub-frame size.                           *
 *   M           : LPC order.                                *
 *   MP1         : LPC order+1                               *
 *   L_TOTAL     : Total size of speech buffer.              *
 *   PIT_MIN     : Minimum pitch lag.                        *
 *   PIT_MAX     : Maximum pitch lag.                        *
 *   L_INTERPOL  : Length of filter for interpolation        *
 *-----------------------------------------------------------*/

/*--------------------------------------------------------*
 *         Static memory allocation.                      *
 *--------------------------------------------------------*/


   /* Speech vector */

    static Word16 old_speech[L_TOTAL];
    static Word16 *speech, *p_window;
    Word16 *new_speech;                    /* Global variable */

   /* Weighted speech vector */

    static Word16 old_wsp[L_FRAME+PIT_MAX];
    static Word16 *wsp;

    /* Excitation vector */
    static Word16 old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
    static Word16 *exc;

    /* Zero vector */
    static Word16 ai_zero[L_SUBFR+M_BWDP1];
    static Word16 *zero;

    /* Lsp (Line spectral pairs) */
    static Word16 lsp_old[M]={
          30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000};
    static Word16 lsp_old_q[M];

    /* Filter's memory */

    static Word16 mem_syn[M_BWD], mem_w0[M_BWD], mem_w[M_BWD];
    static Word16 mem_err[M_BWD+L_SUBFR], *error;
    static Word16 sharp;

    /* to tame the coder */
    static Word32 L_exc_err[4];

    /* for the backward analysis */
    static Word16    prev_filter[M_BWDP1]; /* Previous selected filter */

    static Word32  rexp[M_BWDP1];
    static Word16 synth[L_ANA_BWD];
    static Word16 *synth_ptr;
    static Word16 prev_mode ;
    static Word16 signal[L_FRAME + M_BWD];
    static Word16 *signal_ptr;
    static Word16 gamma1[2], gamma2[2];       /* Weighting factor for the 2 subframes */
    static Word16 A_t_bwd_mem[M_BWDP1];
    static Word16 bwd_dominant, C_int;              /* See file bwfw.c */
    static Word16   glob_stat;  /* Mesure of global stationnarity Q8 */
    static Word16   stat_bwd;       /* Nbre of consecutive backward frames */
    static Word16   val_stat_bwd;   /* Value associated with stat_bwd */

    /* Last backward A(z) for case of unstable filter */
    static Word16 old_A_bwd[M_BWDP1];
    static Word16 old_rc_bwd[2];
    /* Last forkward A(z) for case of unstable filter */
    static Word16 old_A_fwd[MP1];
    static Word16 old_rc_fwd[2];
    static Word16 freq_prev[MA_NP][M];    /* Q13:previous LSP vector       */

/*-----------------------------------------------------------------*
 *   Function  Init_Coder_ld8e                                     *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *  Init_Coder_ld8e(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *       - initialize pointers to speech buffer                    *
 *       - initialize static  pointers                             *
 *       - set static vectors to zero                              *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Init_Coder_ld8e(void)
{

    int i;
  /*----------------------------------------------------------------------*
  *      Initialize pointers to speech vector.                            *
  *                                                                       *
  *                                                                       *
  *   |--------------------|-------------|-------------|------------|     *
  *     previous speech           sf1           sf2         L_NEXT        *
  *                                                                       *
  *   <----------------  Total speech vector (L_TOTAL)   ----------->     *
  *   <----------------  LPC analysis window (L_WINDOW)  ----------->     *
  *   |                   <-- present frame (L_FRAME) -->                 *
  * old_speech            |              <-- new speech (L_FRAME) -->     *
  * p_window              |              |                                *
  *                     speech           |                                *
  *                             new_speech                                *
  *-----------------------------------------------------------------------*/

    new_speech = old_speech + L_TOTAL - L_FRAME;         /* New speech     */
    speech     = new_speech - L_NEXT;                    /* Present frame  */
    p_window   = old_speech + L_TOTAL - L_WINDOW;        /* For LPC window */

    /* Initialize static pointers */
    wsp    = old_wsp + PIT_MAX;
    exc    = old_exc + PIT_MAX + L_INTERPOL;
    zero   = ai_zero + M_BWDP1;
    error  = mem_err + M_BWD;

    /* Static vectors to zero */
    Set_zero(old_speech, L_TOTAL);
    Set_zero(old_exc, PIT_MAX+L_INTERPOL);
    Set_zero(old_wsp, PIT_MAX);
    Set_zero(mem_syn, M_BWD);
    Set_zero(mem_w,   M_BWD);
    Set_zero(mem_w0,  M_BWD);
    Set_zero(mem_err, M_BWD);
    Set_zero(zero, L_SUBFR);
    sharp = SHARPMIN;

    /* Initialize lsp_old_q[] */
    Copy(lsp_old, lsp_old_q, M);

    Lsp_encw_resete(freq_prev);

    for(i=0; i<4; i++) L_exc_err[i] = 0x00004000L;   /* Q14 */

    /* for the backward analysis */
    Set_zero(synth, L_ANA_BWD); /*Q14 */
    synth_ptr = synth + MEM_SYN_BWD;
    prev_mode = 0;
    bwd_dominant = 0;              /* See file bwfw.c */
    C_int = 4506;       /* Filter interpolation parameter */
    glob_stat = 10000;  /* Mesure of global stationnarity Q8 */
    stat_bwd = 0;       /* Nbre of consecutive backward frames */
    val_stat_bwd = 0;   /* Value associated with stat_bwd */

    Set_zero(signal, M_BWD);
    for(i=0; i<M_BWDP1; i++) rexp[i] = 0L;

    signal_ptr = &signal[M_BWD];
    A_t_bwd_mem[0] = 4096;
    for (i=1; i<M_BWDP1; i++) A_t_bwd_mem[i] = 0;
    Set_zero(prev_filter, M_BWDP1);
    prev_filter[0] = 4096;

    Set_zero(old_A_bwd, M_BWDP1);
    old_A_bwd[0]=4096;
    Set_zero(old_rc_bwd, 2);

    Set_zero(old_A_fwd, MP1);
    old_A_fwd[0]=4096;
    Set_zero(old_rc_fwd, 2);

    return;
}

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8e                                          *
 *            ~~~~~~~~~~                                           *
 *  Coder_ld8e(Word16 ana[], Word16 rate);                         *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *   80 speech data should have been copied to vector new_speech[].*
 *   This vector is global and is declared in this function.       *
  *   rate :       rate for the current frame                      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Coder_ld8e(
     Word16 ana[],      /* output  : Analysis parameters */
     Word16 rate        /* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps*/
)
{

  /* LPC analysis */
    Word16 r_l_fwd[MP1], r_h_fwd[MP1];    /* Autocorrelations low and hi (forward) */
    Word32 r_bwd[M_BWDP1];      /* Autocorrelations (backward) */
    Word16 r_l_bwd[M_BWDP1];      /* Autocorrelations low (backward) */
    Word16 r_h_bwd[M_BWDP1];      /* Autocorrelations high (backward) */
    Word16 rc_fwd[M];                 /* Reflection coefficients : forward analysis */
    Word16 rc_bwd[M_BWD];         /* Reflection coefficients : backward analysis */
    Word16 A_t_fwd[MP1*2];          /* A(z) forward unquantized for the 2 subframes */
    Word16 A_t_fwd_q[MP1*2];      /* A(z) forward quantized for the 2 subframes */
    Word16 A_t_bwd[2*M_BWDP1];    /* A(z) backward for the 2 subframes */
    Word16 *Aq;           /* A(z) "quantized" for the 2 subframes */
    Word16 *Ap;           /* A(z) "unquantized" for the 2 subframes */
    Word16 *pAp, *pAq;
    Word16 Ap1[M_BWDP1];          /* A(z) with spectral expansion         */
    Word16 Ap2[M_BWDP1];          /* A(z) with spectral expansion         */
    Word16 lsp_new[M], lsp_new_q[M]; /* LSPs at 2th subframe                 */
    Word16 lsf_int[M];               /* Interpolated LSF 1st subframe.       */
    Word16 lsf_new[M];
    Word16 mode;                  /* Backward / Forward Indication mode */
    Word16 m_ap, m_aq, i_gamma;
    Word16 code_lsp[2];

    /* Other vectors */

    Word16 h1[L_SUBFR];            /* Impulse response h1[]              */
    Word16 xn[L_SUBFR];            /* Target vector for pitch search     */
    Word16 xn2[L_SUBFR];           /* Target vector for codebook search  */
    Word16 code[L_SUBFR];          /* Fixed codebook excitation          */
    Word16 y1[L_SUBFR];            /* Filtered adaptive excitation       */
    Word16 y2[L_SUBFR];            /* Filtered fixed codebook excitation */
    Word16 g_coeff[4];             /* Correlations between xn & y1       */
    Word16 res2[L_SUBFR];          /* residual after long term prediction*/
    Word16 g_coeff_cs[5];
    Word16 exp_g_coeff_cs[5];      /* Correlations between xn, y1, & y2
                                     <y1,y1>, -2<xn,y1>,
                                          <y2,y2>, -2<xn,y2>, 2<y1,y2> */
    /* Scalars */
    Word16 i, j, k, i_subfr;
    Word16 T_op, T0, T0_min, T0_max, T0_frac;
    Word16 gain_pit, gain_code, index;
    Word16 temp, pit_sharp;
    Word16 sat_filter;
    Word32 L_temp;
    Word16 freq_cur[M];

/*------------------------------------------------------------------------*
 *  - Perform LPC analysis:                                               *
 *       * autocorrelation + lag windowing                                *
 *       * Levinson-durbin algorithm to find a[]                          *
 *       * convert a[] to lsp[]                                           *
 *       * quantize and code the LSPs                                     *
 *       * find the interpolated LSPs and convert to a[] for the 2        *
 *         subframes (both quantized and unquantized)                     *
 *------------------------------------------------------------------------*/

  /* -------------------- */
  /* LP Backward analysis */
  /* -------------------- */
    if (rate == 1) {

        /* LPC recursive Window as in G728 */
        autocorr_hyb_window(synth, r_bwd, rexp); /* Autocorrelations */

        Lag_window_bwd(r_bwd, r_h_bwd, r_l_bwd);  /* Lag windowing    */

        /* Fixed Point Levinson (as in G729) */
        Levinsone(M_BWD, r_h_bwd, r_l_bwd, &A_t_bwd[M_BWDP1], rc_bwd,
                    old_A_bwd, old_rc_bwd );

        /* Tests saturation of A_t_bwd */
        sat_filter = 0;
        for (i=M_BWDP1; i<2*M_BWDP1; i++) if (A_t_bwd[i] >= 32767) sat_filter = 1;
        if (sat_filter == 1) Copy(A_t_bwd_mem, &A_t_bwd[M_BWDP1], M_BWDP1);
        else Copy(&A_t_bwd[M_BWDP1], A_t_bwd_mem, M_BWDP1);

        /* Additional bandwidth expansion on backward filter */
        Weight_Az(&A_t_bwd[M_BWDP1], GAMMA_BWD, M_BWD, &A_t_bwd[M_BWDP1]);
    }
    /*--------------------------------------------------*
    * Update synthesis signal for next frame.          *
    *--------------------------------------------------*/
    Copy(&synth[L_FRAME], &synth[0], MEM_SYN_BWD);

    Copy(speech, signal_ptr, L_FRAME);

    /* ------------------- */
    /* LP Forward analysis */
    /* ------------------- */
    Autocorr(p_window, M, r_h_fwd, r_l_fwd);              /* Autocorrelations */
    Lag_window(M, r_h_fwd, r_l_fwd);                      /* Lag windowing    */
    Levinsone(M, r_h_fwd, r_l_fwd, &A_t_fwd[MP1], rc_fwd, /* Levinson Durbin  */
                    old_A_fwd, old_rc_fwd );

    Az_lsp(&A_t_fwd[MP1], lsp_new, lsp_old);      /* From A(z) to lsp */

    /* ---------------- */
    /* LSP quantization */
    /* ---------------- */
    Qua_lspe(lsp_new, lsp_new_q, code_lsp, freq_prev, freq_cur);

    /*--------------------------------------------------------------------*
    * Find interpolated LPC parameters in all subframes (both quantized  *
    * and unquantized).                                                  *
    * The interpolated parameters are in array A_t[] of size (M+1)*4     *
    * and the quantized interpolated parameters are in array Aq_t[]      *
    *--------------------------------------------------------------------*/
    if( prev_mode == 0) {
        Int_lpc(lsp_old, lsp_new, lsf_int, lsf_new, A_t_fwd);
        Int_qlpc(lsp_old_q, lsp_new_q, A_t_fwd_q);
    }
    else {
        /* no interpolation */
        /* unquantized */
        Lsp_Az(lsp_new, A_t_fwd);           /* Subframe 1 */
        Lsp_lsf(lsp_new, lsf_new, M);  /* transformation from LSP to LSF (freq.domain) */
        Copy(lsf_new, lsf_int, M);      /* Subframe 1 */

        /* quantized */
        Lsp_Az(lsp_new_q, &A_t_fwd_q[MP1]);              /* Subframe 2 */
        Copy(&A_t_fwd_q[MP1], A_t_fwd_q, MP1);      /* Subframe 1 */
    }

    /*---------------------------------------------------------------------*
    * - Decision for the switch Forward / Backward                        *
    *---------------------------------------------------------------------*/
    set_lpc_mode(signal_ptr, A_t_fwd_q, A_t_bwd, &mode, lsp_new, lsp_old, rate,
        &bwd_dominant, prev_mode, prev_filter, &C_int,
        &glob_stat, &stat_bwd, &val_stat_bwd);

    if(rate == 1) *ana++ = mode;

    if(mode == 0) {
        C_int = 4506;
        m_ap = M;
        m_aq = M;
        Aq = A_t_fwd_q;
        if (bwd_dominant == 0) Ap = A_t_fwd;
        else Ap = A_t_fwd_q;
        /* update previous filter for next frame */
        Copy(&Aq[MP1], prev_filter, MP1);
        for(i=MP1; i <M_BWDP1; i++) prev_filter[i] = 0;
        for(j=MP1; j<M_BWDP1; j++) ai_zero[j] = 0;
    }
    else {
        m_aq = M_BWD;
        Aq = A_t_bwd;
        if (bwd_dominant == 0) {
            m_ap = M;
            Ap = A_t_fwd;
        }
        else {
            m_ap = M_BWD;
            Ap = A_t_bwd;
        }
        /* update previous filter for next frame */
        Copy(&Aq[M_BWDP1], prev_filter, M_BWDP1);
    }
    /* ---------------------------------- */
    /* update the LSPs for the next frame */
    /* ---------------------------------- */
    Copy(lsp_new, lsp_old, M);


    /*----------------------------------------------------------------------*
     * - Find the weighting factors                                         *
     *----------------------------------------------------------------------*/
    /*----------------------------------------------------------------------*
    * - Find the weighted input speech w_sp[] for the whole speech frame   *
    * - Find the open-loop pitch delay                                     *
    *----------------------------------------------------------------------*/
    if( mode == 0) {
        Copy(lsp_new_q, lsp_old_q, M);
        Lsp_prev_update(freq_cur, freq_prev);
        *ana++ = code_lsp[0];
        *ana++ = code_lsp[1];

        perc_var(gamma1, gamma2, lsf_int, lsf_new, rc_fwd);
    }
    else {
        perc_vare(gamma1, gamma2, bwd_dominant);
    }
    pAp = Ap;
    for (i=0; i<2; i++) {
        Weight_Az(pAp, gamma1[i], m_ap, Ap1);
        Weight_Az(pAp, gamma2[i], m_ap, Ap2);
        Residue(m_ap, Ap1, &speech[i*L_SUBFR], &wsp[i*L_SUBFR], L_SUBFR);
        Syn_filte(m_ap,  Ap2, &wsp[i*L_SUBFR], &wsp[i*L_SUBFR], L_SUBFR,
            &mem_w[M_BWD-m_ap], 0);
        for(j=0; j<M_BWD; j++) mem_w[j] = wsp[i*L_SUBFR+L_SUBFR-M_BWD+j];
        pAp += m_ap+1;
    }

    /* Find open loop pitch lag */
    T_op = Pitch_ol(wsp, PIT_MIN, PIT_MAX, L_FRAME);

    /* Range for closed loop pitch search in 1st subframe */
    T0_min = sub(T_op, 3);
    if (sub(T0_min,PIT_MIN)<0) {
        T0_min = PIT_MIN;
    }

    T0_max = add(T0_min, 6);
    if (sub(T0_max ,PIT_MAX)>0)
    {
        T0_max = PIT_MAX;
        T0_min = sub(T0_max, 6);
    }

    /*------------------------------------------------------------------------*
    *          Loop for every subframe in the analysis frame                 *
    *------------------------------------------------------------------------*
    *  To find the pitch and innovation parameters. The subframe size is     *
    *  L_SUBFR and the loop is repeated 2 times.                             *
    *     - find the weighted LPC coefficients                               *
    *     - find the LPC residual signal res[]                               *
    *     - compute the target signal for pitch search                       *
    *     - compute impulse response of weighted synthesis filter (h1[])     *
    *     - find the closed-loop pitch parameters                            *
    *     - encode the pitch delay                                           *
    *     - update the impulse response h1[] by including fixed-gain pitch   *
    *     - find target vector for codebook search                           *
    *     - codebook search                                                  *
    *     - encode codebook address                                          *
    *     - VQ of pitch and codebook gains                                   *
    *     - find synthesis speech                                            *
    *     - update states of weighting filter                                *
    *------------------------------------------------------------------------*/
    pAp  = Ap;     /* pointer to interpolated "unquantized"LPC parameters           */
    pAq = Aq;    /* pointer to interpolated "quantized" LPC parameters */

    i_gamma = 0;

    for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR) {

        /*---------------------------------------------------------------*
        * Find the weighted LPC coefficients for the weighting filter.  *
        *---------------------------------------------------------------*/
        Weight_Az(pAp, gamma1[i_gamma], m_ap, Ap1);
        Weight_Az(pAp, gamma2[i_gamma], m_ap, Ap2);

        /*---------------------------------------------------------------*
        * Compute impulse response, h1[], of weighted synthesis filter  *
        *---------------------------------------------------------------*/
        for (i = 0; i <=m_ap; i++) ai_zero[i] = Ap1[i];
        Syn_filte(m_aq,  pAq, ai_zero, h1, L_SUBFR, zero, 0);
        Syn_filte(m_ap,  Ap2, h1, h1, L_SUBFR, zero, 0);

        /*------------------------------------------------------------------------*
        *                                                                        *
        *          Find the target vector for pitch search:                      *
        *          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                       *
        *                                                                        *
        *              |------|  res[n]                                          *
        *  speech[n]---| A(z) |--------                                          *
        *              |------|       |   |--------| error[n]  |------|          *
        *                    zero -- (-)--| 1/A(z) |-----------| W(z) |-- target *
        *                    exc          |--------|           |------|          *
        *                                                                        *
        * Instead of subtracting the zero-input response of filters from         *
        * the weighted input speech, the above configuration is used to          *
        * compute the target vector. This configuration gives better performance *
        * with fixed-point implementation. The memory of 1/A(z) is updated by    *
        * filtering (res[n]-exc[n]) through 1/A(z), or simply by subtracting     *
        * the synthesis speech from the input speech:                            *
        *    error[n] = speech[n] - syn[n].                                      *
        * The memory of W(z) is updated by filtering error[n] through W(z),      *
        * or more simply by subtracting the filtered adaptive and fixed          *
        * codebook excitations from the target:                                  *
        *     target[n] - gain_pit*y1[n] - gain_code*y2[n]                       *
        * as these signals are already available.                                *
        *                                                                        *
        *------------------------------------------------------------------------*/
        Residue(m_aq, pAq, &speech[i_subfr], &exc[i_subfr], L_SUBFR);   /* LPC residual */
        for (i=0; i<L_SUBFR; i++) res2[i] = exc[i_subfr+i];
        Syn_filte(m_aq,  pAq, &exc[i_subfr], error, L_SUBFR,
                &mem_err[M_BWD-m_aq], 0);
        Residue(m_ap, Ap1, error, xn, L_SUBFR);
        Syn_filte(m_ap,  Ap2, xn, xn, L_SUBFR, &mem_w0[M_BWD-m_ap], 0);    /* target signal xn[]*/

        /*----------------------------------------------------------------------*
        *                 Closed-loop fractional pitch search                  *
        *----------------------------------------------------------------------*/
        T0 = Pitch_fr3(&exc[i_subfr], xn, h1, L_SUBFR, T0_min, T0_max,
                               i_subfr, &T0_frac);

        index = Enc_lag3(T0, T0_frac, &T0_min, &T0_max,PIT_MIN,PIT_MAX,i_subfr);

        *ana++ = index;

        if (i_subfr == 0) {
            *ana = Parity_Pitch(index);
            if( rate == 1) {
                *ana ^= (shr(index, 1) & 0x0001);
            }
            ana++;
        }
       /*-----------------------------------------------------------------*
        *   - find unity gain pitch excitation (adaptive codebook entry)  *
        *     with fractional interpolation.                              *
        *   - find filtered pitch exc. y1[]=exc[] convolve with h1[])     *
        *   - compute pitch gain and limit between 0 and 1.2              *
        *   - update target vector for codebook search                    *
        *   - find LTP residual.                                          *
        *-----------------------------------------------------------------*/

        Pred_lt_3(&exc[i_subfr], T0, T0_frac, L_SUBFR);

        Convolve(&exc[i_subfr], h1, y1, L_SUBFR);

        gain_pit = G_pitch(xn, y1, g_coeff, L_SUBFR);


        /* clip pitch gain if taming is necessary */
        temp = test_err(T0, T0_frac);
        if( temp == 1){
            if (sub(gain_pit, GPCLIP) > 0) {
                gain_pit = GPCLIP;
            }
        }

        /* xn2[i]   = xn[i] - y1[i] * gain_pit  */

        for (i = 0; i < L_SUBFR; i++) {
            L_temp = L_mult(y1[i], gain_pit);
            L_temp = L_shl(L_temp, 1);               /* gain_pit in Q14 */
            xn2[i] = sub(xn[i], extract_h(L_temp));
        }

        /*-----------------------------------------------------*
        * - Innovative codebook search.                       *
        *-----------------------------------------------------*/

        if(rate == 0) {
         /* case 8 kbit/s */
            index = ACELP_Codebook(xn2, h1, T0, sharp, i_subfr, code, y2, &i);
            *ana++ = index;        /* Positions index */
            *ana++ = i;            /* Signs index     */
        }
        else {
             /* case 11.8 kbit/s */
           /*-----------------------------------------------------------------*
            * Include fixed-gain pitch contribution into impulse resp. h[]    *
            *-----------------------------------------------------------------*/
            pit_sharp = shl(sharp, 1);        /* From Q14 to Q15 */
            for (i = T0; i < L_SUBFR; i++){   /* h[i] += pitch_sharp*h[i-T0] */
              h1[i] = add(h1[i], mult(h1[i-T0], pit_sharp));
            }

            /* calculate residual after long term prediction */
            /* res2[i] -= exc[i+i_subfr] * gain_pit */
            for (i = 0; i < L_SUBFR; i++) {
                L_temp = L_mult(exc[i+i_subfr], gain_pit);
                L_temp = L_shl(L_temp, 1);               /* gain_pit in Q14 */
                res2[i] = sub(res2[i], extract_h(L_temp));
            }
            if (mode == 0) ACELP_10i40_35bits(xn2, res2, h1, code, y2, ana); /* Forward */
            else ACELP_12i40_44bits(xn2, res2, h1, code, y2, ana); /* Backward */
            ana += 5;

           /*-----------------------------------------------------------------*
            * Include fixed-gain pitch contribution into code[].              *
            *-----------------------------------------------------------------*/
            for (i = T0; i < L_SUBFR; i++) {   /* code[i] += pitch_sharp*code[i-T0] */
                code[i] = add(code[i], mult(code[i-T0], pit_sharp));
            }
        }

        /*-----------------------------------------------------*
        * - Quantization of gains.                            *
        *-----------------------------------------------------*/

        g_coeff_cs[0]     = g_coeff[0];                   /* <y1,y1> */
        exp_g_coeff_cs[0] = negate(g_coeff[1]);           /* Q-Format:XXX -> JPN  */
        g_coeff_cs[1]     = negate(g_coeff[2]);           /* (xn,y1) -> -2<xn,y1> */
        exp_g_coeff_cs[1] = negate(add(g_coeff[3], 1));   /* Q-Format:XXX -> JPN  */

        Corr_xy2( xn, y1, y2, g_coeff_cs, exp_g_coeff_cs );  /* Q0 Q0 Q12 ^Qx ^Q0 */
                         /* g_coeff_cs[3]:exp_g_coeff_cs[3] = <y2,y2>   */
                         /* g_coeff_cs[4]:exp_g_coeff_cs[4] = -2<xn,y2> */
                         /* g_coeff_cs[5]:exp_g_coeff_cs[5] = 2<y1,y2>  */

        index = Qua_gain(code, g_coeff_cs, exp_g_coeff_cs,
                      L_SUBFR, &gain_pit, &gain_code, temp);
        *ana++ = index;

        /*------------------------------------------------------------*
        * - Update pitch sharpening "sharp" with quantized gain_pit  *
        *------------------------------------------------------------*/

        sharp = gain_pit;
        if (sub(sharp, SHARPMAX) > 0) sharp = SHARPMAX;
        else {
            if (sub(sharp, SHARPMIN) < 0) sharp = SHARPMIN;
        }

        /*------------------------------------------------------*
        * - Find the total excitation                          *
        * - find synthesis speech corresponding to exc[]       *
        * - update filters memories for finding the target     *
        *   vector in the next subframe                        *
        *   (update error[-m..-1] and mem_w_err[])             *
        *   update error function for taming process           *
        *------------------------------------------------------*/
        for (i = 0; i < L_SUBFR;  i++) {
            /* exc[i] = gain_pit*exc[i] + gain_code*code[i]; */
            /* exc[i]  in Q0   gain_pit in Q14               */
            /* code[i] in Q13  gain_cod in Q1                */

            L_temp = L_mult(exc[i+i_subfr], gain_pit);
            L_temp = L_mac(L_temp, code[i], gain_code);
            L_temp = L_shl(L_temp, 1);
            exc[i+i_subfr] = round(L_temp);
        }

        update_exc_err(gain_pit, T0);

        Syn_filte(m_aq,  pAq, &exc[i_subfr], &synth_ptr[i_subfr], L_SUBFR,
                &mem_syn[M_BWD-m_aq], 0);
        for(j=0; j<M_BWD; j++) mem_syn[j] = synth_ptr[i_subfr+L_SUBFR-M_BWD+j];

        for (i = L_SUBFR-M_BWD, j = 0; i < L_SUBFR; i++, j++) {
            mem_err[j] = sub(speech[i_subfr+i], synth_ptr[i_subfr+i]);
            temp       = extract_h(L_shl( L_mult(y1[i], gain_pit),  1) );
            k          = extract_h(L_shl( L_mult(y2[i], gain_code), 2) );
            mem_w0[j]  = sub(xn[i], add(temp, k));
        }
        pAp   += m_ap+1;
        pAq   += m_aq+1;
        i_gamma = add(i_gamma,1);
    }


    /*--------------------------------------------------*
    * Update signal for next frame.                    *
    * -> shift to the left by L_FRAME:                 *
    *     speech[], wsp[] and  exc[]                   *
    *--------------------------------------------------*/
    Copy(&old_speech[L_FRAME], &old_speech[0], L_TOTAL-L_FRAME);
    Copy(&old_wsp[L_FRAME], &old_wsp[0], PIT_MAX);
    Copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);
    Copy(&signal[L_FRAME], &signal[0], M_BWD);
    prev_mode = mode;
    return;
}

/*---------------------------------------------------------------------------*
 * routine  corr_xy2()                                                       *
 * ~~~~~~~~~~~~~~~~~~~~                                                      *
 * Find the correlations between the target xn[], the filtered adaptive      *
 * codebook excitation y1[], and the filtered 1st codebook innovation y2[].  *
 *   g_coeff[2]:exp_g_coeff[2] = <y2,y2>                                     *
 *   g_coeff[3]:exp_g_coeff[3] = -2<xn,y2>                                   *
 *   g_coeff[4]:exp_g_coeff[4] = 2<y1,y2>                                    *
 *---------------------------------------------------------------------------*/
void Corr_xy2(
      Word16 xn[],           /* (i) Q0  :Target vector.                  */
      Word16 y1[],           /* (i) Q0  :Adaptive codebook.              */
      Word16 y2[],           /* (i) Q12 :Filtered innovative vector.     */
      Word16 g_coeff[],      /* (o) Q[exp]:Correlations between xn,y1,y2 */
      Word16 exp_g_coeff[]   /* (o)       :Q-format of g_coeff[]         */
)
{
      Word16   i,exp;
      Word16   exp_y2y2,exp_xny2,exp_y1y2;
      Word16   y2y2,    xny2,    y1y2;
      Word32   L_acc;
      Word16   scaled_y2[L_SUBFR];       /* Q9 */

      /*------------------------------------------------------------------*
       * Scale down y2[] from Q12 to Q9 to avoid overflow                 *
       *------------------------------------------------------------------*/

      for(i=0; i<L_SUBFR; i++)
         scaled_y2[i] = shr(y2[i], 3);

      /* Compute scalar product <y2[],y2[]> */

      L_acc = 1;                       /* Avoid case of all zeros */
      for(i=0; i<L_SUBFR; i++)
         L_acc = L_mac(L_acc, scaled_y2[i], scaled_y2[i]);    /* L_acc:Q19 */

      exp      = norm_l(L_acc);
      y2y2     = round( L_shl(L_acc, exp) );
      exp_y2y2 = add(exp, 19-16);                          /* Q[19+exp-16] */

      g_coeff[2]     = y2y2;
      exp_g_coeff[2] = exp_y2y2;

      /* Compute scalar product <xn[],y2[]> */

      L_acc = 1;                       /* Avoid case of all zeros */
      for(i=0; i<L_SUBFR; i++)
         L_acc = L_mac(L_acc, xn[i], scaled_y2[i]);           /* L_acc:Q10 */

      exp      = norm_l(L_acc);
      xny2     = round( L_shl(L_acc, exp) );
      exp_xny2 = add(exp, 10-16);                          /* Q[10+exp-16] */

      g_coeff[3]     = negate(xny2);
      exp_g_coeff[3] = sub(exp_xny2,1);                   /* -2<xn,y2> */

      /* Compute scalar product <y1[],y2[]> */

      L_acc = 1;                       /* Avoid case of all zeros */
      for(i=0; i<L_SUBFR; i++)
         L_acc = L_mac(L_acc, y1[i], scaled_y2[i]);           /* L_acc:Q10 */

      exp      = norm_l(L_acc);
      y1y2     = round( L_shl(L_acc, exp) );
      exp_y1y2 = add(exp, 10-16);                          /* Q[10+exp-16] */

      g_coeff[4]     = y1y2;
      exp_g_coeff[4] = sub(exp_y1y2,1);    ;                /* 2<y1,y2> */

      return;
}

/**************************************************************************
 * routine test_err - computes the accumulated potential error in the     *
 * adaptive codebook contribution                                         *
 **************************************************************************/

static Word16 test_err(  /* (o) flag set to 1 if taming is necessary  */
 Word16 T0,       /* (i) integer part of pitch delay           */
 Word16 T0_frac   /* (i) fractional part of pitch delay        */
)
 {
    Word16 i, t1, zone1, zone2, flag;
    Word32 L_maxloc, L_acc;

    if(T0_frac > 0) {
        t1 = add(T0, 1);
    }
    else {
        t1 = T0;
    }

    i = sub(t1, (L_SUBFR+L_INTER10));
    if(i < 0) {
        i = 0;
    }
    zone1 = tab_zone[i];

    i = add(t1, (L_INTER10 - 2));
    zone2 = tab_zone[i];

    L_maxloc = -1L;
    flag = 0 ;
    for(i=zone2; i>=zone1; i--) {
        L_acc = L_sub(L_exc_err[i], L_maxloc);
        if(L_acc > 0L) {
                L_maxloc = L_exc_err[i];
        }
    }
    L_acc = L_sub(L_maxloc, L_THRESH_ERR);
    if(L_acc > 0L) {
        flag = 1;
    }

    return(flag);
}

/**************************************************************************
 *routine update_exc_err - maintains the memory used to compute the error *
 * function due to an adaptive codebook mismatch between encoder and      *
 * decoder                                                                *
 **************************************************************************/

static void update_exc_err(
 Word16 gain_pit,      /* (i) pitch gain */
 Word16 T0             /* (i) integer part of pitch delay */
)
 {

    Word16 i, zone1, zone2, n;
    Word32 L_worst, L_temp, L_acc;
    Word16 hi, lo;

    L_worst = -1L;
    n = sub(T0, L_SUBFR);

    if(n < 0) {
        L_Extract(L_exc_err[0], &hi, &lo);
        L_temp = Mpy_32_16(hi, lo, gain_pit);
        L_temp = L_shl(L_temp, 1);
        L_temp = L_add(0x00004000L, L_temp);
        L_acc = L_sub(L_temp, L_worst);
        if(L_acc > 0L) {
                L_worst = L_temp;
        }
        L_Extract(L_temp, &hi, &lo);
        L_temp = Mpy_32_16(hi, lo, gain_pit);
        L_temp = L_shl(L_temp, 1);
        L_temp = L_add(0x00004000L, L_temp);
        L_acc = L_sub(L_temp, L_worst);
        if(L_acc > 0L) {
                L_worst = L_temp;
        }
    }

    else {

        zone1 = tab_zone[n];

        i = sub(T0, 1);
        zone2 = tab_zone[i];

        for(i = zone1; i <= zone2; i++) {
                L_Extract(L_exc_err[i], &hi, &lo);
                L_temp = Mpy_32_16(hi, lo, gain_pit);
                L_temp = L_shl(L_temp, 1);
                L_temp = L_add(0x00004000L, L_temp);
                L_acc = L_sub(L_temp, L_worst);
                if(L_acc > 0L) L_worst = L_temp;
        }
    }

    for(i=3; i>=1; i--) {
        L_exc_err[i] = L_exc_err[i-1];
    }
    L_exc_err[0] = L_worst;

    return;
}

