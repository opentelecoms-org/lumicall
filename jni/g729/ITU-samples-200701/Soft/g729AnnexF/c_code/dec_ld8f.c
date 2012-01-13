/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex F  - Reference C code for fixed point
                         implementation of G.729 Annex F
                         (integration of Annexes B and D)
   Version 1.2    Last modified: October 2006 
*/

/*
 File : dec_ld8f.c
 */
/* from dec_ld8d.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from dec_ld8k.c G.729 Annex B Version 1.3  Last modified: August 1997 */
/* from dec_ld8kd.c G.729 Version 3.3            */

/*-----------------------------------------------------------------*
 *   Functions Init_Decod_ld8f  and Decod_ld8f                     *
 *-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "ld8f.h"
#include "tabld8kd.h"
#include "dtx.h"
#include "sid.h"

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

   static Word16 lsp_old[M]= { 30000, 26000, 21000, 15000, 8000, 0,
                             -8000,-15000,-21000,-26000};
    /* gain predictor  memory in Q10 */
    static Word16 past_qua_en[4] = { -14336, -14336, -14336, -14336 };

        /* Filter's memory */
   static Word16 mem_syn[M];

   static Word16 sharp;           /* pitch sharpening of previous frame */
   static Word16 prev_T0;         /* integer delay of previous frame    */
   static Word16 prev_T0_frac;    /* integer delay of previous frame    */
   static Word16 gain_code;       /* Code gain                          */
   static Word16 gain_pitch;      /* Pitch gain                         */

   /* for G.729B */
   static Word16 seed_fer;
   /* CNG variables */
   static Word16 past_ftyp;
   static Word16 seed;
   static Word16 sid_sav, sh_sid_sav;

/*-----------------------------------------------------------------*
 *   Function Init_Decod_ld8f                                      *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *   ->Initialization of variables for the decoder section.        *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Init_Decod_ld8f(void)
{
    /* Initialize static pointer */
    exc = old_exc + PIT_MAX + L_INTERPOL;

    /* Static vectors to zero */
     Set_zero(old_exc, PIT_MAX+L_INTERPOL);
     Set_zero(mem_syn, M);

    sharp  = SHARPMIN;
    prev_T0 = 60;
    prev_T0_frac = 0;
    gain_code = 0;
    gain_pitch = 0;

    Lsp_decw_reset();

    /* for G.729B */
    seed_fer = (Word16)21845;
    past_ftyp = 3;
    seed = INIT_SEED;
    sid_sav = 0;
    sh_sid_sav = 1;
    Init_lsfq_noise();

    return;
}

/*-----------------------------------------------------------------*
 *   Function Decod_ld8c                                           *
 *           ~~~~~~~~~~                                            *
 *   ->Main decoder routine.                                       *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Decod_ld8f (

    Word16  parm[],      /* (i)   : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)  */
    Word16  voicing,     /* (i)   : voicing decision from previous frame */
    Word16  synth[],     /* (i/o)   : synthesis speech                     */
    Word16  Az_dec[],       /* (o) : decoded LP filter in 2 subframes     */
    Word16  *T0_first,    /* (o)   : decoded pitch lag in first subframe  */
    Word16 *Vad
)
{
    /* Scalars */
    Word16 i, j, i_subfr;
    Word16 T0, T0_frac, index;
    Word16 bfi;
    Word16 g_p, g_c;               /* fixed and adaptive codebook gain */
    Word16 bad_pitch;              /* bad pitch indicator */
    Word32 L_temp;

 /* Tables */
    Word16 lsp_new[M];         /* LSPs             */
    Word16 code[L_SUBFR];      /* ACELP codevector */
    Word16 *pA_t;                /* Pointer on A_t   */
    Word16 exc_phdisp[L_SUBFR]; /* excitation after phase dispersion */
    extern  Flag Overflow;
    Word16 rate;

    /* for G.729B */
    Word16 ftyp;
    Word16 lsfq_mem[MA_NP][M];

    /* Test bad frame indicator (bfi) */
    bfi = *parm++;

    /* Test frame type */
    ftyp = *parm++;

    if(bfi == 1) {
        ftyp = past_ftyp;
        if(ftyp == 1) ftyp = 0;
        if(ftyp == 3) parm[4] = 1;	/* G.729 maintenance */
        parm[-1] = ftyp;
    }
    *Vad = ftyp;

    rate = ftyp - (Word16)2;
    CODEC_MODE = add(rate,1); /* used by G729D routine */
    /* for G.729B */
    /* Processing non active frames (SID & not transmitted: ftyp = 1 or 0) */
    if(sub(ftyp,2) < 0) {
        Get_decfreq_prev(lsfq_mem);
        Dec_cng(past_ftyp, sid_sav, sh_sid_sav, &parm[-1], exc, lsp_old,
            Az_dec, &seed, lsfq_mem);

        Update_decfreq_prev(lsfq_mem);
        pA_t = Az_dec;
        for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
              Overflow = 0;
            Syn_filt(pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 0);
              if(Overflow != 0) {
            /* In case of overflow in the synthesis          */
            /* -> Scale down vector exc[] and redo synthesis */

            for(i=0; i<PIT_MAX+L_INTERPOL+L_FRAME; i++)
              old_exc[i] = shr(old_exc[i], 2);

            Syn_filt(pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 0);
             }
            Copy(&synth[i_subfr+L_SUBFR-M], mem_syn, M);
            pA_t += MP1;
        }
        *T0_first = prev_T0;
        sharp = SHARPMIN;
    }
    else {
        
        /***************************/
        /* Processing active frame */
        /***************************/
        seed = INIT_SEED;
        
        /* Decode the LSPs */
        D_lsp(parm, lsp_new, bfi);
       parm += 2;
       /* Interpolation of LPC for the 2 subframes */
        Int_qlpc(lsp_old, lsp_new, Az_dec);
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
         pA_t = Az_dec;
        for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {

            index = *parm++;            /* pitch index */

            if(i_subfr == 0) {
                if (rate == G729D)
                    i = 0;      /* no pitch parity at 6.4 kb/s */
                else
                    i = *parm++;             /* get parity check result */

                bad_pitch = add(bfi, i);
                if( bad_pitch == 0) {
                    Dec_lag3D(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac);
                    prev_T0 = T0;
                    prev_T0_frac = T0_frac;
                }
                else {                     /* Bad frame, or parity error */
                    if (bfi == 0) printf(" ! Wrong Pitch 1st subfr. !   ");
                    T0  =  prev_T0;
                    T0_frac = 0;
                    prev_T0 = add( prev_T0, 1);
                    if( sub(prev_T0, PIT_MAX) > 0) {
                        prev_T0 = PIT_MAX;
                    }
                }
                *T0_first = T0;         /* If first frame */
            }
            else {                       /* second subframe */

                if( bfi == 0) {
                    Dec_lag3D(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac);
                    prev_T0 = T0;
                    prev_T0_frac = T0_frac;
                }
                else {
                    T0  =  prev_T0;
                    T0_frac = 0;
                    prev_T0 = add( prev_T0, 1);
                    if( sub(prev_T0, PIT_MAX) > 0) prev_T0 = PIT_MAX;
                }
            }
            /*-------------------------------------------------*
            * - Find the adaptive codebook vector.            *
            *-------------------------------------------------*/
            Pred_lt_3(&exc[i_subfr], T0, T0_frac, L_SUBFR);


            /*-------------------------------------------------------*
            * - Decode innovative codebook.                         *
            *-------------------------------------------------------*/
            if(bfi != 0) {        /* Bad frame */

                parm[0] = Random(&seed_fer);
                parm[1] = Random(&seed_fer);
            }
            if (rate == G729) {

                /* case 8 kbps */
                Decod_ACELP(parm[1], parm[0], code);
                parm += 2;
            }
            else {
                /* case 6.4 kbps */
                Decod_ACELP64(parm[1], parm[0], code);
                parm += 2;
            }
            
            /*-------------------------------------------------------*
            * - Add the fixed-gain pitch contribution to code[].    *
            *-------------------------------------------------------*/
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
            if (rate == G729D)
                Dec_gain_6k(index, code, L_SUBFR, bfi, &gain_pitch, &gain_code,
                                past_qua_en);
            else
                Dec_gain_8k(index, code, L_SUBFR, bfi, &gain_pitch, &gain_code,
                        past_qua_en);
            /*-------------------------------------------------------------*
            * - Update pitch sharpening "sharp" with quantized gain_pitch *
            *-------------------------------------------------------------*/
            sharp = gain_pitch;
            if (sub(sharp, SHARPMAX) > 0) sharp = SHARPMAX;
            if (sub(sharp, SHARPMIN) < 0) sharp = SHARPMIN;

            /*-------------------------------------------------------*
            * - Find the total excitation.                          *
            * - Find synthesis speech corresponding to exc[].       *
            *-------------------------------------------------------*/
            if(bfi != 0) {       /* Bad frame */
                if (voicing == 0 ) {
                    g_p = 0;
                    g_c = gain_code;
                }
                else {
                    g_p = gain_pitch;
                    g_c = 0;
                }
            }
            else {
                g_p = gain_pitch;
                g_c = gain_code;
            }
            for (i = 0; i < L_SUBFR;  i++) {
                /* exc[i] = g_p*exc[i] + g_c*code[i]; */
                /* exc[i]  in Q0   g_p in Q14         */
                /* code[i] in Q13  g_code in Q1       */
                
                L_temp = L_mult(exc[i+i_subfr], g_p);
                L_temp = L_mac(L_temp, code[i], g_c);
                L_temp = L_shl(L_temp, 1);
                exc[i+i_subfr] = round(L_temp);
            }
            
            /* Test whether synthesis yields overflow or not */
            Overflow = 0;
            Syn_filt(pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 0);

            /* In case of overflow in the synthesis          */
            /* -> Scale down vector exc[] and redo synthesis */
            if(Overflow != 0) {
                for(i=0; i<PIT_MAX+L_INTERPOL+L_FRAME; i++) old_exc[i] = shr(old_exc[i], 2);
            }

            if (rate == G729D) {
                PhDisp(&exc[i_subfr], exc_phdisp, gain_code, gain_pitch, code);
                Syn_filt(pA_t, exc_phdisp, &synth[i_subfr], L_SUBFR,
                    mem_syn, 0);
            }
            else {
                Syn_filt(pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR,
                    mem_syn, 0);
                    /* Updates state machine for phase dispersion in
                6.4 kbps mode, if running at other rate */
                Update_PhDisp(gain_pitch, gain_code);
            }
            pA_t += MP1;    /* interpolated LPC parameters for next subframe */
            Copy(&synth[i_subfr+L_SUBFR-M], mem_syn, M);
        }
    }
    /*------------*
    *  For G729b
    *-----------*/
    if(bfi == 0) {
        L_temp = 0L;
        for(i=0; i<L_FRAME; i++) {
            L_temp = L_mac(L_temp, exc[i], exc[i]);
        } /* may overflow => last level of SID quantizer */
        sh_sid_sav = norm_l(L_temp);
        sid_sav = round(L_shl(L_temp, sh_sid_sav));
        sh_sid_sav = sub(16, sh_sid_sav);
    }
    past_ftyp = ftyp;
    
    /*--------------------------------------------------*
    * Update signal for next frame.                    *
    * -> shift to the left by L_FRAME  exc[]           *
    *--------------------------------------------------*/
    Copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);
    return;

}

