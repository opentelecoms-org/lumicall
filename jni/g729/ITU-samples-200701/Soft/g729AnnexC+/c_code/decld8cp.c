/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
   Version 2.2    Last modified: October 2006 
*/

/*
 File : DECLD8CP.C
 */


/*-----------------------------------------------------------------*
*   Functions init_decod_ld8c  and decod_ld8c                     *
*-----------------------------------------------------------------*/

#include "typedef.h"
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
static FLOAT old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
static FLOAT *exc;

/* Lsp (Line spectral pairs) */
static FLOAT lsp_old[M]={
    (F)0.9595,  (F)0.8413,  (F)0.6549,  (F)0.4154,  (F)0.1423,
    (F)-0.1423, (F)-0.4154, (F)-0.6549, (F)-0.8413, (F)-0.9595};
        
        
static FLOAT mem_syn[M_BWD];        /* Filter's memory */
    
static FLOAT sharp ;            /* pitch sharpening of previous fr */
static FLOAT gain_code;         /* fixed codebook gain */
static FLOAT gain_pitch ;       /* adaptive codebook gain */
static int   prev_t0;         /* integer delay of previous frame    */
static int   prev_t0_frac;    /* integer delay of previous frame    */
    
/* for G.729B */
static INT16 seed_fer;
/* CNG variables */
static int past_ftyp;
static INT16 seed;
static FLOAT sid_sav;

/* for the backward analysis */
static FLOAT rexp[M_BWDP1];
static FLOAT A_bwd_mem[M_BWDP1];
static FLOAT A_t_bwd_mem[M_BWDP1];
static int   prev_voicing, prev_bfi, prev_lp_mode;
static FLOAT c_fe, c_int;
static FLOAT prev_filter[M_BWDP1]; /* Previous selected filter */
static int   prev_pitch;
static int   stat_pitch;
static int   pitch_sta, frac_sta;

/* Last backward A(z) for case of unstable filter */
static FLOAT old_A_bwd[M_BWDP1];
static FLOAT old_rc_bwd[2];

static FLOAT   gain_pit_mem;
static FLOAT   gain_cod_mem;
static FLOAT   c_muting;
static int     count_bfi;
static int     stat_bwd;

static FLOAT   freq_prev[MA_NP][M];   

/* static memory for frame erase operation */
static int prev_ma;               /* previous MA prediction coef.*/
static FLOAT prev_lsp[M];           /* previous LSP vector */

/*--------------------------------------------------------------------------
* init_decod_ld8c - Initialization of variables for the decoder section.
*--------------------------------------------------------------------------
*/
void init_decod_ld8c(void)
{
    /* Initialize static pointer */
    exc = old_exc + PIT_MAX + L_INTERPOL;
    
    /* Static vectors to zero */
    set_zero(old_exc, PIT_MAX+L_INTERPOL);
    set_zero(mem_syn, M_BWD);
    
    sharp        = SHARPMIN;
    prev_t0      = 60;
    prev_t0_frac = 0;
    gain_code    = (F)0.;
    gain_pitch   = (F)0.;
    
    lsp_decw_resete(freq_prev, prev_lsp, &prev_ma);
    
    set_zero(A_bwd_mem, M_BWDP1);
    set_zero(A_t_bwd_mem, M_BWDP1);
    A_bwd_mem[0]   = (F)1.;
    A_t_bwd_mem[0] = (F)1.;
    
    prev_voicing = 0;
    prev_bfi     = 0;
    prev_lp_mode    = 0;
    c_fe     = (F)0.;
    c_int        = (F)1.1;       /* Filter interpolation parameter */
    set_zero(prev_filter, M_BWDP1);
    prev_filter[0] = (F)1.;
    prev_pitch     = 30;
    stat_pitch     = 0;
    set_zero(old_A_bwd, M_BWDP1);
    set_zero(rexp, M_BWDP1);
    old_A_bwd[0]   = (F)1.;
    set_zero(old_rc_bwd, 2);
    gain_pit_mem   = (F)0.;
    gain_cod_mem   = (F)0.;
    c_muting       = (F)1.;
    count_bfi      = 0;
    stat_bwd       = 0;
    
    /* for G.729B */
    seed_fer = (INT16)21845;
    past_ftyp = 3;
    seed = INIT_SEED;
    sid_sav = (FLOAT)0.;
    init_lsfq_noise();
    
    return;
}
        
/*--------------------------------------------------------------------------
* decod_ld8c - decoder
*--------------------------------------------------------------------------
*/
void decod_ld8c(
    int    parm[],       /* (i)   : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)    */
    int    voicing,      /* (i)   : voicing decision from previous frame */
    FLOAT  synth_buf[],  /* (i/o) : synthesis speech                     */
    FLOAT  Az_dec[],     /* (o)   : decoded LP filter in 2 subframes     */
    int    *t0_first,    /* (o)   : decoded pitch lag in first subframe  */
    int    *bwd_dominant,/* (o)   : bwd dominant indicator               */
    int    *m_pst,        /* (o)   : LPC order for postfilter             */
    int    *Vad          /* output: decoded frame type                         */
)
{
    /* Scalars */
    int i, j, i_subfr;
    int t0, t0_frac, index;
    int bfi;
    int lp_mode;                   /* Backward / Forward mode indication */
    FLOAT g_p, g_c;               /* fixed and adaptive codebook gain */
    int bad_pitch;              /* bad pitch indicator */
    FLOAT tmp;
    FLOAT energy;
    int  rate;
    
    /* Tables */
    FLOAT A_t_bwd[2*M_BWDP1];   /* LPC Backward filter */
    FLOAT A_t_fwd[2*MP1];     /* LPC Forward filter */
    FLOAT rc_bwd[M_BWD];      /* LPC backward reflection coefficients */
    FLOAT r_bwd[M_BWDP1];   /* Autocorrelations (backward) */
    FLOAT lsp_new[M];         /* LSPs             */
    FLOAT code[L_SUBFR];      /* ACELP codevector */
    FLOAT exc_phdisp[L_SUBFR]; /* excitation after phase dispersion */
    FLOAT *pA_t;                /* Pointer on A_t   */
    int stationnary;
    int m_aq;
    FLOAT *synth;
    int sat_filter;
    
    /* for G.729B */
    int ftyp;
    FLOAT lsfq_mem[MA_NP][M];
    
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
    
    rate = ftyp - 2;
    /* Decoding the Backward/Forward LPC decision */
    /* ------------------------------------------ */
    if( rate != G729E) lp_mode = 0;
    else {
        if (bfi != 0) {
            lp_mode = prev_lp_mode; /* Frame erased => lp_mode = previous lp_mode */
            *parm++ = lp_mode;
        }
        else {
            lp_mode = *parm++;
        }
        if(prev_bfi != 0) voicing = prev_voicing;
    }
    if( bfi == 0) {
        c_muting = (F)1.;
        count_bfi = 0;
    }
    
    /* -------------------- */
    /* Backward LP analysis */
    /* -------------------- */
    if (rate == G729E) {
        /* LPC recursive Window as in G728 */
        autocorr_hyb_window(synth_buf, r_bwd, rexp); /* Autocorrelations */
        
        lag_window_bwd(r_bwd); /* Lag windowing    */
        
        /* Levinson (as in G729) */
        levinsone(M_BWD, r_bwd, &A_t_bwd[M_BWDP1], rc_bwd,
            old_A_bwd, old_rc_bwd );
        
        /* Tests saturation of A_t_bwd */
        sat_filter = 0;
        for (i=M_BWDP1; i<2*M_BWDP1; i++) if (A_t_bwd[i] >= (F)8.) sat_filter = 1;
        if (sat_filter == 1) copy(A_t_bwd_mem, &A_t_bwd[M_BWDP1], M_BWDP1);
        else copy(&A_t_bwd[M_BWDP1], A_t_bwd_mem, M_BWDP1);
        
        /* Additional bandwidth expansion on backward filter */
        weight_az(&A_t_bwd[M_BWDP1], GAMMA_BWD, M_BWD, &A_t_bwd[M_BWDP1]);
    }
    /*--------------------------------------------------*
     * Update synthesis signal for next frame.          *
     *--------------------------------------------------*/
    copy(&synth_buf[L_FRAME], &synth_buf[0], MEM_SYN_BWD);
    
    if(lp_mode == 1) {
        if ((c_fe != (F)0.)) {
            /* Interpolation of the backward filter after a bad frame */
            /* A_t_bwd(z) = c_fe . A_bwd_mem(z) + (1 - c_fe) . A_t_bwd(z) */
            /* ---------------------------------------------------------- */
            tmp = (F)1. - c_fe;
            pA_t = A_t_bwd + M_BWDP1;
            for (i=0; i<M_BWDP1; i++) {
                pA_t[i] *= tmp;
                pA_t[i] += c_fe * A_bwd_mem[i];
            }
        }
    }

    /* Memorize the last good backward filter when the frame is erased */
    if ((bfi != 0)&&(prev_bfi == 0) && (past_ftyp >3))
        copy(&A_t_bwd[M_BWDP1], A_bwd_mem, M_BWDP1);
    
    /* for G.729B */
    /* Processing non active frames (SID & not transmitted: ftyp = 1 or 0) */
    if(ftyp < 2) {
        /* get_decfreq_prev(lsfq_mem); */
        for (i=0; i<MA_NP; i++) 
            copy(&freq_prev[i][0], &lsfq_mem[i][0], M);
        
        dec_cng(past_ftyp, sid_sav, &parm[-1], exc, lsp_old,
            A_t_fwd, &seed, lsfq_mem);
        
        /*   update_decfreq_prev(lsfq_mem); */
        for (i=0; i<MA_NP; i++) 
            copy(&lsfq_mem[i][0], &freq_prev[i][0], M);
        
        pA_t = A_t_fwd;
        for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
            syn_filte(M, pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR, &mem_syn[M_BWD-M], 0);
            copy(&synth[i_subfr+L_SUBFR-M_BWD], mem_syn, M_BWD);

            *t0_first = prev_t0;
            pA_t += MP1;
        }
        sharp = SHARPMIN;
        c_int = (F)1.1;
        /* for gain decoding in case of frame erasure */
        stat_bwd = 0;
        stationnary = 0;
        /* for pitch tracking  in case of frame erasure */
        stat_pitch = 0;
        /* update the previous filter for the next frame */
        copy(&A_t_fwd[MP1], prev_filter, MP1);
        for(i=MP1; i<M_BWDP1; i++) prev_filter[i] = (F)0.;
    }

    /***************************/
    /* Processing active frame */
    /***************************/
    else {
        seed = INIT_SEED;
        
        /* ---------------------------- */
        /* LPC decoding in forward mode */
        /* ---------------------------- */
        if (lp_mode == 0) {

            /* Decode the LSPs */
            d_lspe(parm, lsp_new, bfi, freq_prev, prev_lsp, &prev_ma);
            parm += 2;
            if( prev_lp_mode == 0) { /* Interpolation of LPC for the 2 subframes */
                int_qlpc(lsp_old, lsp_new, A_t_fwd);
            }
            else {
                /* no interpolation */
                lsp_az(lsp_new, A_t_fwd);           /* Subframe 1*/
                copy(A_t_fwd, &A_t_fwd[MP1], MP1); /* Subframe 2 */
            }

            /* update the LSFs for the next frame */
            copy(lsp_new, lsp_old, M);

            c_int = (F)1.1;
            pA_t = A_t_fwd;
            m_aq = M;
            /* update the previous filter for the next frame */
            copy(&A_t_fwd[MP1], prev_filter, MP1);
            for(i=MP1; i<M_BWDP1; i++) prev_filter[i] = (F)0.;
        }
        else {
            int_bwd(A_t_bwd, prev_filter, &c_int);
            pA_t = A_t_bwd;
            m_aq = M_BWD;
            /* update the previous filter for the next frame */
            copy(&A_t_bwd[M_BWDP1], prev_filter, M_BWDP1);
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
                bad_pitch = bfi + i;     
                if( bad_pitch == 0) {
                    dec_lag3cp(index, PIT_MIN, PIT_MAX, i_subfr, &t0, &t0_frac, rate);
                    prev_t0 = t0;
                    prev_t0_frac = t0_frac;
                }
                else {                     /* Bad frame, or parity error */
                    if (bfi == 0) printf(" ! Wrong Pitch 1st subfr. !   ");
                    t0  =  prev_t0;
                    if (rate == G729E) {
                        t0_frac = prev_t0_frac;
                    }
                    else {
                        t0_frac = 0;
                        prev_t0++;
                        if(prev_t0 > PIT_MAX) {
                            prev_t0 = PIT_MAX;
                        }
                    }
                }
                *t0_first = t0;         /* If first frame */
            }
            else {                       /* second subframe */
                
                if( bfi == 0) {
                    dec_lag3cp(index, PIT_MIN, PIT_MAX, i_subfr, &t0, &t0_frac, rate);
                    prev_t0 = t0;
                    prev_t0_frac = t0_frac;
                }
                else {
                    t0  =  prev_t0;
                    if (rate == G729E) {
                        t0_frac = prev_t0_frac;
                    }
                    else {
                        t0_frac = 0;
                        prev_t0++;
                        if(prev_t0 > PIT_MAX) prev_t0 = PIT_MAX;
                    }
                }
            }
            /*-------------------------------------------------*
            * - Find the adaptive codebook vector.            *
            *-------------------------------------------------*/
            pred_lt_3(&exc[i_subfr], t0, t0_frac, L_SUBFR);
            
            /* --------------------------------- */
            /* pitch tracking for frame erasures */
            /* --------------------------------- */
            if( rate == G729E) {
                track_pit(&prev_t0, &prev_t0_frac, &prev_pitch, &stat_pitch,
                    &pitch_sta, &frac_sta);
            }
            else {
                i = prev_t0;
                j = prev_t0_frac;
                track_pit(&i, &j, &prev_pitch, &stat_pitch,
                    &pitch_sta, &frac_sta);
            }

            /*-------------------------------------------------------*
            * - Decode innovative codebook.                         *
            *-------------------------------------------------------*/
            if(bfi != 0) {        /* Bad frame */
                
                parm[0] = (int)random_g729c(&seed_fer);
                parm[1] = (int)random_g729c(&seed_fer);
                if (rate == G729E) {
                    parm[2] = (int)random_g729c(&seed_fer);
                    parm[3] = (int)random_g729c(&seed_fer);
                    parm[4] = (int)random_g729c(&seed_fer);
                }
                
            }
            
            stationnary = 0; /* to avoid visual warning */
            if (rate == G729) {
                /* case 8 kbps */
                decod_ACELP(parm[1], parm[0], code);
                parm += 2;
                /* for gain decoding in case of frame erasure */
                stat_bwd = 0;
                stationnary = 0;
            }
            else if (rate == G729D) {
                /* case 8 kbps */
                decod_ACELP64(parm[1], parm[0], code);
                parm += 2;
                /* for gain decoding in case of frame erasure */
                stat_bwd = 0;
                stationnary = 0;
            }
            else if (rate == G729E) {
                /* case 11.8 kbps */
                if (lp_mode == 0) {
                    dec_ACELP_10i40_35bits(parm, code);
                    /* for gain decoding in case of frame erasure */
                    stat_bwd = 0;
                    stationnary = 0;
                }
                else {
                    dec_ACELP_12i40_44bits(parm, code);
                    /* for gain decoding in case of frame erasure */
                    stat_bwd++;
                    if (stat_bwd >= 30) {
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
            for (i = t0; i < L_SUBFR; i++)   code[i] += sharp * code[i-t0];

            /*-------------------------------------------------*
            * - Decode pitch and codebook gains.              *
            *-------------------------------------------------*/
            index = *parm++;      /* index of energy VQ */

            if (rate == G729D)
                dec_gain_6k(index, code, L_SUBFR, bfi, &gain_pitch, &gain_code);
            else
                dec_gaine(index, code, L_SUBFR, bfi, &gain_pitch, &gain_code, rate,
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
            if (sharp > SHARPMAX) sharp = SHARPMAX;
            if (sharp < SHARPMIN) sharp = SHARPMIN;
            
            /*-------------------------------------------------------*
            * - Find the total excitation.                          *
            * - Find synthesis speech corresponding to exc[].       *
            *-------------------------------------------------------*/
            if(bfi != 0) {       /* Bad frame */
                count_bfi++;
                if (voicing == 0 ) {
                    g_p = (F)0.;
                    g_c = gain_code;
                }
                else {
                    g_p = gain_pitch;
                    g_c = (F)0.;
                }
            }
            else {
                g_p = gain_pitch;
                g_c = gain_code;
            }
        
            for (i = 0; i < L_SUBFR;  i++) {
                exc[i+i_subfr] = g_p * exc[i+i_subfr] + g_c * code[i];
            }
            
            if (rate == G729D) {
                PhDisp(&exc[i_subfr], exc_phdisp, gain_code, gain_pitch, code);
                syn_filte(m_aq, pA_t, exc_phdisp, &synth[i_subfr], L_SUBFR,
                                &mem_syn[M_BWD-m_aq], 0);
            }
            else {
                syn_filte(m_aq, pA_t, &exc[i_subfr], &synth[i_subfr], L_SUBFR,
                                    &mem_syn[M_BWD-m_aq], 0);

                /* Updates state machine for phase dispersion in
                6.4 kbps mode, if running at other rate */
                Update_PhDisp(gain_pitch, gain_code);
            }

            copy(&synth[i_subfr+L_SUBFR-M_BWD], mem_syn, M_BWD);

            pA_t += m_aq+1;    /* interpolated LPC parameters for next subframe */

        }

    }

    /*------------*
     *  For G729b
     *-----------*/
    if(bfi == 0) {
        sid_sav = (FLOAT)0.0;
        for(i=0; i<L_FRAME; i++) {
            sid_sav += exc[i] * exc[i];
        }
    }
    past_ftyp = ftyp;

    /*------------*
     *  For G729E
     *-----------*/
    energy = ener_dB(synth, L_FRAME);
    if (energy >= (F)40.) tst_bwd_dominant(bwd_dominant, lp_mode);

    /*--------------------------------------------------*
    * Update signal for next frame.                    *
    * -> shift to the left by L_FRAME  exc[]           *
    *--------------------------------------------------*/
    copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);

    if( lp_mode == 0) {
        copy(A_t_fwd, Az_dec, 2*MP1);
        *m_pst = M;
    }
    else {
        copy(A_t_bwd, Az_dec, 2*M_BWDP1);
        *m_pst = M_BWD;
    }
    
    prev_bfi     = bfi;
    prev_lp_mode    = lp_mode;
    prev_voicing = voicing;
    
    if (bfi != 0) c_fe = (F)1.;
    else {
        if (lp_mode == 0) c_fe = 0;
        else {
            if (*bwd_dominant == 1) c_fe -= (F)0.1;
            else c_fe -= (F)0.5;
            if (c_fe < 0)  c_fe= 0;
        }
    }
    
    return;
}
