/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729 Annex A
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
 File : COD_LD8A.C
 Used for the floating point version of G.729A only
 (not for G.729 main body)
*/

/*-----------------------------------------------------------------*
 *   Functions coder_ld8a and init_coder_ld8a                      *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *                                                                 *
 *  init_coder_ld8a(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *                                                                 *
 *                                                                 *
 *  coder_ld8a(Short ana[]);                                       *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

#include <math.h>
#include "typedef.h"
#include "ld8a.h"


/*-----------------------------------------------------------*
 *    Coder constant parameters (defined in "ld8a.h")        *
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
 FLOAT *new_speech;                    /* Global variable */

        /* Weighted speech vector */

 static FLOAT old_wsp[L_FRAME+PIT_MAX];
 static FLOAT *wsp;

        /* Excitation vector */

 static FLOAT old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
 static FLOAT *exc;

        /* LSP Line spectral frequencies */

 static FLOAT lsp_old[M] =
     { (F)0.9595,  (F)0.8413,  (F)0.6549,  (F)0.4154,  (F)0.1423,
      (F)-0.1423, (F)-0.4154, (F)-0.6549, (F)-0.8413, (F)-0.9595};

 static FLOAT lsp_old_q[M];

        /* Filter's memory */

 static FLOAT  mem_w0[M], mem_w[M], mem_zero[M];
 static FLOAT  sharp;

/*-----------------------------------------------------------------*
 *   Function  init_coder_ld8a                                     *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *  init_coder_ld8a(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *       - initialize pointers to speech buffer                    *
 *       - initialize static  pointers                             *
 *       - set static vectors to zero                              *
 *-----------------------------------------------------------------*/

void init_coder_ld8a(void)
{

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

   /* Static vectors to zero */

   set_zero(old_speech, L_TOTAL);
   set_zero(old_exc, PIT_MAX+L_INTERPOL);
   set_zero(old_wsp, PIT_MAX);
   set_zero(mem_w,   M);
   set_zero(mem_w0,  M);
   set_zero(mem_zero, M);
   sharp = SHARPMIN;

   copy(lsp_old, lsp_old_q, M);

   lsp_encw_reset() ;
   init_exc_err();

   return;
}

/*-----------------------------------------------------------------*
 *   Function coder_ld8a                                           *
 *            ~~~~~~~~~~                                           *
 *  coder_ld8a(int    ana[]);                                      *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

void coder_ld8a(
 int ana[]                   /* output: analysis parameters */
)
{
   /* LPC coefficients */

   FLOAT Aq_t[(MP1)*2];         /* A(z) quantized for the 2 subframes   */
   FLOAT Ap_t[(MP1)*2];         /* A(z) with spectral expansion         */
   FLOAT *Aq, *Ap;              /* Pointer on Aq_t  and Ap_t            */

   /* Other vectors */

   FLOAT h1[L_SUBFR];           /* Impulse response h1[]              */
   FLOAT xn[L_SUBFR];           /* Target vector for pitch search     */
   FLOAT xn2[L_SUBFR];          /* Target vector for codebook search  */
   FLOAT code[L_SUBFR];         /* Fixed codebook excitation          */
   FLOAT y1[L_SUBFR];           /* Filtered adaptive excitation       */
   FLOAT y2[L_SUBFR];           /* Filtered fixed codebook excitation */
   FLOAT g_coeff[5];            /* Correlations between xn, y1, & y2:
                                   <y1,y1>, <xn,y1>, <y2,y2>, <xn,y2>,<y1,y2>*/

   /* Scalars */

   int i, j, i_subfr;
   int T_op, T0, T0_min, T0_max, T0_frac;
   int index;
   FLOAT   gain_pit, gain_code;
   int     taming;

/*------------------------------------------------------------------------*
 *  - Perform LPC analysis:                                               *
 *       * autocorrelation + lag windowing                                *
 *       * Levinson-durbin algorithm to find a[]                          *
 *       * convert a[] to lsp[]                                           *
 *       * quantize and code the LSPs                                     *
 *       * find the interpolated LSPs and convert to a[] for the 2        *
 *         subframes (both quantized and unquantized)                     *
 *------------------------------------------------------------------------*/
   {
      /* Temporary vectors */
     FLOAT r[MP1];                    /* Autocorrelations       */
     FLOAT rc[M];                     /* Reflexion coefficients */
     FLOAT lsp_new[M];                /* lsp coefficients       */
     FLOAT lsp_new_q[M];              /* Quantized lsp coeff.   */

     /* LP analysis */

     autocorr(p_window, M, r);             /* Autocorrelations */
     lag_window(M, r);                     /* Lag windowing    */
     levinson(r, Ap_t, rc);                /* Levinson Durbin  */
     az_lsp(Ap_t, lsp_new, lsp_old);       /* Convert A(z) to lsp */

     /* LSP quantization */

     qua_lsp(lsp_new, lsp_new_q, ana);
     ana += 2;                        /* Advance analysis parameters pointer */

    /*--------------------------------------------------------------------*
     * Find interpolated LPC parameters in all subframes                  *
     * The interpolated parameters are in array Aq_t[].                   *
     *--------------------------------------------------------------------*/

    int_qlpc(lsp_old_q, lsp_new_q, Aq_t);

    /* Compute A(z/gamma) */

    weight_az(&Aq_t[0],   GAMMA1, M, &Ap_t[0]);
    weight_az(&Aq_t[MP1], GAMMA1, M, &Ap_t[MP1]);

    /* update the LSPs for the next frame */

    copy(lsp_new,   lsp_old,   M);
    copy(lsp_new_q, lsp_old_q, M);
  }

   /*----------------------------------------------------------------------*
    * - Find the weighted input speech w_sp[] for the whole speech frame   *
    * - Find the open-loop pitch delay for the whole speech frame          *
    * - Set the range for searching closed-loop pitch in 1st subframe      *
    *----------------------------------------------------------------------*/

   residu(&Aq_t[0],   &speech[0],       &exc[0],       L_SUBFR);
   residu(&Aq_t[MP1], &speech[L_SUBFR], &exc[L_SUBFR], L_SUBFR);

  {
     FLOAT Ap1[MP1];

     Ap = Ap_t;
     Ap1[0] = (F)1.0;
     for(i=1; i<=M; i++)
       Ap1[i] = Ap[i] - (F)0.7 * Ap[i-1];
     syn_filt(Ap1, &exc[0], &wsp[0], L_SUBFR, mem_w, 1);

     Ap += MP1;
     for(i=1; i<=M; i++)
       Ap1[i] = Ap[i] - (F)0.7 * Ap[i-1];
     syn_filt(Ap1, &exc[L_SUBFR], &wsp[L_SUBFR], L_SUBFR, mem_w, 1);
   }


   /* Find open loop pitch lag for whole speech frame */

   T_op = pitch_ol_fast(wsp, L_FRAME);

   /* Range for closed loop pitch search in 1st subframe */

   T0_min = T_op - 3;
   if (T0_min < PIT_MIN) T0_min = PIT_MIN;
   T0_max = T0_min + 6;
   if (T0_max > PIT_MAX)
   {
      T0_max = PIT_MAX;
      T0_min = T0_max - 6;
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
    *     - find target vector for codebook search                           *
    *     - codebook search                                                  *
    *     - VQ of pitch and codebook gains                                   *
    *     - update states of weighting filter                                *
    *------------------------------------------------------------------------*/

   Aq = Aq_t;    /* pointer to interpolated quantized LPC parameters */
   Ap = Ap_t;    /* pointer to weighted LPC coefficients             */

   for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR)
   {
      /*---------------------------------------------------------------*
       * Compute impulse response, h1[], of weighted synthesis filter  *
       *---------------------------------------------------------------*/

      h1[0] = (F)1.0;
      set_zero(&h1[1], L_SUBFR-1);
      syn_filt(Ap, h1, h1, L_SUBFR, &h1[1], 0);

      /*-----------------------------------------------*
       * Find the target vector for pitch search:      *
       *----------------------------------------------*/

      syn_filt(Ap, &exc[i_subfr], xn, L_SUBFR, mem_w0, 0);

      /*-----------------------------------------------------------------*
       *    Closed-loop fractional pitch search                          *
       *-----------------------------------------------------------------*/

      T0 = pitch_fr3_fast(&exc[i_subfr], xn, h1, L_SUBFR, T0_min, T0_max,
                    i_subfr, &T0_frac);

      index = enc_lag3(T0, T0_frac, &T0_min, &T0_max, PIT_MIN, PIT_MAX,
                            i_subfr);

      *ana++ = index;

      if (i_subfr == 0)
        *ana++ = parity_pitch(index);

      /*-----------------------------------------------------------------*
       *   - find filtered pitch exc                                     *
       *   - compute pitch gain and limit between 0 and 1.2              *
       *   - update target vector for codebook search                    *
       *   - find LTP residual.                                          *
       *-----------------------------------------------------------------*/

      syn_filt(Ap, &exc[i_subfr], y1, L_SUBFR, mem_zero, 0);

      gain_pit = g_pitch(xn, y1, g_coeff, L_SUBFR);

      /* clip pitch gain if taming is necessary */

      taming = test_err(T0, T0_frac);

      if( taming == 1){
        if (gain_pit > GPCLIP) {
          gain_pit = GPCLIP;
      }
    }

    for (i = 0; i < L_SUBFR; i++)
        xn2[i] = xn[i] - y1[i]*gain_pit;

      /*-----------------------------------------------------*
       * - Innovative codebook search.                       *
       *-----------------------------------------------------*/

      index = ACELP_code_A(xn2, h1, T0, sharp, code, y2, &i);

      *ana++ = index;           /* Positions index */
      *ana++ = i;               /* Signs index     */

      /*------------------------------------------------------*
       *  - Compute the correlations <y2,y2>, <xn,y2>, <y1,y2>*
       *  - Vector quantize gains.                            *
       *------------------------------------------------------*/

      corr_xy2(xn, y1, y2, g_coeff);
       *ana++ =qua_gain(code, g_coeff, L_SUBFR, &gain_pit, &gain_code,
                                    taming);

      /*------------------------------------------------------------*
       * - Update pitch sharpening "sharp" with quantized gain_pit  *
       *------------------------------------------------------------*/

      sharp = gain_pit;
      if (sharp > SHARPMAX) sharp = SHARPMAX;
      if (sharp < SHARPMIN) sharp = SHARPMIN;

      /*------------------------------------------------------*
       * - Find the total excitation                          *
       * - update filters' memories for finding the target    *
       *   vector in the next subframe  (mem_w0[])            *
       *------------------------------------------------------*/

      for (i = 0; i < L_SUBFR;  i++)
        exc[i+i_subfr] = gain_pit*exc[i+i_subfr] + gain_code*code[i];

      update_exc_err(gain_pit, T0);

      for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++)
        mem_w0[j]  = xn[i] - gain_pit*y1[i] - gain_code*y2[i];

      Aq += MP1;           /* interpolated LPC parameters for next subframe */
      Ap += MP1;
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
