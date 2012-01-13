/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX D   Version 1.3    Last modified: Oct 2000 */

/*-----------------------------------------------------------------*
 *   Functions Init_Decod_ld8k  and Decod_ld8kD                    *
 *-----------------------------------------------------------------*/

#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "tabld8kd.h"

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

 static Word16 old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
 static Word16 *exc;

        /* Lsp (Line spectral pairs) */

 static Word16 lsp_old[M]={
             30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000};

        /* Filter's memory */

 static Word16 mem_syn[M];

 static Word16 sharp;           /* pitch sharpening of previous frame */
 static Word16 old_T0;          /* integer delay of previous frame    */
 static Word16 gain_code;       /* Code gain                          */
 static Word16 gain_pitch;      /* Pitch gain                         */

 static Word16 past_qua_en[4] = { -14336, -14336, -14336, -14336 };

/*-----------------------------------------------------------------*
 *   Function Init_Decod_ld8k                                      *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *   ->Initialization of variables for the decoder section.        *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Init_Decod_ld8k(void)
{

  /* Initialize static pointer */

  exc = old_exc + PIT_MAX + L_INTERPOL;

  /* Static vectors to zero */

  Set_zero(old_exc, PIT_MAX+L_INTERPOL);
  Set_zero(mem_syn, M);

  sharp  = SHARPMIN;
  old_T0 = 60;
  gain_code = 0;
  gain_pitch = 0;

  Lsp_decw_reset();
  return;
}

/*-----------------------------------------------------------------*
 *   Function Decod_ld8kD                                          *
 *           ~~~~~~~~~~                                            *
 *   ->Main decoder routine.                                       *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Decod_ld8kD(
  Word16  parm[],      /* (i)   : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)  */
  Word16  voicing,     /* (i)   : voicing decision from previous frame */
  Word16  synth[],     /* (o)   : synthesis speech                     */
  Word16  A_t[],       /* (o)   : decoded LP filter in 2 subframes     */
  Word16  *T0_first    /* (o)   : decoded pitch lag in first subframe  */
)
{
  Word16  *Az;                  /* Pointer on A_t   */
  Word16  lsp_new[M];           /* LSPs             */
  Word16  code[L_SUBFR];        /* ACELP codevector */

  /* Scalars */

  Word16  i, j, i_subfr;
  Word16  T0, T0_frac, index;
  Word16  bfi;
  Word32  L_temp;
  Word16 g_p, g_c;              /* fixed and adaptive codebook gain */

  Word16 bad_pitch;             /* bad pitch indicator */
  extern Flag Overflow;

  /* Test bad frame indicator (bfi) */

  bfi = *parm++;

  /* Decode the LSPs */

  D_lsp(parm, lsp_new, bfi);
  parm += 2;

  /* Interpolation of LPC for the 2 subframes */

  Int_qlpc(lsp_old, lsp_new, A_t);

  /* update the LSFs for the next frame */

  Copy(lsp_new, lsp_old, M);

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

  for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
  {

    index = *parm++;            /* pitch index */

    if(i_subfr == 0)
    {
      if (sub(CODEC_MODE, 1) == 0) {
          i = 0;
      }
      else if (sub(CODEC_MODE, 2) == 0) {
          i = *parm++;             /* get parity check result */
      }
      else {
        fprintf(stderr, "CODEC mode invalid\n");
        exit(-1);
      }

      bad_pitch = add(bfi, i);
      if( bad_pitch == 0)
      {
        Dec_lag3D(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac);
        old_T0 = T0;
      }
      else                     /* Bad frame, or parity error */
      {
        T0  =  old_T0;
        T0_frac = 0;
        old_T0 = add( old_T0, 1);
        if( sub(old_T0, PIT_MAX) > 0) {
            old_T0 = PIT_MAX;
        }
      }
       *T0_first = T0;         /* If first frame */
    }
    else                       /* second subframe */
    {
      if( bfi == 0)
      {
        Dec_lag3D(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac);
        old_T0 = T0;
      }
      else
      {
        T0  =  old_T0;
        T0_frac = 0;
        old_T0 = add( old_T0, 1);
        if( sub(old_T0, PIT_MAX) > 0) {
            old_T0 = PIT_MAX;
        }
      }
    }

   /*-------------------------------------------------*
    * - Find the adaptive codebook vector.            *
    *-------------------------------------------------*/

    Pred_lt_3(&exc[i_subfr], T0, T0_frac, L_SUBFR);

   /*-------------------------------------------------------*
    * - Decode innovative codebook.                         *
    * - Add the fixed-gain pitch contribution to code[].    *
    *-------------------------------------------------------*/

    if (sub(CODEC_MODE, 1) == 0) {
      if(bfi != 0) {       /* Bad frame */
        parm[0] = Random() & (Word16)0x01ff;     /*  9 bits random */
        parm[1] = Random() & (Word16)0x0003;     /*  2 bits random */
      }
      Decod_ACELP64(parm[1], parm[0], code);
    }
    else if (sub(CODEC_MODE, 2) == 0) {
      if(bfi != 0) {       /* Bad frame */
        parm[0] = Random() & (Word16)0x1fff;     /* 13 bits random */
        parm[1] = Random() & (Word16)0x000f;     /*  4 bits random */
      }
      Decod_ACELP(parm[1], parm[0], code);
    }
    else {
      fprintf(stderr, "CODEC mode invalid\n");
      exit(-1);
    }

    parm +=2;

    j = shl(sharp, 1);          /* From Q14 to Q15 */
    if(sub(T0, L_SUBFR) <0 ) {
        for (i = T0; i < L_SUBFR; i++) {
          code[i] = add(code[i], mult(code[i-T0], j));
        }
    }

   /*-------------------------------------------------*
    * - Decode pitch and codebook gains.              *
    *-------------------------------------------------*/

    index = *parm++;      /* index of energy VQ */

    if (sub(CODEC_MODE, 1) == 0) {
        Dec_gain_6k(index, code, L_SUBFR, bfi, &gain_pitch, &gain_code, past_qua_en);
    }
    else if (sub(CODEC_MODE, 2) == 0) {
        Dec_gain_8k(index, code, L_SUBFR, bfi, &gain_pitch, &gain_code, past_qua_en);
    }
    else {
      fprintf(stderr, "CODEC mode invalid\n");
      exit(-1);
    }

   /*-------------------------------------------------------------*
    * - Update pitch sharpening "sharp" with quantized gain_pitch *
    *-------------------------------------------------------------*/

    sharp = gain_pitch;
    if (sub(sharp, SHARPMAX) > 0) { sharp = SHARPMAX;  }
    if (sub(sharp, SHARPMIN) < 0) { sharp = SHARPMIN;  }

   /*-------------------------------------------------------*
    * - Find the total excitation.                          *
    * - Find synthesis speech corresponding to exc[].       *
    *-------------------------------------------------------*/

    if(bfi != 0)        /* Bad frame */
    {
       if (voicing == 0 ) {
          g_p = 0;
          g_c = gain_code;
       } else {
          g_p = gain_pitch;
          g_c = 0;
       }
    } else {
       g_p = gain_pitch;
       g_c = gain_code;
    }
    for (i = 0; i < L_SUBFR;  i++)
    {
       /* exc[i] = g_p*exc[i] + g_c*code[i]; */
       /* exc[i]  in Q0   g_p in Q14         */
       /* code[i] in Q13  g_code in Q1       */

       L_temp = L_mult(exc[i+i_subfr], g_p);
       L_temp = L_mac(L_temp, code[i], g_c);
       L_temp = L_shl(L_temp, 1);
       exc[i+i_subfr] = round(L_temp);
    }

    /*-------------------------------------------------*
     * Updates state machine for phase dispersion in   *
     * 6.4 kbps mode, if running in 8.0 kbps mode.   *
     *-------------------------------------------------*/
    if (sub(CODEC_MODE, 2) == 0) {
      Update64(gain_pitch, gain_code);
    }

    /* Test whether synthesis yields overflow or not */
    Overflow = 0; 
    Syn_filt(Az, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 0);

    /* In case of overflow in the synthesis          */
    /* -> Scale down vector exc[] and redo synthesis */
    if(Overflow != 0)
    {
      for(i=0; i<PIT_MAX+L_INTERPOL+L_FRAME; i++)
        old_exc[i] = shr(old_exc[i], 2);
    }

    if (sub(CODEC_MODE, 1) == 0) {
      Syn_filt64(Az, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 0,
                  gain_code, gain_pitch, code );
    }
    else {
      Syn_filt(Az, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 0);
    }

    /* Update of synthesis filter memory */
    Copy(&synth[i_subfr+L_SUBFR-M], mem_syn, M); 

    Az += MP1;    /* interpolated LPC parameters for next subframe */
  }

 /*--------------------------------------------------*
  * Update signal for next frame.                    *
  * -> shift to the left by L_FRAME  exc[]           *
  *--------------------------------------------------*/

  Copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);

  return;
}

