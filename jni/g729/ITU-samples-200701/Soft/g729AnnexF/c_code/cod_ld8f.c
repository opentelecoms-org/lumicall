/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex F  - Reference C code for fixed point
                         implementation of G.729 Annex F
                         (integration of Annexes B and D)
   Version 1.2    Last modified: October 2006 
*/

/*
 File : COD_LD8F.C
 */

/* from codld8kd.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from cod_ld8k.c G.729 Annex B Version 1.3  Last modified: August 1997 */
/* from cod_ld8k.c G.729 Version 3.3 */

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8f and Init_Coder_ld8f                      *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "ld8f.h"
#include "tab_ld8k.h"
#include "oper_32b.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"

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
    static Word16 ai_zero[L_SUBFR+MP1];
    static Word16 *zero;

    /* Lsp (Line spectral pairs) */
    static Word16 lsp_old[M]={
          30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000};
    static Word16 lsp_old_q[M];

    /* Filter's memory */
    static Word16 mem_syn[M], mem_w0[M], mem_w[M];
    static Word16 mem_err[M+L_SUBFR], *error;
    static Word16 sharp;

    /* gain predictor  memory in Q10 */
    static Word16 past_qua_en[4] = { -14336, -14336, -14336, -14336 };

    /* to tame the coder */
    /* static Word32 L_exc_err[4]; */	/* G.729 maintenance */
    /* For G.729B */
    /* DTX variables */
    static Word16 pastVad;
    static Word16 ppastVad;
    static Word16 seed;

    static Word16 freq_prev[MA_NP][M];    /* Q13:previous LSP vector       */

/*-----------------------------------------------------------------*
 *   Function  Init_Coder_ld8f                                     *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *  Init_Coder_ld8f(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *       - initialize pointers to speech buffer                    *
 *       - initialize static  pointers                             *
 *       - set static vectors to zero                              *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Init_Coder_ld8f(Word16 dtx_enable)
{

    Word16 i;
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
    zero   = ai_zero + MP1;
    error  = mem_err + M;

    /* Static vectors to zero */
    Set_zero(old_speech, L_TOTAL);
    Set_zero(old_exc, PIT_MAX+L_INTERPOL);
    Set_zero(old_wsp, PIT_MAX);
    Set_zero(mem_syn, M);
    Set_zero(mem_w,   M);
    Set_zero(mem_w0,  M);
    Set_zero(mem_err, M);
    Set_zero(zero, L_SUBFR);
    sharp = SHARPMIN;

    /* Initialize lsp_old_q[] */
    Copy(lsp_old, lsp_old_q, M);

    Lsp_encw_reset();

	/* to tame the coder */
	Init_exc_err();			/* G.729 maintenance */
							/* before : for(i=0; i<4; i++) L_exc_err[i] = 0x00004000L;   /* Q14 */

    /* For G.729B */
    /* Initialize VAD/DTX parameters */
    if(dtx_enable == 1) {
        pastVad = 1;
        ppastVad = 1;
        seed = INIT_SEED;
        vad_init();
        Init_lsfq_noise();
		Init_Cod_cng();		/* G.729 maintenance */
    }

    return;
}

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8f                                          *
 *            ~~~~~~~~~~                                           *
 *  Coder_ld8f(Word16 ana[], Word16 rate);                         *
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

void Coder_ld8f(
  Word16 ana[],     /* (o)     : analysis parameters                        */
  Word16 frame,     /* input : frame counter */
  Word16 dtx_enable, /* input : DTX enable flag */
  Word16 rate        /* input   : rate selector/frame  =0 6.4kbps , =1 8kbps*/
)
{

  /* LPC analysis */
    Word16 r_l[NP+1], r_h[NP+1];    /* Autocorrelations low and hi (forward) */
    Word16 rc[M];                 /* Reflection coefficients : forward analysis */
    Word16 A_t[MP1*2];          /* A(z) forward unquantized for the 2 subframes */
    Word16 A_t_q[MP1*2];      /* A(z) forward quantized for the 2 subframes */
    Word16 *pAp, *pAq;
    Word16 Ap1[MP1];          /* A(z) with spectral expansion         */
    Word16 Ap2[MP1];          /* A(z) with spectral expansion         */
    Word16 lsp_new[M], lsp_new_q[M]; /* LSPs at 2th subframe                 */
    Word16 lsf_int[M];               /* Interpolated LSF 1st subframe.       */
    Word16 lsf_new[M];
    Word16 i_gamma;
    Word16 code_lsp[2];

    /* Other vectors */
  Word16 synth[L_FRAME];           /* Buffer for synthesis speech           */
  Word16 gamma1[2], gamma2[2];     /* Weighting factor for the 2 subframes */
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
    Word16 taming;
    Word32 L_temp;
    Word16 freq_cur[M];

    /* For G.729B */
    Word16 rh_nbe[MP1];
    Word16 lsfq_mem[MA_NP][M];
    Word16 exp_R0, Vad;

    Word16 temp;
    
/*------------------------------------------------------------------------*
 *  - Perform LPC analysis:                                               *
 *       * autocorrelation + lag windowing                                *
 *       * Levinson-durbin algorithm to find a[]                          *
 *       * convert a[] to lsp[]                                           *
 *       * quantize and code the LSPs                                     *
 *       * find the interpolated LSPs and convert to a[] for the 2        *
 *         subframes (both quantized and unquantized)                     *
 *------------------------------------------------------------------------*/
    /* ------------------- */
    /* LP Forward analysis */
    /* ------------------- */
    Autocorr(p_window, NP, r_h, r_l, &exp_R0);    /* Autocorrelations */
    Copy(r_h, rh_nbe, MP1);
    Lag_window(NP, r_h, r_l);                     /* Lag windowing    */
    Levinson(r_h, r_l, &A_t[MP1], rc, &temp ); /* Levinson Durbin  */
    Az_lsp(&A_t[MP1], lsp_new, lsp_old);      /* From A(z) to lsp */

    /* For G.729B */
    /* ------ VAD ------- */
    if (dtx_enable == 1) {
        Lsp_lsf(lsp_new, lsf_new, M);
        vad(rc[1], lsf_new, r_h, r_l, exp_R0, p_window, frame,
            pastVad, ppastVad, &Vad);
        Update_cng(rh_nbe, exp_R0, Vad);
    }
    else Vad = 1;

    /*--------------------------------------------------------------------*
    * Find interpolated LPC parameters in all subframes (unquantized).                                                  *
    * The interpolated parameters are in array A_t[] of size (M+1)*4     *
    *--------------------------------------------------------------------*/
    Int_lpc(lsp_old, lsp_new, lsf_int, lsf_new, A_t);

    if(Vad == 1) {
            /* ---------------- */
            /* LSP quantization */
            /* ---------------- */
            Qua_lsp(lsp_new, lsp_new_q, code_lsp);

            /*--------------------------------------------------------------------*
            * Find interpolated LPC parameters in all subframes (quantized)  *
            * the quantized interpolated parameters are in array Aq_t[]      *
            *--------------------------------------------------------------------*/
            Int_qlpc(lsp_old_q, lsp_new_q, A_t_q);
    }
    /* ---------------------------------- */
    /* update the LSPs for the next frame */
    /* ---------------------------------- */
    Copy(lsp_new, lsp_old, M);
    /*----------------------------------------------------------------------*
    * - Find the weighted input speech w_sp[] for the whole speech frame   *
    *----------------------------------------------------------------------*/
    perc_var(gamma1, gamma2, lsf_int, lsf_new, rc);
    pAp = A_t;
    for (i=0; i<2; i++) {
        Weight_Az(pAp, gamma1[i], M, Ap1);
        Weight_Az(pAp, gamma2[i], M, Ap2);
        Residu(Ap1, &speech[i*L_SUBFR], &wsp[i*L_SUBFR], L_SUBFR);
        Syn_filt(Ap2, &wsp[i*L_SUBFR], &wsp[i*L_SUBFR], L_SUBFR, mem_w, 1);
        pAp += MP1;
    }

    /* ---------------------- */
    /* Case of Inactive frame */
    /* ---------------------- */
    if (Vad == 0){
        Get_freq_prev(lsfq_mem);
        Cod_cng(exc, pastVad, lsp_old_q, A_t_q, ana, lsfq_mem, &seed);
        if(ana[0] != 0) ana[0] = 1;
        Update_freq_prev(lsfq_mem);
        ppastVad = pastVad;
        pastVad = Vad;

        /* UPDATE wsp, mem_w, mem_syn, mem_err, and mem_w0 */
        pAp  = A_t;     /* pointer to interpolated LPC parameters           */
        pAq = A_t_q;    /* pointer to interpolated quantized LPC parameters */
        i_gamma = 0;
        for(i_subfr=0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
            Weight_Az(pAp, gamma1[i_gamma], M, Ap1);
            Weight_Az(pAp, gamma2[i_gamma], M, Ap2);
              i_gamma = add(i_gamma,1);

            /* update mem_syn */
            Syn_filt(pAq, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 1);

            /* update mem_w0 */
            for (i=0; i<L_SUBFR; i++)
                error[i] = speech[i_subfr+i] - synth[i_subfr+i];
            Residu(Ap1, error, xn, L_SUBFR);
            Syn_filt(Ap2, xn, xn, L_SUBFR, mem_w0, 1);

            /* update mem_err */
            for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++)
                mem_err[j] = error[i];

            pAp += MP1;
            pAq += MP1;
        }

        sharp = SHARPMIN;

        /* Update memories for next frames */
        Copy(&old_speech[L_FRAME], &old_speech[0], L_TOTAL-L_FRAME);
        Copy(&old_wsp[L_FRAME], &old_wsp[0], PIT_MAX);
        Copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);
        return;

    }  /* End of inactive frame case */

    /* -------------------- */
    /* Case of Active frame */
    /* -------------------- */
    *ana++ = rate+ (Word16)2; /* bit rate mode */

    if(dtx_enable == 1) {
        seed = INIT_SEED;
        ppastVad = pastVad;
        pastVad = Vad;
    }

    /*----------------------------------------------------------------------*
    * - Find the weighted input speech w_sp[] for the whole speech frame   *
    * - Find the open-loop pitch delay                                     *
    *----------------------------------------------------------------------*/
    Copy(lsp_new_q, lsp_old_q, M);
    Lsp_prev_update(freq_cur, freq_prev);
   *ana++ = code_lsp[0];
    *ana++ = code_lsp[1];

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
    pAp  = A_t;     /* pointer to interpolated "unquantized"LPC parameters           */
    pAq = A_t_q;    /* pointer to interpolated "quantized" LPC parameters */

    i_gamma = 0;

    for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR) {

        /*---------------------------------------------------------------*
        * Find the weighted LPC coefficients for the weighting filter.  *
        *---------------------------------------------------------------*/
        Weight_Az(pAp, gamma1[i_gamma], M, Ap1);
        Weight_Az(pAp, gamma2[i_gamma], M, Ap2);

        /*---------------------------------------------------------------*
        * Compute impulse response, h1[], of weighted synthesis filter  *
        *---------------------------------------------------------------*/
        for (i = 0; i <=M; i++) ai_zero[i] = Ap1[i];
        Syn_filt(pAq, ai_zero, h1, L_SUBFR, zero, 0);
        Syn_filt(Ap2, h1, h1, L_SUBFR, zero, 0);

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
        Residu(pAq, &speech[i_subfr], &exc[i_subfr], L_SUBFR);   /* LPC residual */
        for (i=0; i<L_SUBFR; i++) res2[i] = exc[i_subfr+i];
        Syn_filt(pAq, &exc[i_subfr], error, L_SUBFR, mem_err, 0);
        Residu(Ap1, error, xn, L_SUBFR);
        Syn_filt(Ap2, xn, xn, L_SUBFR, mem_w0, 0);    /* target signal xn[]*/

        /*----------------------------------------------------------------------*
        *                 Closed-loop fractional pitch search                  *
        *----------------------------------------------------------------------*/
        T0 = Pitch_fr3D(&exc[i_subfr], xn, h1, L_SUBFR, T0_min, T0_max,
                               i_subfr, &T0_frac);

        index = Enc_lag3D(T0, T0_frac, &T0_min, &T0_max,PIT_MIN,PIT_MAX,
                            i_subfr);

        *ana++ = index;

        if ( (i_subfr == 0) && (rate != G729D) ) {
            *ana = Parity_Pitch(index);
            ana++;
        }
       /*-----------------------------------------------------------------*
        *   - find unity  gain pitch excitation (adaptive codebook entry)  *
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
        taming = test_err(T0, T0_frac);
        if( taming == 1){
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
        switch (rate) {

            case G729:    /* 8 kbit/s */
            {

             /* case 8 kbit/s */
                index = ACELP_CodebookD(xn2, h1, T0, sharp, i_subfr, code, y2, &i);
                *ana++ = index;        /* Positions index */
                *ana++ = i;            /* Signs index     */
                break;
            }

            case G729D:    /* 6.4 kbit/s */
            {
                index = ACELP_CodebookD(xn2, h1, T0, sharp, i_subfr, code, y2, &i);
                *ana++ = index;        /* Positions index */
                *ana++ = i;            /* Signs index     */
                break;
            }

            default : {
                printf("Unrecognized bit rate\n");
                exit(-1);
            }
        }  /* end of switch */

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

        if (rate == G729D)
            index = Qua_gain_6k(code, g_coeff_cs, exp_g_coeff_cs, L_SUBFR,
                &gain_pit, &gain_code, taming, past_qua_en);
        else
            index = Qua_gain_8k(code, g_coeff_cs, exp_g_coeff_cs, L_SUBFR,
                &gain_pit, &gain_code, taming, past_qua_en);

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

        Syn_filt(pAq, &exc[i_subfr], &synth[i_subfr], L_SUBFR,
                mem_syn, 1);

        for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++) {
            mem_err[j] = sub(speech[i_subfr+i], synth[i_subfr+i]);
            temp       = extract_h(L_shl( L_mult(y1[i], gain_pit),  1) );
            k          = extract_h(L_shl( L_mult(y2[i], gain_code), 2) );
            mem_w0[j]  = sub(xn[i], add(temp, k));
        }
        pAp   += MP1;
        pAq   += MP1;
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


