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
 File : COD_LD8K.C
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/

/*-----------------------------------------------------------------*
 *   Functions coder_ld8k and init_coder_ld8k                      *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *-----------------------------------------------------------------*/

#include "typedef.h"
#include "ld8k.h"

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

static FLOAT old_speech[L_TOTAL];
static FLOAT *speech, *p_window;
FLOAT  *new_speech;                       /* Global variable */

                /* Weighted speech vector */

static FLOAT old_wsp[L_FRAME+PIT_MAX];
static FLOAT *wsp;

                /* Excitation vector */

static FLOAT old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
static FLOAT *exc;

        /* Zero vector */

static FLOAT ai_zero[L_SUBFR+MP1];
static FLOAT *zero;


                /* Lsp (Line spectral pairs) */
static FLOAT lsp_old[M]=
     { (F)0.9595,  (F)0.8413,  (F)0.6549,  (F)0.4154,  (F)0.1423,
      (F)-0.1423, (F)-0.4154, (F)-0.6549, (F)-0.8413, (F)-0.9595};
static FLOAT lsp_old_q[M];

        /* Filter's memory */

static FLOAT mem_syn[M], mem_w0[M], mem_w[M];
static FLOAT mem_err[M+L_SUBFR], *error;

static FLOAT sharp;

/*----------------------------------------------------------------------------
 * init_coder_ld8k - initialization of variables for the encoder
 *----------------------------------------------------------------------------
 */
void init_coder_ld8k(void)
{
  /*-----------------------------------------------------------------------*
   *      Initialize pointers to speech vector.                            *
   *                                                                       *
   *                                                                       *
   *   |--------------------|-------------|-------------|------------|     *
   *     previous speech           sf1           sf2         L_NEXT        *
   *                                                                       *
   *   <----------------  Total speech vector (L_TOTAL)   ----------->     *
   *   |   <------------  LPC analysis window (L_WINDOW)  ----------->     *
   *   |   |               <-- present frame (L_FRAME) -->                 *
   * old_speech            |              <-- new speech (L_FRAME) -->     *
   *     p_wind            |              |                                *
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

  set_zero(old_speech, L_TOTAL);
  set_zero(old_exc, PIT_MAX+L_INTERPOL);
  set_zero(old_wsp, PIT_MAX);
  set_zero(mem_syn, M);
  set_zero(mem_w,   M);
  set_zero(mem_w0,  M);
  set_zero(mem_err, M);
  set_zero(zero, L_SUBFR);
  sharp = SHARPMIN;

  /* Initialize lsp_old_q[] */
  copy(lsp_old, lsp_old_q, M);

  lsp_encw_reset();
  init_exc_err();

 return;
}

/*----------------------------------------------------------------------------
 * coder_ld8k - encoder routine ( speech data should be in new_speech )
 *----------------------------------------------------------------------------
 */
void coder_ld8k(
 int ana[]             /* output: analysis parameters */
)
{
  /* LPC coefficients */
  FLOAT r[MP1];                /* Autocorrelations low and hi          */
  FLOAT A_t[(MP1)*2];          /* A(z) unquantized for the 2 subframes */
  FLOAT Aq_t[(MP1)*2];         /* A(z)   quantized for the 2 subframes */
  FLOAT Ap1[MP1];              /* A(z) with spectral expansion         */
  FLOAT Ap2[MP1];              /* A(z) with spectral expansion         */
  FLOAT *A, *Aq;               /* Pointer on A_t and Aq_t              */

  /* LSP coefficients */
  FLOAT lsp_new[M], lsp_new_q[M]; /* LSPs at 2th subframe                 */
  FLOAT lsf_int[M];               /* Interpolated LSF 1st subframe.       */
  FLOAT lsf_new[M];

  /* Variable added for adaptive gamma1 and gamma2 of the PWF */

  FLOAT rc[M];                        /* Reflection coefficients */
  FLOAT gamma1[2];             /* Gamma1 for 1st and 2nd subframes */
  FLOAT gamma2[2];             /* Gamma2 for 1st and 2nd subframes */

  /* Other vectors */
  FLOAT synth[L_FRAME];        /* Buffer for synthesis speech        */
  FLOAT h1[L_SUBFR];           /* Impulse response h1[]              */
  FLOAT xn[L_SUBFR];           /* Target vector for pitch search     */
  FLOAT xn2[L_SUBFR];          /* Target vector for codebook search  */
  FLOAT code[L_SUBFR];         /* Fixed codebook excitation          */
  FLOAT y1[L_SUBFR];           /* Filtered adaptive excitation       */
  FLOAT y2[L_SUBFR];           /* Filtered fixed codebook excitation */
  FLOAT g_coeff[5];            /* Correlations between xn, y1, & y2:
                                  <y1,y1>, <xn,y1>, <y2,y2>, <xn,y2>,<y1,y2>*/

  /* Scalars */

  int   i, j, i_gamma, i_subfr;
  int   T_op, t0, t0_min, t0_max, t0_frac;
  int   index, taming;
  FLOAT gain_pit, gain_code;

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

  autocorr(p_window, M, r);                     /* Autocorrelations */
  lag_window(M, r);                             /* Lag windowing    */
  levinson(r, &A_t[MP1], rc);                   /* Levinson Durbin  */
  az_lsp(&A_t[MP1], lsp_new, lsp_old);          /* From A(z) to lsp */

  /* LSP quantization */

  qua_lsp(lsp_new, lsp_new_q, ana);
  ana += 2;                         /* Advance analysis parameters pointer */

  /*--------------------------------------------------------------------*
   * Find interpolated LPC parameters in all subframes (both quantized  *
   * and unquantized).                                                  *
   * The interpolated parameters are in array A_t[] of size (M+1)*4     *
   * and the quantized interpolated parameters are in array Aq_t[]      *
   *--------------------------------------------------------------------*/

  int_lpc(lsp_old, lsp_new, lsf_int, lsf_new,  A_t);
  int_qlpc(lsp_old_q, lsp_new_q, Aq_t);

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
  * - Find the open-loop pitch delay for the whole speech frame          *
  * - Set the range for searching closed-loop pitch in 1st subframe      *
  *----------------------------------------------------------------------*/

  weight_az(&A_t[0], gamma1[0], M, Ap1);
  weight_az(&A_t[0], gamma2[0], M, Ap2);
  residu(Ap1, &speech[0], &wsp[0], L_SUBFR);
  syn_filt(Ap2, &wsp[0], &wsp[0], L_SUBFR, mem_w, 1);

  weight_az(&A_t[MP1], gamma1[1], M, Ap1);
  weight_az(&A_t[MP1], gamma2[1], M, Ap2);
  residu(Ap1, &speech[L_SUBFR], &wsp[L_SUBFR], L_SUBFR);
  syn_filt(Ap2, &wsp[L_SUBFR], &wsp[L_SUBFR], L_SUBFR, mem_w, 1);

  /* Find open loop pitch lag for whole speech frame */

  T_op = pitch_ol(wsp, PIT_MIN, PIT_MAX, L_FRAME);

  /* range for closed loop pitch search in 1st subframe */

  t0_min = T_op - 3;
  if (t0_min < PIT_MIN) t0_min = PIT_MIN;
  t0_max = t0_min + 6;
  if (t0_max > PIT_MAX)
    {
       t0_max = PIT_MAX;
       t0_min = t0_max - 6;
    }

 /*------------------------------------------------------------------------*
  *          Loop for every subframe in the analysis frame                 *
  *------------------------------------------------------------------------*
  *  To find the pitch and innovation parameters. The subframe size is     *
  *  L_SUBFR and the loop is repeated L_FRAME/L_SUBFR times.               *
  *     - find the weighted LPC coefficients                               *
  *     - find the LPC residual signal                                     *
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

    weight_az(A, gamma1[i_gamma], M, Ap1);
    weight_az(A, gamma2[i_gamma], M, Ap2);
    i_gamma++;

   /*---------------------------------------------------------------*
    * Compute impulse response, h1[], of weighted synthesis filter  *
    *---------------------------------------------------------------*/

    for (i = 0; i <= M; i++) ai_zero[i] = Ap1[i];
    syn_filt(Aq, ai_zero, h1, L_SUBFR, zero, 0);
    syn_filt(Ap2, h1, h1, L_SUBFR, zero, 0);

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


    residu(Aq, &speech[i_subfr], &exc[i_subfr], L_SUBFR);   /* LPC residual */

    syn_filt(Aq, &exc[i_subfr], error, L_SUBFR, mem_err, 0);

    residu(Ap1, error, xn, L_SUBFR);

    syn_filt(Ap2, xn, xn, L_SUBFR, mem_w0, 0);    /* target signal xn[]*/

   /*----------------------------------------------------------------------*
    *                 Closed-loop fractional pitch search                  *
    *----------------------------------------------------------------------*/

    t0 = pitch_fr3(&exc[i_subfr], xn, h1, L_SUBFR, t0_min, t0_max,
                              i_subfr, &t0_frac);


    index = enc_lag3(t0, t0_frac, &t0_min, &t0_max,PIT_MIN,PIT_MAX,i_subfr);

    *ana++ = index;
    if (i_subfr == 0)
      *ana++ = parity_pitch(index);


   /*-----------------------------------------------------------------*
    *   - find unity gain pitch excitation (adaptive codebook entry)  *
    *     with fractional interpolation.                              *
    *   - find filtered pitch exc. y1[]=exc[] convolve with h1[])     *
    *   - compute pitch gain and limit between 0 and 1.2              *
    *   - update target vector for codebook search                    *
    *   - find LTP residual.                                          *
    *-----------------------------------------------------------------*/

    pred_lt_3(&exc[i_subfr], t0, t0_frac, L_SUBFR);

    convolve(&exc[i_subfr], h1, y1, L_SUBFR);

    gain_pit = g_pitch(xn, y1, g_coeff, L_SUBFR);

    /* clip pitch gain if taming is necessary */
    taming = test_err(t0, t0_frac);

    if( taming == 1){
      if ( gain_pit>  GPCLIP) {
        gain_pit = GPCLIP;
      }
    }

    for (i = 0; i < L_SUBFR; i++)
       xn2[i] = xn[i] - y1[i]*gain_pit;

   /*-----------------------------------------------------*
    * - Innovative codebook search.                       *
    *-----------------------------------------------------*/

    index = ACELP_codebook(xn2, h1, t0, sharp, i_subfr, code, y2, &i);
    *ana++ = index;        /* Positions index */
    *ana++ = i;            /* Signs index     */


   /*-----------------------------------------------------*
    * - Quantization of gains.                            *
    *-----------------------------------------------------*/
    corr_xy2(xn, y1, y2, g_coeff);

    *ana++ =qua_gain(code, g_coeff, L_SUBFR, &gain_pit, &gain_code, taming );

   /*------------------------------------------------------------*
    * - Update pitch sharpening "sharp" with quantized gain_pit  *
    *------------------------------------------------------------*/

    sharp = gain_pit;
    if (sharp > SHARPMAX) sharp = SHARPMAX;
    if (sharp < SHARPMIN) sharp = SHARPMIN;
    /*------------------------------------------------------*
     * - Find the total excitation                          *
     * - find synthesis speech corresponding to exc[]       *
     * - update filters' memories for finding the target    *
     *   vector in the next subframe                        *
     *   (update error[-m..-1] and mem_w0[])                *
     *   update error function for taming process           *
     *------------------------------------------------------*/

    for (i = 0; i < L_SUBFR;  i++)
      exc[i+i_subfr] = gain_pit*exc[i+i_subfr] + gain_code*code[i];

    update_exc_err(gain_pit, t0);

    syn_filt(Aq, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 1);

    for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++)
      {
         mem_err[j] = speech[i_subfr+i] - synth[i_subfr+i];
         mem_w0[j]  = xn[i] - gain_pit*y1[i] - gain_code*y2[i];
      }
    A  += MP1;      /* interpolated LPC parameters for next subframe */
    Aq += MP1;

  }

  /*--------------------------------------------------*
   * Update signal for next frame.                    *
   * -> shift to the left by L_FRAME:                 *
   *     speech[], wsp[] and  exc[]                   *
   *--------------------------------------------------*/

  copy(&old_speech[L_FRAME], &old_speech[0], L_TOTAL-L_FRAME);
  copy(&old_wsp[L_FRAME], &old_wsp[0], PIT_MAX);
  copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);


  return;
}

