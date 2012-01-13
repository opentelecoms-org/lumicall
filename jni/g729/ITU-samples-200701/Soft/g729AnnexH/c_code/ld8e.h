/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.2    Last modified: May 1998 */

/*--------------------------------------------------------------*
 * LD8E.H                                                       *
 * ~~~~~~                                                       *
 * Function prototypes and constants use in G.729E              *
 *                                                              *
 *--------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
 *       Codec constant parameters (coder, decoder, and postfilter)         *
 *--------------------------------------------------------------------------*/
 /* bitstream paramaters */
#define PRM_SIZE_E_fwd  18       /* Size of vector of analysis parameters.    */
#define PRM_SIZE_E_bwd  16       /* Size of vector of analysis parameters.    */
#define PRM_SIZE_E      18       /* Size of vector of analysis parameters.    */

#define SERIAL_SIZE_E (116+4)  /* Bits/frame + bfi+ number of speech bits
                                             + bit de mode + protection */

/* backward LPC analysis parameters */
#define M_BWD       30         /* Order of Backward LP filter.              */
#define M_BWDP1     (M_BWD+1)  /* Order of Backward LP filter + 1           */
#define NRP         35
#define MEM_SYN_BWD     M_BWD + NRP
#define N1              M_BWD + L_FRAME
#define L_ANA_BWD       L_FRAME + MEM_SYN_BWD
#define L_ANA_BWD_M1    L_ANA_BWD - 1
#define W_FACT  10368
#define GAMMA_BWD 32113

/* short term pst parameters :                                              */
#define GAMMA1_PST_E  22938      /* denominator weighting factor   (Q15)      */
#define GAMMA2_PST_E  21300      /* numerator  weighting factor (Q15)         */
#define LONG_H_ST_E   32        /* impulse response length                   */
#define GAMMA_HARM_E 8192
#define GAMMA_HARM 16384

/* ACELP codebooks parameters */
#define NB_TRACK  5
#define Q15_1_5   6554

/*--------------------------------------------------------------------------*
 * Main coder and decoder functions                                         *
 *--------------------------------------------------------------------------*/
void Init_Coder_ld8e(void);

void Coder_ld8e(
  Word16 ana[],     /* (o)     : analysis parameters                        */
  Word16 rate           /* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps*/
);

void Init_Decod_ld8e(void);

void Decod_ld8e(
  Word16 parm[],   /* (i)     : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)      */
  Word16 rate,          /* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps*/
  Word16 voicing,  /* (i)     : voicing decision from previous frame       */
  Word16 synth[],  /* (o)     : synthesized speech                         */
  Word16  A_t[],    /* (o)     : decoded LP filter for 2 subframes          */
  Word16 *T0_first, /* (o)     : decoded pitch lag in first subframe        */
  Word16 *stationnary,  /* output:  stationnarity indicator */
  Word16 *m_pst         /* output:  LPC order */
);

/*--------------------------------------------------------------------------*
 * protypes of functions  similar to G729                                   *
 * differences :                                                            *
 * list of arguments modified                                               *
 * local static variables and arrays are now passed as parameters           *
 * LPC order formerly constant is now passed as variable parameter          *
 * some temporary variables are now passed to the calling routine           *
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
 * LPC analysis and filtering                                               *
 *--------------------------------------------------------------------------*/
void Levinsone(
  Word16 m,        /* (i)    : LPC order                         */
  Word16 Rh[],      /* (i)     : Rh[m+1] autocorrelation coefficients (msb) */
  Word16 Rl[],      /* (i)     : Rl[m+1] autocorrelation coefficients (lsb) */
  Word16 A[],       /* (o) Q12 : A[m]    LPC coefficients  (m = 10)         */
  Word16 rc[],       /* (o) Q15 : rc[M]   Reflection coefficients.           */
  Word16 old_A[], /* (i/o) Q12 : last stable filter LPC coefficients  */
  Word16 old_rc[] /* (i/o) Q15 : last stable filter Reflection coefficients.         */
);

void Residue(
  Word16 m,        /* (i)    : LPC order                         */
  Word16 a[],    /* (i) Q12 : prediction coefficients                     */
  Word16 x[],    /* (i)     : speech (values x[-m..-1] are needed         */
  Word16 y[],    /* (o)     : residual signal                             */
  Word16 lg      /* (i)     : size of filtering                           */
);
void Syn_filte(
  Word16 m,        /* (i)    : LPC order                         */
  Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
  Word16 x[],     /* (i)     : input signal                             */
  Word16 y[],     /* (o)     : output signal                            */
  Word16 lg,      /* (i)     : size of filtering                        */
  Word16 mem[],   /* (i/o)   : memory associated with this filtering.   */
  Word16 update   /* (i)     : 0=no update, 1=update of memory.         */
);

/*--------------------------------------------------------------------------*
 * LSP VQ functions.                                                        *
 *--------------------------------------------------------------------------*/
void Qua_lspe(
  Word16 lsp[],     /* (i) Q15 : Unquantized LSP                            */
  Word16 lsp_q[],   /* (o) Q15 : Quantized LSP                              */
  Word16 ana[],      /* (o)     : indexes                                    */
  Word16 freq_prev[MA_NP][M],    /* (i) Q13 : previous LSP MA vector        */
  Word16 freq_cur[]   /* (o) Q13 : current LSP MA vector        */
);
void Lsp_encw_resete(
  Word16 freq_prev[MA_NP][M]    /* (i) Q13 : previous LSP MA vector        */
);
void Lsp_qua_cse(
  Word16 flsp_in[M],    /* (i) Q13 : Original LSP parameters    */
  Word16 lspq_out[M],   /* (o) Q13 : Quantized LSP parameters   */
  Word16 *code,         /* (o)     : codes of the selected LSP  */
  Word16 freq_prev[MA_NP][M],    /* (i) Q13 : previous LSP MA vector        */
  Word16 freq_cur[]     /* (o) Q13 : current LSP MA vector        */
);
void Lsp_get_quante(
  Word16 lspcb1[][M],      /* (i) Q13 : first stage LSP codebook      */
  Word16 lspcb2[][M],      /* (i) Q13 : Second stage LSP codebook     */
  Word16 code0,               /* (i)     : selected code of first stage  */
  Word16 code1,               /* (i)     : selected code of second stage */
  Word16 code2,               /* (i)     : selected code of second stage */
  Word16 fg[][M],          /* (i) Q15 : MA prediction coef.           */
  Word16 freq_prev[][M],   /* (i) Q13 : previous LSP vector           */
  Word16 lspq[],              /* (o) Q13 : quantized LSP parameters      */
  Word16 fg_sum[],             /* (i) Q15 : present MA prediction coef.   */
  Word16 freq_cur[]             /* (i) Q15 : present MA prediction coef.   */
);
void Relspwede(
  Word16 lsp[],                     /* (i) Q13 : unquantized LSP parameters */
  Word16 wegt[],                    /* (i) norm: weighting coefficients     */
  Word16 lspq[],                    /* (o) Q13 : quantized LSP parameters   */
  Word16 lspcb1[][M],            /* (i) Q13 : first stage LSP codebook   */
  Word16 lspcb2[][M],            /* (i) Q13 : Second stage LSP codebook  */
  Word16 fg[MODE][MA_NP][M],     /* (i) Q15 : MA prediction coefficients */
  Word16 freq_prev[MA_NP][M],    /* (i) Q13 : previous LSP vector        */
  Word16 fg_sum[MODE][M],        /* (i) Q15 : present MA prediction coef.*/
  Word16 fg_sum_inv[MODE][M],    /* (i) Q12 : inverse coef.              */
  Word16 code_ana[],                 /* (o)     : codes of the selected LSP  */
  Word16 freq_cur[]             /* (o) Q13 : current LSP MA vector        */
);

void D_lspe(
  Word16 prm[],          /* (i)     : indexes of the selected LSP */
  Word16 lsp_q[],        /* (o) Q15 : Quantized LSP parameters    */
  Word16 erase,           /* (i)     : frame erase information     */
  Word16 freq_prev[MA_NP][M],    /* (i/o) Q13 : previous LSP MA vector        */
  Word16 prev_lsp[],            /* (i/o) Q13 : previous LSP vector        */
  Word16 *prev_ma               /* (i/o) previous MA prediction coef.*/
);
void Lsp_decw_resete(
  Word16 freq_prev[MA_NP][M],    /* (o) Q13 : previous LSP MA vector        */
  Word16 prev_lsp[],            /* (o) Q13 : previous LSP vector        */
  Word16 *prev_ma               /* previous MA prediction coef.*/
);
void Lsp_iqua_cse(
 Word16 prm[],          /* (i)     : indexes of the selected LSP */
 Word16 lsp_q[],        /* (o) Q13 : Quantized LSP parameters    */
 Word16 erase,           /* (i)     : frame erase information     */
  Word16 freq_prev[MA_NP][M],    /* (i/o) Q13 : previous LSP MA vector        */
  Word16 prev_lsp[],            /* (i/o) Q13 : previous LSP vector        */
  Word16 *prev_ma               /* (i/o) previous MA prediction coef.*/
);
/*--------------------------------------------------------------------------*
 * Postfilter functions                                                     *
 *--------------------------------------------------------------------------*/
void Poste(
  Word16 t0,        /* (i) : 1st subframe delay given by coder              */
  Word16 *signal_ptr, /* (i) : input signal (pointer to current subframe    */
  Word16  *coeff,    /* (i) : LPC coefficients for current subframe          */
  Word16 *sig_out,  /* (o) : postfiltered output                            */
  Word16 *vo,        /* (o) : voicing decision 0 = uv,  > 0 delay            */
  Word16 gamma1,            /* input: short term postfilt. den. weighting factor*/
  Word16 gamma2,            /* input: short term postfilt. num. weighting factor*/
  Word16 gamma_harm,        /* input: long term postfilter weighting factor*/
  Word16  long_h_st,    /* input: impulse response length*/
  Word16 m_pst          /* input:  LPC order */
);
/*--------------------------------------------------------------------------*
 * protypes of functions  containing G729 source code + specific G729E code *
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
 * gain VQ functions.                                                       *
 *--------------------------------------------------------------------------*/
void Dec_gaine(
   Word16 index,        /* (i)     :Index of quantization.         */
   Word16 code[],       /* (i) Q13 :Innovative vector.             */
   Word16 L_subfr,      /* (i)     :Subframe length.               */
   Word16 bfi,          /* (i)     :Bad frame indicator            */
   Word16 *gain_pit,    /* (o) Q14 :Pitch gain.                    */
   Word16 *gain_cod,     /* (o) Q1  :Code gain.                     */
   Word16 rate,             /* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps*/
   Word16 gain_pit_mem,
   Word16 gain_cod_mem,
   Word16  *c_muting,
   int      count_bfi,
   int      stationnary
);

/*--------------------------------------------------------------------------*
 * Bitstream functions                                                      *
 *--------------------------------------------------------------------------*/
void bits2prm_ld8e(
    Word16 bits[],  /* I   serial bits (nb_bits)                          */
    Word16 prm[],   /* O   output: decoded parameters (11 parameters)     */
    Word16 rate             /* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps*/
);


void  prm2bits_ld8e(
  Word16 prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
  Word16 bits[],          /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = nbbits */
  Word16 rate           /* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps*/
);

/*--------------------------------------------------------------------------*
 * functions  specific to G729E                                             *
 *--------------------------------------------------------------------------*/
/* backward LPC analysis and switch forward/backward */
void autocorr_hyb_window(
    Word16  *x,         /* Input speech signal  */
    Word32  *r_bwd,        /* (out)    Autocorrelations    */
    Word32 *rexp        /* (in/out) */
);
void Lag_window_bwd(Word32 *r_bwd, Word16 *r_h_bwd, Word16 *r_l_bwd);
void Int_bwd(Word16 *a_bwd, Word16 *prev_filter, Word16 *C_int );

void set_lpc_mode(Word16 *signal_ptr, Word16 *a_fwd, Word16 *a_bwd,
  Word16 *bwd, Word16 *lspnew, Word16 *lspold, Word16 rate,
  Word16 *high_stat, Word16 prev_mode, Word16 *prev_filter, Word16 *C_int,
  Word16 *glob_stat, Word16 *stat_bwd, Word16 *val_stat_bwd);
Word16 ener_dB(Word16 *synth, Word16 L);
void tst_bwd_dominant(Word16 *high_stat, Word16 mode);

void perc_vare (
  Word16 *gamma1,   /* Bandwidth expansion parameter */
  Word16 *gamma2,   /* Bandwidth expansion parameter */
  Word16  high_stat /* high_stat indication (see file bwfw.c) */
);

/*--------------------------------------------------------------------------*
 * G729E fixed (ACELP) codebook excitation.                                               *
 *--------------------------------------------------------------------------*/

void ACELP_12i40_44bits(
  Word16 x[],    /* (i) Q0 : target vector                                 */
  Word16 cn[],   /* (i) Q0 : residual after long term prediction           */
  Word16 H[],    /* (i) Q12: impulse response of weighted synthesis filter */
  Word16 code[], /* (o) Q12: algebraic (fixed) codebook excitation         */
  Word16 y[],    /* (o) Q11: filtered fixed codebook excitation            */
  Word16 indx[]  /* (o)    : index 5 words: 13,10,7,7,7 = 44 bits          */
);
void ACELP_10i40_35bits(
  Word16 x[],    /* (i) Q0 : target vector                                 */
  Word16 cn[],   /* (i) Q0 : residual after long term prediction           */
  Word16 H[],    /* (i) Q12: impulse response of weighted synthesis filter */
  Word16 code[], /* (o) Q12: algebraic (fixed) codebook excitation         */
  Word16 y[],    /* (o) Q11: filtered fixed codebook excitation            */
  Word16 indx[]  /* (o)    : index 5 words: 7,7,7,7,7 = 35 bits            */
);
void Dec_ACELP_12i40_44bits(
  Word16 *index,  /* (i)     : 5 words index (positions & sign)      */
  Word16 cod[]    /* (o) Q13 : algebraic (fixed) codebook excitation */
);
void Dec_ACELP_10i40_35bits(
  Word16 *index,  /* (i)     : 5 words index (positions & sign)      */
  Word16 cod[]    /* (o) Q13 : algebraic (fixed) codebook excitation */
);
/* pitch tracking routine */
void track_pit(Word16 *T0, Word16 *T0_frac, Word16 *prev_pitch,
        Word16 *stat_pitch, Word16 *pitch_sta,  Word16 *frac_sta);
