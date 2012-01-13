/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
    ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
                         Version 1.1 of October 1999
*/
/*
 File : ld8cp.h
 */

/* from ld8e. G.729 Annex E Version 1.2  Last modified: May 1998 */
/* from ld8kd.h G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from ld8k.h G.729 Annex B Version 1.3  Last modified: August 1997 */

/*---------------------------------------------------------------------------*
 * constants for bitstream packing                                           *
 *---------------------------------------------------------------------------*/
#define PRM_SIZE_E_fwd  18       /* Size of vector of analysis parameters.    */
#define PRM_SIZE_E_bwd  16       /* Size of vector of analysis parameters.    */
#define PRM_SIZE_E      18       /* Size of vector of analysis parameters.    */
#define PRM_SIZE_D      10       /* Size of vector of analysis parameters.    */
#define PRM_SIZE_SID    4        /* Size of vector of analysis parameters.    */

#define SERIAL_SIZE_E (116+4)  /* Bits/frame + bfi+ number of speech bits
                                             + bit for mode + protection */

#define RATE_6400       64      /* Low  rate  (6400 bit/s)       */
#define RATE_8000       80      /* Full rate  (8000 bit/s)       */
#define RATE_11800      118     /* High rate (11800 bit/s)       */
#define RATE_SID        15      /* SID                           */
#define RATE_0          0       /* 0 bit/s rate                  */

#define G729D           0      /* Low  rate  (6400 bit/s)       */
#define G729            1      /* Full rate  (8000 bit/s)       */
#define G729E           2      /* High rate (11800 bit/s)       */

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
/* 6.4K GAIN quantizer constants*/
#define NCODE1_B_6K  3             /* number of Codebook-bit                */
#define NCODE2_B_6K  3             /* number of Codebook-bit                */
#define NCODE1_6K (1<<NCODE1_B_6K) /* Codebook 1 size                       */
#define NCODE2_6K (1<<NCODE2_B_6K) /* Codebook 2 size                       */
#define NCAN1_6K  6                /* Pre-selecting order for #1            */
#define NCAN2_6K  6                /* Pre-selecting order for #2            */
#define INV_COEF_6K  -28940        /* Q20                                   */

/*--------------------------------------------------------------------------*
 * Main coder and decoder functions                                         *
 *--------------------------------------------------------------------------*/
void Init_Coder_ld8c(Word16 dtx_enable);

void Coder_ld8c(
  Word16 ana[],     /* (o)     : analysis parameters                        */
 Word16 frame,                   /* input : frame counter */
  Word16 dtx_enable,               /* input : DTX enable flag */
  Word16 rate           /* input   : rate selector/frame  =0 6.4kbps , =1 8kbps,= 2 11.8 kbps*/
);

void Init_Decod_ld8c(void);

void Decod_ld8c(
  Word16 parm[],   /* (i)     : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)      */
  Word16 voicing,  /* (i)     : voicing decision from previous frame       */
  Word16 synth[],  /* (o)     : synthesized speech                         */
  Word16  A_t[],    /* (o)     : decoded LP filter for 2 subframes          */
  Word16 *T0_first, /* (o)     : decoded pitch lag in first subframe        */
  Word16 *stationnary,  /* output:  stationnarity indicator */
  Word16 *m_pst,         /* output:  LPC order */
  Word16 *Vad
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
void Autocorrcp(
  Word16 x[],       /* (i)     : input signal                               */
  Word16 m,         /* (i)     : LPC order                                  */
  Word16 r_h[],     /* (o)     : autocorrelations  (msb)                    */
  Word16 r_l[],     /* (o)     : autocorrelations  (lsb)                    */
  Word16 *exp_R0
);

void Levinsoncp(
  Word16 m,        /* (i)    : LPC order                         */
  Word16 Rh[],      /* (i)     : Rh[m+1] autocorrelation coefficients (msb) */
  Word16 Rl[],      /* (i)     : Rl[m+1] autocorrelation coefficients (lsb) */
  Word16 A[],       /* (o) Q12 : A[m]    LPC coefficients  (m = 10)         */
  Word16 rc[],       /* (o) Q15 : rc[M]   Reflection coefficients.           */
  Word16 old_A[], /* (i/o) Q12 : last stable filter LPC coefficients  */
  Word16 old_rc[], /* (i/o) Q15 : last stable filter Reflection coefficients.         */
  Word16 *Err
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
  Word16 m_pst,          /* input:  LPC order */
  Word16 Vad                /* input : active/inactive flag indicator */
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
   Word16 *c_muting,
   Word16 count_bfi,
   Word16 stationnary
);

/*--------------------------------------------------------------------------*
 * Bitstream functions                                                      *
 *--------------------------------------------------------------------------*/
void bits2prm_ld8c(
    Word16 bits[],  /* I   serial bits (nb_bits)                          */
    Word16 prm[]   /* O   output: decoded parameters (11 parameters)     */
);


void  prm2bits_ld8c(
  Word16 prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
  Word16 bits[]          /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = nbbits */
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
  Word16 *lp_mode, Word16 *lspnew, Word16 *lspold,
  Word16 *bwd_dominant, Word16 prev_lp_mode, Word16 *prev_filter,
  Word16 *C_int, Word16 *glob_stat, Word16 *stat_bwd, Word16 *val_stat_bwd);
void update_bwd( Word16 *lp_mode, Word16 *bwd_dominant, Word16 *C_int,
        Word16 *glob_stat);
Word16 ener_dB(Word16 *synth, Word16 L);
void tst_bwd_dominant(Word16 *high_stat, Word16 mode);

void perc_vare (
  Word16 *gamma1,   /* Bandwidth expansion parameter */
  Word16 *gamma2,   /* Bandwidth expansion parameter */
  Word16  high_stat /* high_stat indication (see file bwfw.c) */
);

 Word16 Pitch_fr3cp( /* (o)     : pitch period.                              */
  Word16 exc[],     /* (i)     : excitation buffer                          */
  Word16 xn[],      /* (i)     : target vector                              */
  Word16 h[],       /* (i) Q12 : impulse response of filters.               */
  Word16 L_subfr,   /* (i)     : length of subframe                         */
  Word16 t0_min,    /* (i)     : minimum value in the searched range.       */
  Word16 t0_max,    /* (i)     : maximum value in the searched range.       */
  Word16 i_subfr,   /* (i)     : indicator for first subframe.              */
  Word16 *pit_frac,  /* (o)     : chosen fraction.                           */
  Word16 rate        /* (i)     : frame rate */
);
Word16 Enc_lag3cp(    /* (o)     : Return index of encoding                   */
  Word16 T0,        /* (i)     : Pitch delay                                */
  Word16 T0_frac,   /* (i)     : Fractional pitch delay                     */
  Word16 *T0_min,   /* (i/o)   : Minimum search delay                       */
  Word16 *T0_max,   /* (i/o)   : Maximum search delay                       */
  Word16 pit_min,   /* (i)     : Minimum pitch delay                        */
  Word16 pit_max,   /* (i)     : Maximum pitch delay                        */
  Word16 pit_flag,   /* (i)     : Flag for 1st subframe                      */
  Word16 rate        /* (i)     : frame rate */
);
void Dec_lag3cp(      /* (o)     : return integer pitch lag                   */
  Word16 index,     /* (i)     : received pitch index                       */
  Word16 pit_min,   /* (i)     : minimum pitch lag                          */
  Word16 pit_max,   /* (i)     : maximum pitch lag                          */
  Word16 i_subfr,   /* (i)     : subframe flag                              */
  Word16 *T0,       /* (o)     : integer part of pitch lag                  */
  Word16 *T0_frac,   /* (o)     : fractional part of pitch lag               */
  Word16 rate        /* (i)     : frame rate */
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


Word16 ACELP_Codebook64( /*(o) : index of pulses positions                  */
  Word16 x[],       /* (i)     : Target vector                              */
  Word16 h[],       /* (i) Q12 : Impulse response of filters                */
  Word16 T0,        /* (i)     : Pitch lag                                  */
  Word16 pitch_sharp,/* (i) Q14: Last quantized pitch gain                  */
  Word16 code[],    /* (o) Q13 : Innovative codebook                        */
  Word16 y[],       /* (o) Q12 : Filtered innovative codebook               */
  Word16 *sign      /* (o)     : Signs of 4 pulses                          */
);
void Decod_ACELP64(
  Word16 sign,      /* (i)     : signs of 4 pulses.                         */
  Word16 index,     /* (i)     : Positions of the 4 pulses.                 */
  Word16 cod[]      /* (o) Q13 : algebraic (fixed) codebook excitation      */
);

void PhDisp(
  Word16 x[],       /* (i)     : input signal                               */
  Word16 y[],       /* (o)     : output signal                              */
  Word16 cbGain,    /* (i) Q1  : codebook gain                              */
  Word16 ltpGain,   /* (i) Q14 : pitch gain                                 */
  Word16 inno[]     /* (i/o)   : innovation vector                          */
);

void Update_PhDisp(
  Word16 ltpGain,   /* (i) Q14 : pitch gain                                 */
  Word16 cbGain     /* (i) Q1  : codebook gain                              */
);




/* pitch tracking routine */
void track_pit(Word16 *T0, Word16 *T0_frac, Word16 *prev_pitch,
        Word16 *stat_pitch, Word16 *pitch_sta,  Word16 *frac_sta);
Word16 Qua_gain_6k(
  Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  Word16 g_coeff[], /* (i)     : Correlations <xn y1> -2<y1 y1>             */
                    /*            <y2,y2>, -2<xn,y2>, 2<y1,y2>              */
  Word16 exp_coeff[],/* (i)    : Q-Format g_coeff[]                         */
  Word16 L_subfr,   /* (i)     : Subframe length.                           */
  Word16 *gain_pit, /* (o) Q14 : Pitch gain.                                */
  Word16 *gain_cod, /* (o) Q1  : Code gain.                                 */
  Word16 tameflag   /* (i)     : flag set to 1 if taming is needed          */
);
void Dec_gain_6k(
  Word16 index,     /* (i)     : Index of quantization.                     */
  Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  Word16 L_subfr,   /* (i)     : Subframe length.                           */
  Word16 bfi,       /* (i)     : Bad frame indicator                        */
  Word16 *gain_pit, /* (o) Q14 : Pitch gain.                                */
  Word16 *gain_cod  /* (o) Q1  : Code gain.                                 */
);


Word16 Random_g729cp(Word16 *seed);

/*--------------------------------------------------------------------------*
 * taming functions                                                         *
 *--------------------------------------------------------------------------*/
void Init_exc_err(void);
Word16 test_err(Word16 t0, Word16 t0_frac);
void update_exc_err(Word16 gain_pit, Word16 t0);


void musdetect( Word16 rate, Word16 r_h, Word16 r_l, Word16 exp_R0, Word16 *rc,
               Word16 *lags, Word16 *pgains, Word16 stat_flg,
               Word16 frm_count, Word16 prev_vad, Word16 *Vad, Word16 Energy_db);
