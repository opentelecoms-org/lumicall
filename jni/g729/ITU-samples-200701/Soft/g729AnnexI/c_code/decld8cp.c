/*   ITU-T G.729 Annex I Software Package Release 2 (November 2006) */
/*
    ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
   Version 1.2    Last modified: October 2006 
*/

/*
 File : decld8cp.c
 */
/* from dec_ld8e.c G.729 Annex E Version 1.2  Last modified: May 1998 */
/* from decld8kd.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from dec_ld8k.c G.729 Annex B Version 1.3  Last modified: August 1997 */
/* from dec_ld8kd.c G.729 Version 3.3            */

/*-----------------------------------------------------------------*
 *   Functions Init_Decod_ld8c  and Decod_ld8c                     *
 *-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8cp.h"
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

        /* Filter's memory */

   static Word16 mem_syn[M_BWD];
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

    /* for the backward analysis */
    static Word32  rexp[M_BWDP1];
    static Word16 A_bwd_mem[M_BWDP1];
    static Word16 A_t_bwd_mem[2*M_BWDP1];
    static Word16  prev_voicing, prev_bfi, prev_lp_mode, C_fe_fix, C_int ;
    static Word16    prev_filter[M_BWDP1]; /* Previous selected filter */
    static Word16 prev_pitch;
    static Word16 stat_pitch;
    static Word16 pitch_sta, frac_sta;
    /* Last backward A(z) for case of unstable filter */
    static Word16 old_A_bwd[M_BWDP1];
    static Word16 old_rc_bwd[2];

    static Word16   gain_pit_mem;
    static Word16   gain_cod_mem;
    static Word16   c_muting;
    static Word16   count_bfi;
    static Word16   stat_bwd;

    static Word16 freq_prev[MA_NP][M];   /* Q13 */

    /* static memory for frame erase operation */
    static Word16 prev_ma;                  /* previous MA prediction coef.*/
    static Word16 prev_lsp[M];           /* previous LSP vector */


/*-----------------------------------------------------------------*
 *   Function Init_Decod_ld8c                                      *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *   ->Initialization of variables for the decoder section.        *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Init_Decod_ld8c(void)
{
    /* Initialize static pointer */
    exc = old_exc + PIT_MAX + L_INTERPOL;

    /* Static vectors to zero */
     Set_zero(old_exc, PIT_MAX+L_INTERPOL);
     Set_zero(mem_syn, M_BWD);

    sharp  = SHARPMIN;
    prev_T0 = 60;
    prev_T0_frac = 0;
    gain_code = 0;
    gain_pitch = 0;

    Lsp_decw_resete(freq_prev, prev_lsp, &prev_ma);

    Set_zero(A_bwd_mem, M_BWDP1);
    Set_zero(A_t_bwd_mem, M_BWDP1);
    A_bwd_mem[0] = 4096;
    A_t_bwd_mem[0] = 4096;

    prev_voicing = 0;
    prev_bfi = 0;
    prev_lp_mode = 0;
    C_fe_fix = 0;       /* Q12 */
    C_int = 4506;       /* Filter interpolation parameter */
    Set_zero(prev_filter, M_BWDP1);
    prev_filter[0] = 4096;
    prev_pitch = 30;
    stat_pitch = 0;
    Set_zero(old_A_bwd, M_BWDP1);
    old_A_bwd[0]= 4096;
    Set_zero(old_rc_bwd, 2);
    gain_pit_mem = 0;
    gain_cod_mem = 0;
    c_muting = 32767;
    count_bfi = 0;
    stat_bwd = 0;

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

void Decod_ld8c (

    Word16  parm[],      /* (i)   : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)  */
    Word16  voicing,     /* (i)   : voicing decision from previous frame */
    Word16  synth_buf[],     /* (i/o)   : synthesis speech                     */
    Word16  Az_dec[],       /* (o) : decoded LP filter in 2 subframes     */
    Word16  *T0_first,    /* (o)   : decoded pitch lag in first subframe  */
    Word16 *bwd_dominant,  /* (o)   : */
    Word16 *m_pst,         /* (o)   : LPC order for postfilter */
    Word16 *Vad
)
{
    /* Scalars */
    Word16 i, j, i_subfr;
    Word16 T0, T0_frac, index;
    Word16 bfi;
    Word16 lp_mode;                   /* Backward / Forward mode indication */
    Word16 g_p, g_c;               /* fixed and adaptive codebook gain */
    Word16 bad_pitch;              /* bad pitch indicator */
    Word16 tmp, tmp1, tmp2;
    Word16 sat_filter;
    Word32 L_temp;
    Word32 energy;
    Word16 temp;

 /* Tables */
    Word16 A_t_bwd[2*M_BWDP1];   /* LPC Backward filter */
    Word16 A_t_fwd[2*MP1];     /* LPC Forward filter */
    Word16 rc_bwd[M_BWD];      /* LPC backward reflection coefficients */
    Word32 r_bwd[M_BWDP1];   /* Autocorrelations (backward) */
    Word16 r_l_bwd[M_BWDP1];   /* Autocorrelations low (backward) */
    Word16 r_h_bwd[M_BWDP1];   /* Autocorrelations high (backward) */
    Word16 lsp_new[M];         /* LSPs             */
    Word16 code[L_SUBFR];      /* ACELP codevector */
    Word16 *pA_t;                /* Pointer on A_t   */
    Word16 stationnary;
    Word16 m_aq;
    Word16 *synth;
    Word16 exc_phdisp[L_SUBFR]; /* excitation after phase dispersion */
    extern  Flag Overflow;
    Word16 rate;

    /* for G.729B */
    Word16 ftyp;
    Word16 lsfq_mem[MA_NP][M];

    synth = synth_buf + MEM_SYN_BWD;
    /* Test bad frame indicator (bfi) */
    bfi = *parm++;

    /* Test frame type */
    ftyp = *parm++;

    if(bfi == 1) {
        ftyp = past_ftyp;
        if(ftyp == 1) ftyp = 0;
        if(ftyp > 2) {    /* G.729 maintenance */
          if(ftyp == 3) parm[4] = 1;
          else {
            if(prev_lp_mode == 0) parm[5] = 1;
            else parm[3] = 1;
          }
        }
        parm[-1] = ftyp;
    }
    *Vad = ftyp;

    rate = ftyp - (Word16)2;

    /* Decoding the Backward/Forward LPC decision */
     /* ------------------------------------------ */
    if( rate != G729E) lp_mode = 0;
    else {
        if (bfi != 0) {
            lp_mode = prev_lp_mode; /* Frame erased => mode = previous mode */
            *parm++ = lp_mode;
        }
        else {
            lp_mode = *parm++;
        }
        if(prev_bfi != 0) voicing = prev_voicing;
    }
    if( bfi == 0) {
        c_muting = 32767;
        count_bfi = 0;
    }
    /* -------------------- */
    /* Backward LP analysis */
    /* -------------------- */
    if (rate == G729E) {
        /* LPC recursive Window as in G728 */
        autocorr_hyb_window(synth_buf, r_bwd, rexp); /* Autocorrelations */

        Lag_window_bwd(r_bwd, r_h_bwd, r_l_bwd); /* Lag windowing    */

        /* Fixed Point Levinson (as in G729) */
        Levinsoncp(M_BWD, r_h_bwd, r_l_bwd, &A_t_bwd[M_BWDP1], rc_bwd,
                    old_A_bwd, old_rc_bwd, &temp);

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
    Copy(&synth_buf[L_FRAME], &synth_buf[0], MEM_SYN_BWD);

    if(lp_mode == 1) {
        if ((C_fe_fix != 0)) {
            /* Interpolation of the backward filter after a bad frame */
            /* A_t_bwd(z) = C_fe . A_bwd_mem(z) + (1 - C_fe) . A_t_bwd(z) */
            /* ---------------------------------------------------------- */
            tmp = sub(4096, C_fe_fix);
            pA_t = A_t_bwd + M_BWDP1;
            for (i=0; i<M_BWDP1; i++) {
                L_temp = L_mult(pA_t[i], tmp);
                L_temp = L_shr(L_temp, 13);
                tmp1 = extract_l(L_temp);
                L_temp = L_mult(A_bwd_mem[i], C_fe_fix);
                L_temp = L_shr(L_temp, 13);
                tmp2 = extract_l(L_temp);
                pA_t[i] = add(tmp1, tmp2);
            }
        }
    }

    /* Memorize the last good backward filter when the frame is erased */
    if ((bfi != 0)&&(prev_bfi == 0) && (past_ftyp >3))
        for (i=0; i<M_BWDP1; i++) A_bwd_mem[i] = A_t_bwd[i+M_BWDP1];

    /* for G.729B */
    /* Processing non active frames (SID & not transmitted: ftyp = 1 or 0) */
    if(ftyp < 2) {
        /* get_decfreq_prev(lsfq_mem); */
        for (i=0; i<MA_NP; i++) 
            Copy(&freq_prev[i][0], &lsfq_mem[i][0], M);

        Dec_cng(past_ftyp, sid_sav, sh_sid_sav, &parm[-1], exc, lsp_old,
            A_t_fwd, &seed, lsfq_mem);
        
        /*   update_decfreq_prev(lsfq_mem); */
        for (i=0; i<MA_NP; i++) 
            Copy(&lsfq_mem[i][0], &freq_prev[i][0], M);

        pA_t = A_t_fwd;
        for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
              Overflow = 0;
            Syn_filte(M, pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR,
                    &mem_syn[M_BWD-M], 0);
              if(Overflow != 0) {
            /* In case of overflow in the synthesis          */
            /* -> Scale down vector exc[] and redo synthesis */

            for(i=0; i<PIT_MAX+L_INTERPOL+L_FRAME; i++)
              old_exc[i] = shr(old_exc[i], 2);

            Syn_filte(M, pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR,
                    &mem_syn[M_BWD-M], 0);
             }
            Copy(&synth[i_subfr+L_SUBFR-M_BWD], mem_syn, M_BWD);
            pA_t += MP1;
        }
        *T0_first = prev_T0;
        sharp = SHARPMIN;
        C_int = 4506;
        /* for gain decoding in case of frame erasure */
        stat_bwd = 0;
        stationnary = 0;
        /* for pitch tracking  in case of frame erasure */
        stat_pitch = 0;
        /* update the previous filter for the next frame */
        Copy(&A_t_fwd[MP1], prev_filter, MP1);
        for(i=MP1; i<M_BWDP1; i++) prev_filter[i] = 0;
    }
    else {
        
        /***************************/
        /* Processing active frame */
        /***************************/
        seed = INIT_SEED;
        
        /* ---------------------------- */
        /* LPC decoding in forward mode */
        /* ---------------------------- */
        if (lp_mode == 0) {
            
            /* Decode the LSPs */
            D_lspe(parm, lsp_new, bfi, freq_prev, prev_lsp, &prev_ma);
            parm += 2;
            if( prev_lp_mode == 0) { /* Interpolation of LPC for the 2 subframes */
                Int_qlpc(lsp_old, lsp_new, A_t_fwd);
            }
            else {
                /* no interpolation */
                Lsp_Az(lsp_new, A_t_fwd);           /* Subframe 1*/
                Copy(A_t_fwd, &A_t_fwd[MP1], MP1); /* Subframe 2 */
            }
            
            /* update the LSFs for the next frame */
            Copy(lsp_new, lsp_old, M);
            
            C_int = 4506;
            pA_t = A_t_fwd;
            m_aq = M;
            /* update the previous filter for the next frame */
            Copy(&A_t_fwd[MP1], prev_filter, MP1);
            for(i=MP1; i<M_BWDP1; i++) prev_filter[i] = 0;
        }
        else {
            Int_bwd(A_t_bwd, prev_filter, &C_int);
            pA_t = A_t_bwd;
            m_aq = M_BWD;
            /* update the previous filter for the next frame */
            Copy(&A_t_bwd[M_BWDP1], prev_filter, M_BWDP1);
        }
        
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
        
        for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
            
            index = *parm++;            /* pitch index */
            
            if(i_subfr == 0) {
                if (rate == G729D)
                    i = 0;      /* no pitch parity at 6.4 kb/s */
                else
                    i = *parm++;             /* get parity check result */
                
                bad_pitch = add(bfi, i);
                if( bad_pitch == 0) {
                    Dec_lag3cp(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac,rate);
                    prev_T0 = T0;
                    prev_T0_frac = T0_frac;
                }
                else {                     /* Bad frame, or parity error */
                    if (bfi == 0) printf(" ! Wrong Pitch 1st subfr. !   ");
                    T0  =  prev_T0;
                    if (rate == G729E) {
                        T0_frac = prev_T0_frac;
                    }
                    else {
                        T0_frac = 0;
                        prev_T0 = add( prev_T0, 1);
                        if( sub(prev_T0, PIT_MAX) > 0) {
                            prev_T0 = PIT_MAX;
                        }
                    }
                }
                *T0_first = T0;         /* If first frame */
            }
            else {                       /* second subframe */
                
                if( bfi == 0) {
                    Dec_lag3cp(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac, rate);
                    prev_T0 = T0;
                    prev_T0_frac = T0_frac;
                }
                else {
                    T0  =  prev_T0;
                    if (rate == G729E) {
                        T0_frac = prev_T0_frac;
                    }
                    else {
                        T0_frac = 0;
                        prev_T0 = add( prev_T0, 1);
                        if( sub(prev_T0, PIT_MAX) > 0) prev_T0 = PIT_MAX;
                    }
                }
            }
            /*-------------------------------------------------*
            * - Find the adaptive codebook vector.            *
            *-------------------------------------------------*/
            Pred_lt_3(&exc[i_subfr], T0, T0_frac, L_SUBFR);
            
            /* --------------------------------- */
            /* pitch tracking for frame erasures */
            /* --------------------------------- */
            if( rate == G729E) {
                track_pit(&prev_T0, &prev_T0_frac, &prev_pitch, &stat_pitch,
                    &pitch_sta, &frac_sta);
            }
            else {
                i = prev_T0;
                j = prev_T0_frac;
                track_pit(&i, &j, &prev_pitch, &stat_pitch,
                    &pitch_sta, &frac_sta);
            }
            
            /*-------------------------------------------------------*
            * - Decode innovative codebook.                         *
            *-------------------------------------------------------*/
            if(bfi != 0) {        /* Bad frame */
                
                parm[0] = Random_g729cp(&seed_fer);
                parm[1] = Random_g729cp(&seed_fer);
                if (rate == G729E) {
                    parm[2] = Random_g729cp(&seed_fer);
                    parm[3] = Random_g729cp(&seed_fer);
                    parm[4] = Random_g729cp(&seed_fer);
                }
            }
            stationnary = 0; /* to avoid visual warning */
            
            if (rate == G729) {
                
                /* case 8 kbps */
                Decod_ACELP(parm[1], parm[0], code);
                parm += 2;
                /* for gain decoding in case of frame erasure */
                stat_bwd = 0;
                stationnary = 0;
            }
            else if (rate == G729D) {
                /* case 6.4 kbps */
                Decod_ACELP64(parm[1], parm[0], code);
                parm += 2;
                /* for gain decoding in case of frame erasure */
                stat_bwd = 0;
                stationnary = 0;
            }
            else if (rate == G729E) {
                /* case 11.8 kbps */
                if (lp_mode == 0) {
                    Dec_ACELP_10i40_35bits(parm, code);
                    /* for gain decoding in case of frame erasure */
                    stat_bwd = 0;
                    stationnary = 0;
                }
                else {
                    Dec_ACELP_12i40_44bits(parm, code);
                    /* for gain decoding in case of frame erasure */
                    stat_bwd = add(stat_bwd,1);
                    if (sub(stat_bwd,30) >= 0) {
                        stationnary = 1;
                        stat_bwd = 30;
                    }
                    else stationnary = 0;
                }
                parm += 5;
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
                Dec_gain_6k(index, code, L_SUBFR, bfi, &gain_pitch, &gain_code);
            else
                Dec_gaine(index, code, L_SUBFR, bfi, &gain_pitch, &gain_code, rate,
                gain_pit_mem, gain_cod_mem, &c_muting, count_bfi, stationnary);
                /*-------------------------------------------------------------*
                * - Update previous gains
            *-------------------------------------------------------------*/
            gain_pit_mem = gain_pitch;
            gain_cod_mem = gain_code;
            
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
                count_bfi = add(count_bfi,1);
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
            Syn_filte(m_aq, pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR,
                &mem_syn[M_BWD-m_aq], 0);
            
            /* In case of overflow in the synthesis          */
            /* -> Scale down vector exc[] and redo synthesis */
            if(Overflow != 0) {
                for(i=0; i<PIT_MAX+L_INTERPOL+L_FRAME; i++) old_exc[i] = shr(old_exc[i], 2);
            }
            
            if (rate == G729D) {
                PhDisp(&exc[i_subfr], exc_phdisp, gain_code, gain_pitch, code);
                Syn_filte(m_aq, pA_t, exc_phdisp, &synth[i_subfr], L_SUBFR,
                    &mem_syn[M_BWD-m_aq], 0);
            }
            else {
                Syn_filte(m_aq, pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR,
                    &mem_syn[M_BWD-m_aq], 0);
                    /* Updates state machine for phase dispersion in
                6.4 kbps mode, if running at other rate */
                Update_PhDisp(gain_pitch, gain_code);
            }
            pA_t += m_aq+1;    /* interpolated LPC parameters for next subframe */
            Copy(&synth[i_subfr+L_SUBFR-M_BWD], mem_syn, M_BWD);
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
    
    /*------------*
    *  For G729E
    *-----------*/
    energy = ener_dB(synth, L_FRAME);
    if (energy >= 8192) tst_bwd_dominant(bwd_dominant, lp_mode);
    
    /*--------------------------------------------------*
    * Update signal for next frame.                    *
    * -> shift to the left by L_FRAME  exc[]           *
    *--------------------------------------------------*/
    Copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);
    
    if( lp_mode == 0) {
        Copy(A_t_fwd, Az_dec, 2*MP1);
        *m_pst = M;
    }
    else {
        Copy(A_t_bwd, Az_dec, 2*M_BWDP1);
        *m_pst = M_BWD;
    }
    
    
    prev_bfi = bfi;
    prev_lp_mode = lp_mode;
    prev_voicing = voicing;
    
    if (bfi != 0) C_fe_fix = 4096;
    else {
        if (lp_mode == 0) C_fe_fix = 0;
        else {
            if (*bwd_dominant == 1) C_fe_fix = sub(C_fe_fix, 410);
            else C_fe_fix = sub(C_fe_fix, 2048);
            if (C_fe_fix < 0)  C_fe_fix= 0;
        }
    }
    return;

}

