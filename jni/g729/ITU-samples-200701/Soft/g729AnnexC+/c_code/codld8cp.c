/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : CODLD8CP.C
*/

/*-----------------------------------------------------------------*
 *   Functions coder_ld8c and init_coder_ld8c                      *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *-----------------------------------------------------------------*/
#include <math.h>
#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "tab_ld8k.h"
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
static FLOAT ai_zero[L_SUBFR+M_BWDP1];
static FLOAT *zero;

/* Lsp (Line spectral pairs) */
static FLOAT lsp_old[M]=
{ (F)0.9595,  (F)0.8413,  (F)0.6549,  (F)0.4154,  (F)0.1423,
(F)-0.1423, (F)-0.4154, (F)-0.6549, (F)-0.8413, (F)-0.9595};
static FLOAT lsp_old_q[M];

/* Filter's memory */
static FLOAT mem_syn[M_BWD], mem_w0[M_BWD], mem_w[M_BWD];
static FLOAT mem_err[M_BWD+L_SUBFR], *error;
static FLOAT pit_sharp;

/* For G.729B */
/* DTX variables */
static int pastVad;
static int ppastVad;
static INT16 seed;

/* for G.729E */
/* for the backward analysis */
static FLOAT prev_filter[M_BWDP1]; /* Previous selected filter */

static FLOAT rexp[M_BWDP1];
static FLOAT synth[L_ANA_BWD];
static FLOAT *synth_ptr;
static int prev_lp_mode ;
static FLOAT gamma1[2], gamma2[2];       /* Weighting factor for the 2 subframes */
static FLOAT A_t_bwd_mem[M_BWDP1];
static int bwd_dominant;
static FLOAT C_int;              /* See file bwfw.c */
static INT16 glob_stat;  /* Mesure of global stationnarity */
static INT16 stat_bwd;       /* Nbre of consecutive backward frames */
static INT16 val_stat_bwd;   /* Value associated with stat_bwd */

/* Last backward A(z) for case of unstable filter */
static FLOAT old_A_bwd[M_BWDP1];
static FLOAT old_rc_bwd[2];
/* Last forkward A(z) for case of unstable filter */
static FLOAT old_A_fwd[MP1];
static FLOAT old_rc_fwd[2];
static FLOAT freq_prev[MA_NP][M];    /* previous LSP vector       */

static int lag_buf[5]={20,20, 20, 20,20};
static FLOAT pgain_buf[5]={(F)0.7,(F)0.7, (F)0.7, (F)0.7,(F)0.7};
#define         AVG(a,b,c,d) (int)(((a)+(b)+(c)+(d))/((F)4.0)+(F)0.5)

/*----------------------------------------------------------------------------
 * init_coder_ld8c - initialization of variables for the encoder
 *----------------------------------------------------------------------------
 */

void init_coder_ld8c(
    int dtx_enable   /* input : DTX enable flag */
)
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
    int   i;

    new_speech = old_speech + L_TOTAL - L_FRAME;         /* New speech     */
    speech     = new_speech - L_NEXT;                    /* Present frame  */
    p_window   = old_speech + L_TOTAL - L_WINDOW;        /* For LPC window */

    /* Initialize static pointers */
    wsp    = old_wsp + PIT_MAX;
    exc    = old_exc + PIT_MAX + L_INTERPOL;
    zero   = ai_zero + M_BWDP1;
    error  = mem_err + M_BWD;

    /* Static vectors to zero */
    set_zero(old_speech, L_TOTAL);
    set_zero(old_exc, PIT_MAX+L_INTERPOL);
    set_zero(old_wsp, PIT_MAX);
    set_zero(mem_syn, M_BWD);
    set_zero(mem_w,   M_BWD);
    set_zero(mem_w0,  M_BWD);
    set_zero(mem_err, M_BWD);
    set_zero(zero, L_SUBFR);
    pit_sharp = SHARPMIN;
    
    /* Initialize lsp_old_q[] */
    copy(lsp_old, lsp_old_q, M);
    
    lsp_encw_resete(freq_prev);
    init_exc_err();
    
    /* For G.729B */
    /* Initialize VAD/DTX parameters */
    if(dtx_enable == 1) {
        pastVad = 1;
        ppastVad = 1;
        seed = INIT_SEED;
        vad_init();
        init_lsfq_noise();
    }
    /* for G.729E */
    /* for the backward analysis */
    set_zero(synth, L_ANA_BWD);
    synth_ptr = synth + MEM_SYN_BWD;
    prev_lp_mode = 0;
    bwd_dominant = 0;              /* See file bwfw.c */
    C_int = (F)1.1;       /* Filter interpolation parameter */
    glob_stat = 10000;  /* Mesure of global stationnarity */
    stat_bwd = 0;       /* Nbre of consecutive backward frames */
    val_stat_bwd = 0;   /* Value associated with stat_bwd */

    for(i=0; i<M_BWDP1; i++) rexp[i] = (F)0.;

    A_t_bwd_mem[0] = (F)1.;
    for (i=1; i<M_BWDP1; i++) A_t_bwd_mem[i] = (F)0.;
    set_zero(prev_filter, M_BWDP1);
    prev_filter[0] = (F)1.;

    set_zero(old_A_bwd, M_BWDP1);
    old_A_bwd[0]= (F)1.;
    set_zero(old_rc_bwd, 2);

    set_zero(old_A_fwd, MP1);
    old_A_fwd[0]= (F)1.;
    set_zero(old_rc_fwd, 2);


    return;
}

/*----------------------------------------------------------------------------
* coder_ld8c - encoder routine ( speech data should be in new_speech )
*----------------------------------------------------------------------------
*/
void coder_ld8c(
    int ana[],        /* output: analysis parameters */
    int frame,        /* input : frame counter */
    int dtx_enable,   /* input : DTX enable flag */
    int rate          /* input : rate selector/ G729, G729D, and G729E */
)
{
    /* LPC analysis */
    FLOAT r_fwd[NP+1];          /* Autocorrelations (forward) */
    FLOAT r_bwd[M_BWDP1];      /* Autocorrelations (backward) */
    FLOAT rc_fwd[M];           /* Reflection coefficients : forward analysis */
    FLOAT rc_bwd[M_BWD];       /* Reflection coefficients : backward analysis */
    FLOAT A_t_fwd[MP1*2];      /* A(z) forward unquantized for the 2 subframes */
    FLOAT A_t_fwd_q[MP1*2];    /* A(z) forward quantized for the 2 subframes */
    FLOAT A_t_bwd[2*M_BWDP1];  /* A(z) backward for the 2 subframes */
    FLOAT *Aq;           /* A(z) "quantized" for the 2 subframes */
    FLOAT *Ap;           /* A(z) "unquantized" for the 2 subframes */
    FLOAT *pAp, *pAq;
    FLOAT Ap1[M_BWDP1];          /* A(z) with spectral expansion         */
    FLOAT Ap2[M_BWDP1];          /* A(z) with spectral expansion         */
    FLOAT lsp_new[M], lsp_new_q[M]; /* LSPs at 2th subframe                 */
    FLOAT lsf_int[M];               /* Interpolated LSF 1st subframe.       */
    FLOAT lsf_new[M];
    int lp_mode;                  /* LP Backward (1) / Forward (0) Indication mode */
    int m_ap, m_aq;
    int code_lsp[2];
    
    /* Other vectors */
    FLOAT h1[L_SUBFR];           /* Impulse response h1[]              */
    FLOAT xn[L_SUBFR];           /* Target vector for pitch search     */
    FLOAT xn2[L_SUBFR];          /* Target vector for codebook search  */
    FLOAT code[L_SUBFR];         /* Fixed codebook excitation          */
    FLOAT y1[L_SUBFR];           /* Filtered adaptive excitation       */
    FLOAT y2[L_SUBFR];           /* Filtered fixed codebook excitation */
    FLOAT res2[L_SUBFR];         /* Pitch prediction residual          */
    FLOAT g_coeff[5];            /* Correlations between xn, y1, & y2:
                                 <y1,y1>, <xn,y1>, <y2,y2>, <xn,y2>,<y1,y2>*/
                                 
    /* Scalars */
    int   i, j, i_gamma, i_subfr;
    int   T_op, t0, t0_min, t0_max, t0_frac;
    int   index, taming;
    FLOAT gain_pit, gain_code;

    /* for G.729E */
    int sat_filter;
    FLOAT freq_cur[M];
    
    /* For G.729B */
    FLOAT r_nbe[MP1];
    FLOAT lsfq_mem[MA_NP][M];
    int Vad;
    FLOAT Energy_db;

    int avg_lag;
    
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
    autocorr(p_window, NP, r_fwd);                     /* Autocorrelations */
    copy(r_fwd, r_nbe, MP1);
    lag_window(NP, r_fwd);                             /* Lag windowing    */
    levinsone(M, r_fwd, &A_t_fwd[MP1], rc_fwd,         /* Levinson Durbin  */
        old_A_fwd, old_rc_fwd );
    az_lsp(&A_t_fwd[MP1], lsp_new, lsp_old);           /* From A(z) to lsp */

    /* For G.729B */
    /* ------ VAD ------- */
    if (dtx_enable == 1) {
        lsp_lsf(lsp_new, lsf_new, M);
        vad(rc_fwd[1], lsf_new, r_fwd, p_window, frame,
            pastVad, ppastVad, &Vad, &Energy_db);

        musdetect( rate, r_fwd[0],rc_fwd, lag_buf , pgain_buf,
                      prev_lp_mode, frame,pastVad, &Vad, Energy_db);

        update_cng(r_nbe, Vad);
    }
    else Vad = 1;

    /* -------------------- */
    /* LP Backward analysis */
    /* -------------------- */
    if ( (rate-(1-Vad))== G729E) {

        /* LPC recursive Window as in G728 */
        autocorr_hyb_window(synth, r_bwd, rexp); /* Autocorrelations */
        lag_window_bwd(r_bwd);  /* Lag windowing    */
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
    copy(&synth[L_FRAME], &synth[0], MEM_SYN_BWD);

    /*--------------------------------------------------------------------*
    * Find interpolated LPC parameters in all subframes unquantized.      *
    * The interpolated parameters are in array A_t[] of size (M+1)*4      *
    *---------------------------------------------------------------------*/
    if( prev_lp_mode == 0) {
        int_lpc(lsp_old, lsp_new, lsf_int, lsf_new, A_t_fwd);
    }
    else {
        /* no interpolation */
        /* unquantized */
        lsp_az(lsp_new, A_t_fwd);           /* Subframe 1 */
        lsp_lsf(lsp_new, lsf_new, M);  /* transformation from LSP to LSF (freq.domain) */
        copy(lsf_new, lsf_int, M);      /* Subframe 1 */
    }

    if(Vad == 1) {
        /* ---------------- */
        /* LSP quantization */
        /* ---------------- */
        qua_lspe(lsp_new, lsp_new_q, code_lsp, freq_prev, freq_cur);

        /*-------------------------------------------------------------------*
        * Find interpolated LPC parameters in all subframes quantized.       *
        * and the quantized interpolated parameters are in array Aq_t[]      *
        *--------------------------------------------------------------------*/
        if( prev_lp_mode == 0) {
            int_qlpc(lsp_old_q, lsp_new_q, A_t_fwd_q);
        }
        else {
            /* no interpolation */
            lsp_az(lsp_new_q, &A_t_fwd_q[MP1]);              /* Subframe 2 */
            copy(&A_t_fwd_q[MP1], A_t_fwd_q, MP1);      /* Subframe 1 */
        }
        /*---------------------------------------------------------------------*
        * - Decision for the switch Forward / Backward                        *
        *---------------------------------------------------------------------*/
        if(rate == G729E) {
            set_lpc_mode(speech, A_t_fwd_q, A_t_bwd, &lp_mode, 
            lsp_new, lsp_old, &bwd_dominant, prev_lp_mode, prev_filter, &C_int,
                &glob_stat, &stat_bwd, &val_stat_bwd);
        }
        else 
             update_bwd( &lp_mode, &bwd_dominant, &C_int, &glob_stat);
    }
    else 
         update_bwd( &lp_mode, &bwd_dominant, &C_int, &glob_stat);

    /* ---------------------------------- */
    /* update the LSPs for the next frame */
    /* ---------------------------------- */
    copy(lsp_new, lsp_old, M);

    /*----------------------------------------------------------------------*
    * - Find the weighted input speech w_sp[] for the whole speech frame   *
    *----------------------------------------------------------------------*/
    if(lp_mode == 0) {
        m_ap = M;
        if (bwd_dominant == 0) Ap = A_t_fwd;
        else Ap = A_t_fwd_q;
        perc_var(gamma1, gamma2, lsf_int, lsf_new, rc_fwd);
    }
    else {
        if (bwd_dominant == 0) {
            m_ap = M;
            Ap = A_t_fwd;
        }
        else {
            m_ap = M_BWD;
            Ap = A_t_bwd;
        }
        perc_vare(gamma1, gamma2, bwd_dominant);
    }
    pAp = Ap;
    for (i=0; i<2; i++) {
        weight_az(pAp, gamma1[i], m_ap, Ap1);
        weight_az(pAp, gamma2[i], m_ap, Ap2);
        residue(m_ap, Ap1, &speech[i*L_SUBFR], &wsp[i*L_SUBFR], L_SUBFR);
        syn_filte(m_ap,  Ap2, &wsp[i*L_SUBFR], &wsp[i*L_SUBFR], L_SUBFR,
            &mem_w[M_BWD-m_ap], 0);
        for(j=0; j<M_BWD; j++) mem_w[j] = wsp[i*L_SUBFR+L_SUBFR-M_BWD+j];
        pAp += m_ap+1;
    }

    /* ---------------------- */
    /* Case of Inactive frame */
    /* ---------------------- */
    if (Vad == 0){

        for (i=0; i<MA_NP; i++) copy(&freq_prev[i][0], &lsfq_mem[i][0], M);
        cod_cng(exc, pastVad, lsp_old_q, old_A_fwd, old_rc_fwd, A_t_fwd_q,
                        ana, lsfq_mem, &seed);

        for (i=0; i<MA_NP; i++) copy(&lsfq_mem[i][0], &freq_prev[i][0], M);

        ppastVad = pastVad;
        pastVad = Vad;

        /* UPDATE wsp, mem_w, mem_syn, mem_err, and mem_w0 */
        pAp  = A_t_fwd;     /* pointer to interpolated LPC parameters           */
        pAq = A_t_fwd_q;    /* pointer to interpolated quantized LPC parameters */
        i_gamma = 0;
        for(i_subfr=0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
            weight_az(pAp, gamma1[i_gamma], M, Ap1);
            weight_az(pAp, gamma2[i_gamma], M, Ap2);
            i_gamma++;

            /* update mem_syn */
            syn_filte(M, pAq, &exc[i_subfr], &synth_ptr[i_subfr], L_SUBFR, &mem_syn[M_BWD-M], 0);
            for(j=0; j<M_BWD; j++) mem_syn[j] = synth_ptr[i_subfr+L_SUBFR-M_BWD+j];

            /* update mem_w0 */
            for (i=0; i<L_SUBFR; i++)
                error[i] = speech[i_subfr+i] - synth_ptr[i_subfr+i];
            residue(M, Ap1, error, xn, L_SUBFR);
            syn_filte(M, Ap2, xn, xn, L_SUBFR, &mem_w0[M_BWD-M], 0);
            for(j=0; j<M_BWD; j++) mem_w0[j] = xn[L_SUBFR-M_BWD+j];

            /* update mem_err */
            for (i = L_SUBFR-M_BWD, j = 0; i < L_SUBFR; i++, j++)
                mem_err[j] = error[i];

            for (i= 0; i< 4; i++)
                pgain_buf[i] = pgain_buf[i+1];
            pgain_buf[4] =  (F)0.5;

            pAp += MP1;
            pAq += MP1;
        }
        /* update previous filter for next frame */
        copy(&A_t_fwd_q[MP1], prev_filter, MP1);
        for(i=MP1; i <M_BWDP1; i++) prev_filter[i] = (F)0.;
        prev_lp_mode = lp_mode;

        pit_sharp = SHARPMIN;

        /* Update memories for next frames */
        copy(&old_speech[L_FRAME], &old_speech[0], L_TOTAL-L_FRAME);
        copy(&old_wsp[L_FRAME], &old_wsp[0], PIT_MAX);
        copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);
        return;

    }  /* End of inactive frame case */

    /* -------------------- */
    /* Case of Active frame */
    /* -------------------- */
    *ana++ = rate+2; /* bit rate mode */

    if(lp_mode == 0) {
        m_aq = M;
        Aq = A_t_fwd_q;
        /* update previous filter for next frame */
        copy(&Aq[MP1], prev_filter, MP1);
        for(i=MP1; i <M_BWDP1; i++) prev_filter[i] = (F)0.;
        for(j=MP1; j<M_BWDP1; j++) ai_zero[j] = (F)0.;
    }
    else {
        m_aq = M_BWD;
        Aq = A_t_bwd;
        if (bwd_dominant == 0) {
            for(j=MP1; j<M_BWDP1; j++) ai_zero[j] = (F)0.;
        }
        /* update previous filter for next frame */
        copy(&Aq[M_BWDP1], prev_filter, M_BWDP1);
    }


    if(dtx_enable == 1) {
        seed = INIT_SEED;
        ppastVad = pastVad;
        pastVad = Vad;
    }

    if (rate == G729E) *ana++ = lp_mode;

    if( lp_mode == 0) {
        copy(lsp_new_q, lsp_old_q, M);
        lsp_prev_update(freq_cur, freq_prev);
        *ana++ = code_lsp[0];
        *ana++ = code_lsp[1];
    }

    /*--------------------------------------------------------------------*
    * - Find the open-loop pitch delay for the whole speech frame        *
    * - Set the range for searching closed-loop pitch in 1st subframe    *
    *--------------------------------------------------------------------*/

    T_op = pitch_ol(wsp, PIT_MIN, PIT_MAX, L_FRAME);

    for (i= 0; i< 4; i++)
        lag_buf[i] = lag_buf[i+1];

    avg_lag = AVG(lag_buf[0],lag_buf[1],lag_buf[2],lag_buf[3]);
    if( abs( (int) (T_op/2.0) - avg_lag)<=2)
        lag_buf[4] = (int) (T_op/2.0);
    else if( abs((int) (T_op/3.0) - avg_lag)<=2)
        lag_buf[4] = (int) (T_op/3.0);
    else
        lag_buf[4] = T_op;

    /* Range for closed loop pitch search in 1st subframe */
    t0_min = T_op - 3;
    if (t0_min < PIT_MIN) t0_min = PIT_MIN;
    t0_max = t0_min + 6;
    if (t0_max > PIT_MAX) {
        t0_max = PIT_MAX;
        t0_min = t0_max - 6;
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
    pAq = Aq;      /* pointer to interpolated "quantized" LPC parameters */
        
    i_gamma = 0;
    
    for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR) {
        
    /*---------------------------------------------------------------*
    * Find the weighted LPC coefficients for the weighting filter.  *
        *---------------------------------------------------------------*/
        weight_az(pAp, gamma1[i_gamma], m_ap, Ap1);
        weight_az(pAp, gamma2[i_gamma], m_ap, Ap2);
        i_gamma++;
        
        /*---------------------------------------------------------------*
        * Compute impulse response, h1[], of weighted synthesis filter  *
        *---------------------------------------------------------------*/
        for (i = 0; i <=m_ap; i++) ai_zero[i] = Ap1[i];
        syn_filte(m_aq,  pAq, ai_zero, h1, L_SUBFR, zero, 0);
        syn_filte(m_ap,  Ap2, h1, h1, L_SUBFR, zero, 0);
        
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
        residue(m_aq, pAq, &speech[i_subfr], &exc[i_subfr], L_SUBFR);   /* LPC residual */
        for (i=0; i<L_SUBFR; i++) res2[i] = exc[i_subfr+i];
        syn_filte(m_aq,  pAq, &exc[i_subfr], error, L_SUBFR,
            &mem_err[M_BWD-m_aq], 0);
        residue(m_ap, Ap1, error, xn, L_SUBFR);
        syn_filte(m_ap,  Ap2, xn, xn, L_SUBFR, &mem_w0[M_BWD-m_ap], 0);    /* target signal xn[]*/
        
        t0 = pitch_fr3cp(&exc[i_subfr], xn, h1, L_SUBFR, t0_min, t0_max,
            i_subfr, &t0_frac, rate);
        
        index = enc_lag3cp(t0, t0_frac, &t0_min, &t0_max,PIT_MIN,PIT_MAX,i_subfr, rate);
        
        *ana++ = index;
        
        if ( (i_subfr == 0) && (rate != G729D) ) {
            *ana = parity_pitch(index);
            if( rate == G729E) {       
                *ana ^= ((index >> 1) & 0x0001);
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
        pred_lt_3(&exc[i_subfr], t0, t0_frac, L_SUBFR);
                
        convolve(&exc[i_subfr], h1, y1, L_SUBFR);
        
        gain_pit = g_pitch(xn, y1, g_coeff, L_SUBFR);
        
        /* clip pitch gain if taming is necessary */
        taming = test_err(t0, t0_frac);

        if(taming == 1){
            if ( gain_pit>  GPCLIP) {
                gain_pit = GPCLIP;
            }
        }
        
        for (i = 0; i < L_SUBFR; i++)
            xn2[i] = xn[i] - y1[i]*gain_pit;
        
        /*-----------------------------------------------------*
        * - Innovative codebook search.                       *
        *-----------------------------------------------------*/
        
        switch (rate) {
                        
            case G729:    /* 8 kbit/s */
            { 
                index = ACELP_codebook(xn2, h1, t0, pit_sharp, i_subfr, code,
                    y2, &i);
                *ana++ = index;        /* Positions index */
                *ana++ = i;            /* Signs index     */
                break;
            }

            case G729D:    /* 6.4 kbit/s */
            {
                index = ACELP_codebook64(xn2, h1, t0, pit_sharp, code, y2, &i);
                *ana++ = index;        /* Positions index */
                *ana++ = i;            /* Signs index     */
                break;
            }
                
            case G729E:    /* 11.8 kbit/s */
            {
            /*-----------------------------------------------------------------*
            * Include fixed-gain pitch contribution into impulse resp. h[]    *
                *-----------------------------------------------------------------*/
                if(t0 < L_SUBFR) {
                    for (i = t0; i < L_SUBFR; i++) {
                        h1[i] += pit_sharp * h1[i-t0];
                    }
                }
                    
                /* calculate residual after long term prediction */
                for (i = 0; i < L_SUBFR;  i++)
                    res2[i] -= gain_pit*exc[i+i_subfr];
                if (lp_mode == 0)
                    ACELP_10i40_35bits(xn2, res2, h1, code, y2, ana); /* Forward */
                else
                    ACELP_12i40_44bits(xn2, res2, h1, code, y2, ana); /* Backward */
                ana += 5;
                
                /*-----------------------------------------------------------------*
                * Include fixed-gain pitch contribution into code[].              *
                *-----------------------------------------------------------------*/
                if(t0 < L_SUBFR)
                    for (i = t0; i < L_SUBFR; i++)
                        code[i] += pit_sharp * code[i-t0];
                    
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
        corr_xy2( xn, y1, y2, g_coeff);
        
        if (rate == G729D) 
            index = qua_gain_6k(code, g_coeff, L_SUBFR, &gain_pit, &gain_code, taming );
        else 
            index = qua_gain(code, g_coeff, L_SUBFR, &gain_pit, &gain_code, taming);
        
        *ana++ = index;
        
        /*------------------------------------------------------------*
        * - Update pitch sharpening  with quantized gain_pit          *
        *------------------------------------------------------------*/
        for (i= 0; i< 4; i++)
            pgain_buf[i] = pgain_buf[i+1];
        pgain_buf[4] = gain_pit;

        pit_sharp = gain_pit;
        if (pit_sharp > SHARPMAX) pit_sharp = SHARPMAX;
        if (pit_sharp < SHARPMIN) pit_sharp = SHARPMIN;
        
        /*------------------------------------------------------*
        * - Find the total excitation                          *
        * - find synthesis speech corresponding to exc[]       *
        * - update filters memories for finding the target     *
        *   vector in the next subframe                        *
        *   (update error[-m..-1] and mem_w_err[])             *
        *   update error function for taming process           *
        *------------------------------------------------------*/
        for (i = 0; i < L_SUBFR;  i++)
            exc[i+i_subfr] = gain_pit*exc[i+i_subfr] + gain_code*code[i];
        
        update_exc_err(gain_pit, t0);
                
        syn_filte(m_aq,  pAq, &exc[i_subfr], &synth_ptr[i_subfr], L_SUBFR,
            &mem_syn[M_BWD-m_aq], 0);
        for(j=0; j<M_BWD; j++) mem_syn[j] = synth_ptr[i_subfr+L_SUBFR-M_BWD+j];
        
        for (i = L_SUBFR-M_BWD, j = 0; i < L_SUBFR; i++, j++) {
            mem_err[j] = speech[i_subfr+i] - synth_ptr[i_subfr+i];
            mem_w0[j]  = xn[i] - gain_pit*y1[i] - gain_code*y2[i];
        }
        pAp   += m_ap+1;
        pAq   += m_aq+1;
    }
    
    /*--------------------------------------------------*
    * Update signal for next frame.                    *
    * -> shift to the left by L_FRAME:                 *
    *     speech[], wsp[] and  exc[]                   *
    *--------------------------------------------------*/
    copy(&old_speech[L_FRAME], &old_speech[0], L_TOTAL-L_FRAME);
    copy(&old_wsp[L_FRAME], &old_wsp[0], PIT_MAX);
    copy(&old_exc[L_FRAME], &old_exc[0], PIT_MAX+L_INTERPOL);
    prev_lp_mode = lp_mode;
    return;
}

