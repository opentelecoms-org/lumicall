/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX D   Version 1.3    Last modified: Oct 2000 */

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8kD and Init_Coder_ld8k                     *
 *             ~~~~~~~~~~~     ~~~~~~~~~~~~~~~                     *
 *-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "tabld8kd.h"
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

 /* to tame the coder */
 static Word32 L_exc_err[4];

 static Word16 past_qua_en[4] = { -14336, -14336, -14336, -14336 };

/*-----------------------------------------------------------------*
 *   Function  Init_Coder_ld8k                                     *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *  Init_Coder_ld8k(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *       - initialize pointers to speech buffer                    *
 *       - initialize static  pointers                             *
 *       - set static vectors to zero                              *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Init_Coder_ld8k(void)
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

  for(i=0; i<4; i++) L_exc_err[i] = 0x00004000L;   /* Q14 */

 return;
}

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8kD                                         *
 *             ~~~~~~~~~~~                                         *
 *  Coder_ld8kD(Word16 ana[], Word16 synth[]);                     *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *   80 speech data should have been copied to vector new_speech[].*
 *   This vector is global and is declared in this function.       *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *    synth[]    ->Local synthesis (for debug purpose)             *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Coder_ld8kD(
     Word16 ana[],      /* output  : Analysis parameters */
     Word16 synth[]     /* output  : Local synthesis     */
)
{

  /* LPC analysis */

  Word16 r_l[MP1], r_h[MP1];       /* Autocorrelations low and hi          */
  Word16 rc[M];                    /* Reflection coefficients.             */
  Word16 A_t[(MP1)*2];             /* A(z) unquantized for the 2 subframes */
  Word16 Aq_t[(MP1)*2];            /* A(z)   quantized for the 2 subframes */
  Word16 Ap1[MP1];                 /* A(z) with spectral expansion         */
  Word16 Ap2[MP1];                 /* A(z) with spectral expansion         */
  Word16 *A, *Aq;                  /* Pointer on A_t and Aq_t              */
  Word16 lsp_new[M], lsp_new_q[M]; /* LSPs at 2th subframe                 */
  Word16 lsf_int[M];               /* Interpolated LSF 1st subframe.       */
  Word16 lsf_new[M];
  Word16 gamma1[2], gamma2[2];     /* Weighting factor for the 2 subframes */

  /* Other vectors */

  Word16 h1[L_SUBFR];            /* Impulse response h1[]              */
  Word16 xn[L_SUBFR];            /* Target vector for pitch search     */
  Word16 xn2[L_SUBFR];           /* Target vector for codebook search  */
  Word16 code[L_SUBFR];          /* Fixed codebook excitation          */
  Word16 y1[L_SUBFR];            /* Filtered adaptive excitation       */
  Word16 y2[L_SUBFR];            /* Filtered fixed codebook excitation */
  Word16 g_coeff[4];             /* Correlations between xn & y1       */

  Word16 g_coeff_cs[5];
  Word16 exp_g_coeff_cs[5];      /* Correlations between xn, y1, & y2
                                     <y1,y1>, -2<xn,y1>,
                                          <y2,y2>, -2<xn,y2>, 2<y1,y2> */
  /* Scalars */

  Word16 i, j, k, i_subfr, i_gamma;
  Word16 T_op, T0, T0_min, T0_max, T0_frac;
  Word16 gain_pit, gain_code, index;
  Word16 temp;
  Word32 L_temp;

/*------------------------------------------------------------------------*
 *  - Perform LPC analysis:                                               *
 *       * autocorrelation + lag windowing                                *
 *       * Levinson-durbin algorithm to find a[]                          *
 *       * convert a[] to lsp[]                                           *
 *       * quantize and code the LSPs                                     *
 *       * find the interpolated LSPs and convert to a[] for the 2        *
 *         subframes (both quantized and unquantized)                     *
 *------------------------------------------------------------------------*/

  /* LP analysis */

  Autocorr(p_window, M, r_h, r_l);              /* Autocorrelations */
  Lag_window(M, r_h, r_l);                      /* Lag windowing    */
  Levinson(r_h, r_l, &A_t[MP1],rc);             /* Levinson Durbin  */
  Az_lsp(&A_t[MP1], lsp_new, lsp_old);          /* From A(z) to lsp */

  /* LSP quantization */

  Qua_lsp(lsp_new, lsp_new_q, ana);
  ana += 2;                         /* Advance analysis parameters pointer */

  /*--------------------------------------------------------------------*
   * Find interpolated LPC parameters in all subframes (both quantized  *
   * and unquantized).                                                  *
   * The interpolated parameters are in array A_t[] of size (M+1)*4     *
   * and the quantized interpolated parameters are in array Aq_t[]      *
   *--------------------------------------------------------------------*/

  Int_lpc(lsp_old, lsp_new, lsf_int, lsf_new,  A_t);
  Int_qlpc(lsp_old_q, lsp_new_q, Aq_t);

  /* update the LSPs for the next frame */

  for(i=0; i<M; i++)
  {
    lsp_old[i]   = lsp_new[i];
    lsp_old_q[i] = lsp_new_q[i];
  }

 /*----------------------------------------------------------------------*
  * - Find the weighting factors                                         *
  *----------------------------------------------------------------------*/

  perc_var(gamma1, gamma2, lsf_int, lsf_new, rc);

 /*----------------------------------------------------------------------*
  * - Find the weighted input speech w_sp[] for the whole speech frame   *
  * - Find the open-loop pitch delay                                     *
  *----------------------------------------------------------------------*/

  Weight_Az(&A_t[0], gamma1[0], M, Ap1);
  Weight_Az(&A_t[0], gamma2[0], M, Ap2);
  Residu(Ap1, &speech[0], &wsp[0], L_SUBFR);
  Syn_filt(Ap2, &wsp[0], &wsp[0], L_SUBFR, mem_w, 1);

  Weight_Az(&A_t[MP1], gamma1[1], M, Ap1);
  Weight_Az(&A_t[MP1], gamma2[1], M, Ap2);
  Residu(Ap1, &speech[L_SUBFR], &wsp[L_SUBFR], L_SUBFR);
  Syn_filt(Ap2, &wsp[L_SUBFR], &wsp[L_SUBFR], L_SUBFR, mem_w, 1);

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

  A  = A_t;     /* pointer to interpolated LPC parameters           */
  Aq = Aq_t;    /* pointer to interpolated quantized LPC parameters */

  i_gamma = 0;
  for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR)
  {

    /*---------------------------------------------------------------*
     * Find the weighted LPC coefficients for the weighting filter.  *
     *---------------------------------------------------------------*/

    Weight_Az(A, gamma1[i_gamma], M, Ap1);
    Weight_Az(A, gamma2[i_gamma], M, Ap2);
    i_gamma = add(i_gamma,1);

    /*---------------------------------------------------------------*
     * Compute impulse response, h1[], of weighted synthesis filter  *
     *---------------------------------------------------------------*/

    for (i = 0; i <= M; i++) {
        ai_zero[i] = Ap1[i];
    }

    Syn_filt(Aq, ai_zero, h1, L_SUBFR, zero, 0);
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


    Residu(Aq, &speech[i_subfr], &exc[i_subfr], L_SUBFR);   /* LPC residual */

    Syn_filt(Aq, &exc[i_subfr], error, L_SUBFR, mem_err, 0);

    Residu(Ap1, error, xn, L_SUBFR);

    Syn_filt(Ap2, xn, xn, L_SUBFR, mem_w0, 0);    /* target signal xn[]*/

    /*----------------------------------------------------------------------*
     *                 Closed-loop fractional pitch search                  *
     *----------------------------------------------------------------------*/

    T0 = Pitch_fr3D(&exc[i_subfr], xn, h1, L_SUBFR, T0_min, T0_max,
                               i_subfr, &T0_frac);

    index = Enc_lag3D(T0, T0_frac, &T0_min, &T0_max,PIT_MIN,PIT_MAX,i_subfr);

    *ana++ = index;
    if (i_subfr == 0) {
       if (sub(CODEC_MODE, 2) == 0) {
          *ana++ = Parity_Pitch(index);
       }
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

    for (i = 0; i < L_SUBFR; i++)
    {
      L_temp = L_mult(y1[i], gain_pit);
      L_temp = L_shl(L_temp, 1);               /* gain_pit in Q14 */
      xn2[i] = sub(xn[i], extract_h(L_temp));
    }


   /*-----------------------------------------------------*
    * - Innovative codebook search.                       *
    *-----------------------------------------------------*/

    index = ACELP_CodebookD(xn2, h1, T0, sharp, i_subfr, code, y2, &i);

    *ana++ = index;        /* Positions index */
    *ana++ = i;            /* Signs index     */


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

    if (sub(CODEC_MODE, 1) == 0) {
        *ana++ = Qua_gain_6k(code, g_coeff_cs, exp_g_coeff_cs,
                          L_SUBFR, &gain_pit, &gain_code, temp, past_qua_en);
    }
    else if (sub(CODEC_MODE, 2) == 0) {
        *ana++ = Qua_gain_8k(code, g_coeff_cs, exp_g_coeff_cs,
                          L_SUBFR, &gain_pit, &gain_code, temp, past_qua_en);
    }
    else {
        fprintf(stderr, "CODEC mode invalid\n");
        exit(-1);
    }
    
   /*------------------------------------------------------------*
    * - Update pitch sharpening "sharp" with quantized gain_pit  *
    *------------------------------------------------------------*/

    sharp = gain_pit;
    if (sub(sharp, SHARPMAX) > 0) { sharp = SHARPMAX;         }
    if (sub(sharp, SHARPMIN) < 0) { sharp = SHARPMIN;         }

   /*------------------------------------------------------*
    * - Find the total excitation                          *
    * - find synthesis speech corresponding to exc[]       *
    * - update filters memories for finding the target     *
    *   vector in the next subframe                        *
    *   (update error[-m..-1] and mem_w_err[])             *
    *   update error function for taming process           *
    *------------------------------------------------------*/

    for (i = 0; i < L_SUBFR;  i++)
    {
      /* exc[i] = gain_pit*exc[i] + gain_code*code[i]; */
      /* exc[i]  in Q0   gain_pit in Q14               */
      /* code[i] in Q13  gain_cod in Q1                */

      L_temp = L_mult(exc[i+i_subfr], gain_pit);
      L_temp = L_mac(L_temp, code[i], gain_code);
      L_temp = L_shl(L_temp, 1);
      exc[i+i_subfr] = round(L_temp);
    }

    update_exc_err(gain_pit, T0);

    Syn_filt(Aq, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 1);

    for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++)
    {
      mem_err[j] = sub(speech[i_subfr+i], synth[i_subfr+i]);
      temp       = extract_h(L_shl( L_mult(y1[i], gain_pit),  1) );
      k          = extract_h(L_shl( L_mult(y2[i], gain_code), 2) );
      mem_w0[j]  = sub(xn[i], add(temp, k));
    }

    A  += MP1;           /* interpolated LPC parameters for next subframe */
    Aq += MP1;

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

