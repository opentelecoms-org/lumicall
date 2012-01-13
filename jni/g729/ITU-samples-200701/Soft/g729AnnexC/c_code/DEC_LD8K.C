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
 File : DEC_LD8K.C
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/


/*-----------------------------------------------------------------*
 *   Functions init_decod_ld8k  and decod_ld8k                     *
 *-----------------------------------------------------------------*/

#include "typedef.h"
#include "ld8k.h"

/*---------------------------------------------------------------*
 *   Decoder constant parameters (defined in "ld8k.h")           *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   PIT_MIN     : Minimum pitch lag.                            *
 *   PIT_MAX     : Maximum pitch lag.                            *
 *   L_INTERPOL  : Length of filter for interpolation            *
 *   PRM_SIZE    : Size of vector containing analysis parameters *
 *---------------------------------------------------------------*/

/*--------------------------------------------------------*
 *         Static memory allocation.                      *
 *--------------------------------------------------------*/

        /* Excitation vector */

static FLOAT old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
static FLOAT *exc;

        /* Lsp (Line spectral pairs) */

 static FLOAT lsp_old[M]={
       (F)0.9595,  (F)0.8413,  (F)0.6549,  (F)0.4154,  (F)0.1423,
      (F)-0.1423, (F)-0.4154, (F)-0.6549, (F)-0.8413, (F)-0.9595};

        /* Filter's memory */
static FLOAT mem_syn[M];        /* Filter's memory */

static FLOAT sharp ;            /* pitch sharpening of previous fr */
static int old_t0;              /* integer delay of previous frame */
static FLOAT gain_code;         /* fixed codebook gain */
static FLOAT gain_pitch ;       /* adaptive codebook gain */

/*--------------------------------------------------------------------------
 * init_decod_ld8k - Initialization of variables for the decoder section.
 *--------------------------------------------------------------------------
 */
void init_decod_ld8k(void)
{
    /* Initialize static pointer */
    exc    = old_exc + PIT_MAX + L_INTERPOL;

    /* Static vectors to zero */
    set_zero(old_exc,PIT_MAX + L_INTERPOL);
    set_zero(mem_syn, M);

    sharp = SHARPMIN;
    old_t0 = 60;
    gain_code = (F)0.;
    gain_pitch = (F)0.;

    lsp_decw_reset();

    return;
}

/*--------------------------------------------------------------------------
 * decod_ld8k - decoder
 *--------------------------------------------------------------------------
 */
void decod_ld8k(
 int parm[],            /* input : synthesis parameters (parm[0] = bfi)       */
 int voicing,           /* input : voicing decision from previous frame       */
 FLOAT synth[],         /* output: synthesized speech                         */
 FLOAT A_t[],           /* output: two sets of A(z) coefficients length=2*MP1 */
 int *t0_first          /* output: integer delay of first subframe            */
)
{
   FLOAT *Az;                  /* Pointer to A_t (LPC coefficients)  */
   FLOAT lsp_new[M];           /* LSPs                               */
   FLOAT code[L_SUBFR];        /* algebraic codevector               */

  /* Scalars */
  int   i, i_subfr;
  int   t0, t0_frac, index;

  int bfi;
  int bad_pitch;

  /* Test bad frame indicator (bfi) */

  bfi = *parm++;

  /* Decode the LSPs */

  d_lsp(parm, lsp_new, bfi);
  parm += 2;             /* Advance synthesis parameters pointer */

  /* Interpolation of LPC for the 2 subframes */

  int_qlpc(lsp_old, lsp_new, A_t);

  /* update the LSFs for the next frame */

  copy(lsp_new, lsp_old, M);

/*------------------------------------------------------------------------*
 *          Loop for every subframe in the analysis frame                 *
 *------------------------------------------------------------------------*
 * The subframe size is L_SUBFR and the loop is repeated L_FRAME/L_SUBFR  *
 *  times                                                                 *
 *     - decode the pitch delay                                           *
 *     - decode algebraic code                                            *
 *     - decode pitch and codebook gains                                  *
 *     - find the excitation and compute synthesis speech                 *
 *------------------------------------------------------------------------*/

  Az = A_t;            /* pointer to interpolated LPC parameters */

  for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {

   index = *parm++;          /* pitch index */

   if (i_subfr == 0) {      /* if first subframe */
     i = *parm++;             /* get parity check result */
     bad_pitch = bfi+ i;
     if( bad_pitch == 0)
     {
       dec_lag3(index, PIT_MIN, PIT_MAX, i_subfr, &t0, &t0_frac);
       old_t0 = t0;
     }
     else                     /* Bad frame, or parity error */
     {
       t0  =  old_t0;
       t0_frac = 0;
       old_t0++;
       if( old_t0> PIT_MAX) {
           old_t0 = PIT_MAX;
       }
     }
      *t0_first = t0;         /* If first frame */
   }
   else                       /* second subframe */
   {
     if( bfi == 0)
     {
       dec_lag3(index, PIT_MIN, PIT_MAX, i_subfr, &t0, &t0_frac);
       old_t0 = t0;
     }
     else
     {
       t0  =  old_t0;
       t0_frac = 0;
       old_t0++;
       if( old_t0 >PIT_MAX) {
           old_t0 = PIT_MAX;
       }
     }
   }


   /*-------------------------------------------------*
    *  - Find the adaptive codebook vector.            *
    *--------------------------------------------------*/

   pred_lt_3(&exc[i_subfr], t0, t0_frac, L_SUBFR);

   /*-------------------------------------------------------*
    * - Decode innovative codebook.                         *
    * - Add the fixed-gain pitch contribution to code[].    *
    *-------------------------------------------------------*/

   if(bfi != 0) {            /* Bad Frame Error Concealment */
     parm[0] = (int) (random_g729() & 0x1fff);      /* 13 bits random*/
     parm[1]= (int) (random_g729() & 0x000f);      /*  4 bits random */
   }

   decod_ACELP(parm[1], parm[0], code);
   parm +=2;
   for (i = t0; i < L_SUBFR; i++)   code[i] += sharp * code[i-t0];

   /*-------------------------------------------------*
    * - Decode pitch and codebook gains.              *
    *-------------------------------------------------*/

   index = *parm++;          /* index of energy VQ */
   dec_gain(index, code, L_SUBFR, bfi, &gain_pitch, &gain_code);

   /*-------------------------------------------------------------*
    * - Update pitch sharpening "sharp" with quantized gain_pitch *
    *-------------------------------------------------------------*/

   sharp = gain_pitch;
   if (sharp > SHARPMAX) sharp = SHARPMAX;
   if (sharp < SHARPMIN) sharp = SHARPMIN;

   /*-------------------------------------------------------*
    * - Find the total excitation.                          *
    *-------------------------------------------------------*/

   if(bfi != 0 ) {
     if(voicing  == 0) {     /* for unvoiced frame */
         for (i = 0; i < L_SUBFR;  i++) {
            exc[i+i_subfr] = gain_code*code[i];
         }
      } else {               /* for voiced frame */
         for (i = 0; i < L_SUBFR;  i++) {
            exc[i+i_subfr] = gain_pitch*exc[i+i_subfr];
         }
      }
    } else {                  /* No frame errors */
      for (i = 0; i < L_SUBFR;  i++) {
         exc[i+i_subfr] = gain_pitch*exc[i+i_subfr] + gain_code*code[i];
      }
    }

    /*-------------------------------------------------------*
     * - Find synthesis speech corresponding to exc[].       *
     *-------------------------------------------------------*/

    syn_filt(Az, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 1);

    Az  += MP1;        /* interpolated LPC parameters for next subframe */
  }

   /*--------------------------------------------------*
    * Update signal for next frame.                    *
    * -> shift to the left by L_FRAME  exc[]           *
    *--------------------------------------------------*/
  copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);

   return;
}
