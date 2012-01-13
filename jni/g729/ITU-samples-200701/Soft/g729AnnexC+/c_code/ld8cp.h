/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/
/*
 File : LD8CP.H
*/

/*--------------------------------------------------------------*
 * LD8CP.H                                                       *
 * ~~~~~~                                                       *
 *--------------------------------------------------------------*/

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

/*--------------------------------------------------------------*
 * Function prototypes and constants use in G.729E              *
 *                                                              *
 *--------------------------------------------------------------*/

/* backward LPC analysis parameters */
#define M_BWD       30         /* Order of Backward LP filter.              */
#define M_BWDP1     (M_BWD+1)  /* Order of Backward LP filter + 1           */
#define NRP         35
#define MEM_SYN_BWD     (M_BWD + NRP)
#define N1              (M_BWD + L_FRAME)
#define L_ANA_BWD       (L_FRAME + MEM_SYN_BWD)
#define L_ANA_BWD_M1    (L_ANA_BWD - 1)
#define W_FACT  (FLOAT)0.31640625 /* 10368 */
#define GAMMA_BWD (FLOAT)0.98 /* 32113 */

/* short term pst parameters :                                              */
#define GAMMA1_PST_E  (FLOAT)0.7 /* denominator weighting factor */
#define GAMMA2_PST_E  (FLOAT)0.65 /* numerator  weighting factor */
#define LONG_H_ST_E   32        /* impulse response length                   */
#define GAMMA_HARM_E (FLOAT)0.25
#define GAMMA_HARM (FLOAT)0.5

/* ACELP codebooks parameters */
#define NB_TRACK  5
#define Q15_1_5   (FLOAT)0.2

/* Bw / Fw constants */
#define THRES_ENERGY (FLOAT)40.
#define TH1 (FLOAT)1.
#define TH2 (FLOAT)2.
#define TH3 (FLOAT)3.
#define TH4 (FLOAT)4.
#define TH5 (FLOAT)4.7
#define GAP_FACT (FLOAT)0.000114375
#define INV_LOG2 (FLOAT) (1./log10(2.))

/*--------------------------------------------------------------------------*
 *       6.4kbps                                                            *
 *--------------------------------------------------------------------------*/

#ifndef max
#define max(a, b)     ((a) > (b) ? (a) : (b))
#endif

#define BPS_8K        0        /* Indicates 8kbps mode                      */
#define BPS_6K        1        /* Indicates 6.4kbps mode                    */

#define SIZE_WORD_6K (short)64 /* number of speech bits                    */

#define PRM_SIZE_6K   10       /* Size of vector of analysis parameters.    */
#define SERIAL_SIZE_6K (64+2)  /* Bits/frame + bfi+ number of speech bits   */

#define NB_PULSES_6K  2            /* number of pulses */
#define NC1_B_6K    4          /* number of second stage bits higher        */
#define NC1_6K   (1<<NC1_B_6K) /* number of entries in second stage (higher)*/

#define NCODE1_B_6K  3             /* number of Codebook-bit                */
#define NCODE2_B_6K  3             /* number of Codebook-bit                */
#define NCODE1_6K (1<<NCODE1_B_6K) /* Codebook 1 size                       */
#define NCODE2_6K (1<<NCODE2_B_6K) /* Codebook 2 size                       */
#define NCAN1_6K  6                /* Pre-selecting order for #1            */
#define NCAN2_6K  6                /* Pre-selecting order for #2            */
#define INV_COEF_6K  ((F)-0.027599)

#define GAIN_PIT_MAX_6K (F)1.4     /* maximum adaptive codebook gain        */

/*--------------------------------------------------------------------------*
 *       VAD                                                                *
 *--------------------------------------------------------------------------*/
#define EPSI            (F)1.0e-38   /* very small positive floating point number      */
/*--------------------------------------------------------------------------*
 * Main coder and decoder functions                                         *
 *--------------------------------------------------------------------------*/
void  init_coder_ld8c(int dtx_enable);
void coder_ld8c(
 int ana[],                   /* output: analysis parameters */
 int frame,                   /* input : frame counter */
 int dtx_enable,               /* input : VAD enable flag */
 int rate
);

void  init_decod_ld8c(void);
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
);
/*--------------------------------------------------------------------------*
 * bitstream packing VQ functions.                                          *
 *--------------------------------------------------------------------------*/
void  prm2bits_ld8c(int prm[], INT16 bits[]);
void  bits2prm_ld8c(INT16 bits[], int prm[]);

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
FLOAT levinsone(
  int m,         /* (i)  : LPC order                         */
  FLOAT *r,     /* (i)  : r[m+1] autocorrelation coefficients */
  FLOAT *A,     /* (o)  : A[m]    LPC coefficients  (m = 10)         */
  FLOAT *rc,    /* (o)  : rc[M]   Reflection coefficients.           */
  FLOAT *old_A, /* (i/o) : last stable filter LPC coefficients  */
  FLOAT *old_rc /* (i/o) : last stable filter Reflection coefficients.         */
);

void residue(
  int m,        /* (i)    : LPC order                         */
  FLOAT a[],    /* (i)  : prediction coefficients                     */
  FLOAT x[],    /* (i)     : speech (values x[-m..-1] are needed         */
  FLOAT y[],    /* (o)     : residual signal                             */
  int lg      /* (i)     : size of filtering                           */
);
void syn_filte(
  int m,        /* (i)    : LPC order                         */
  FLOAT a[],     /* (i)  : a[m+1] prediction coefficients   (m=10)  */
  FLOAT x[],     /* (i)     : input signal                             */
  FLOAT y[],     /* (o)     : output signal                            */
  int lg,      /* (i)     : size of filtering                        */
  FLOAT mem[],   /* (i/o)   : memory associated with this filtering.   */
  int update   /* (i)     : 0=no update, 1=update of memory.         */
);

/*--------------------------------------------------------------------------*
 * LSP VQ functions.                                                        *
 *--------------------------------------------------------------------------*/
void  lsp_az(FLOAT *lsp, FLOAT *a);
void qua_lspe(
  FLOAT lsp[],     /* (i)  : Unquantized LSP                            */
  FLOAT lsp_q[],   /* (o)  : Quantized LSP                              */
  int ana[],      /* (o)     : indexes                                    */
  FLOAT freq_prev[MA_NP][M],    /* (i)  : previous LSP MA vector        */
  FLOAT freq_cur[]   /* (o)  : current LSP MA vector        */
);
void lsp_encw_resete(
  FLOAT freq_prev[MA_NP][M]    /* (i)  : previous LSP MA vector        */
);
void lsp_stability(FLOAT  buf[]);
void lsp_prev_compose(FLOAT lsp_ele[],FLOAT lsp[],FLOAT fg[][M],
            FLOAT freq_prev[][M], FLOAT fg_sum[]);
void lsp_qua_cse(
  FLOAT flsp_in[M],    /* (i)  : Original LSP parameters    */
  FLOAT lspq_out[M],   /* (o)  : Quantized LSP parameters   */
  int *code,         /* (o)     : codes of the selected LSP  */
  FLOAT freq_prev[MA_NP][M],    /* (i)  : previous LSP MA vector        */
  FLOAT freq_cur[]     /* (o)  : current LSP MA vector        */
);
void lsp_get_quante(
  FLOAT lspcb1[][M],      /* (i)  : first stage LSP codebook      */
  FLOAT lspcb2[][M],      /* (i)  : Second stage LSP codebook     */
  int code0,               /* (i)     : selected code of first stage  */
  int code1,               /* (i)     : selected code of second stage */
  int code2,               /* (i)     : selected code of second stage */
  FLOAT fg[][M],          /* (i)  : MA prediction coef.           */
  FLOAT freq_prev[][M],   /* (i)  : previous LSP vector           */
  FLOAT lspq[],              /* (o)  : quantized LSP parameters      */
  FLOAT fg_sum[],             /* (i)  : present MA prediction coef.   */
  FLOAT freq_cur[]             /* (i)  : present MA prediction coef.   */
);
void relspwede(
  FLOAT lsp[],                     /* (i)  : unquantized LSP parameters */
  FLOAT wegt[],                    /* (i) norm: weighting coefficients     */
  FLOAT lspq[],                    /* (o)  : quantized LSP parameters   */
  FLOAT lspcb1[][M],            /* (i)  : first stage LSP codebook   */
  FLOAT lspcb2[][M],            /* (i)  : Second stage LSP codebook  */
  FLOAT fg[MODE][MA_NP][M],     /* (i)  : MA prediction coefficients */
  FLOAT freq_prev[MA_NP][M],    /* (i)  : previous LSP vector        */
  FLOAT fg_sum[MODE][M],        /* (i)  : present MA prediction coef.*/
  FLOAT fg_sum_inv[MODE][M],    /* (i)  : inverse coef.              */
  int code_ana[],                 /* (o)     : codes of the selected LSP  */
  FLOAT freq_cur[]             /* (o)  : current LSP MA vector        */
);
void get_wegt( FLOAT flsp[], FLOAT wegt[] );
void d_lspe(
  int prm[],          /* (i)     : indexes of the selected LSP */
  FLOAT lsp_q[],        /* (o)  : Quantized LSP parameters    */
  int erase,           /* (i)     : frame erase information     */
  FLOAT freq_prev[MA_NP][M],    /* (i/o)  : previous LSP MA vector        */
  FLOAT prev_lsp[],            /* (i/o)  : previous LSP vector        */
  int *prev_ma               /* (i/o) previous MA prediction coef.*/
);
void lsp_decw_resete(
  FLOAT freq_prev[MA_NP][M],    /* (o)  : previous LSP MA vector        */
  FLOAT prev_lsp[],            /* (o)  : previous LSP vector        */
  int *prev_ma               /* previous MA prediction coef.*/
);
/*--------------------------------------------------------------------------*
 *       LTP prototypes                                                     *
 *--------------------------------------------------------------------------*/
int   pitch_fr3cp(FLOAT exc[], FLOAT xn[], FLOAT h[], int l_subfr,
    int t0_min, int t0_max, int i_subfr, int *pit_frac, int rate);
int enc_lag3cp( int T0, int T0_frac, int *T0_min, int *T0_max, int pit_min,
      int pit_max, int pit_flag, int rate);
void dec_lag3cp( int index, int pit_min, int pit_max, int i_subfr,
  int *T0, int *T0_frac, int rate);

/*--------------------------------------------------------------------------*
 * Postfilter functions                                                     *
 *--------------------------------------------------------------------------*/

void poste(
 int   t0,             /* input : pitch delay given by coder */
 FLOAT *signal_ptr,    /* input : input signal (pointer to current subframe */
 FLOAT *coeff,         /* input : LPC coefficients for current subframe */
 FLOAT *sig_out,       /* output: postfiltered output */
 int   *vo,            /* output: voicing decision 0 = uv,  > 0 delay */
 FLOAT gamma1,         /* input: short term postfilt. den. weighting factor*/
 FLOAT gamma2,         /* input: short term postfilt. num. weighting factor*/
 FLOAT gamma_harm,     /* input: long term postfilter weighting factor*/
 int   long_h_st,      /* input: impulse response length*/
 int   m_pst,          /* input:  LPC order */
 int Vad               /* input : VAD information (frame type)  */
);

/*--------------------------------------------------------------------------*
 * protypes of functions  containing G729 source code + specific G729E code *
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
 * gain VQ functions.                                                       *
 *--------------------------------------------------------------------------*/
void dec_gaine(
   int index,        /* (i)    :Index of quantization.         */
   FLOAT code[],       /* (i)  :Innovative vector.             */
   int L_subfr,      /* (i)    :Subframe length.               */
   int bfi,          /* (i)    :Bad frame indicator            */
   FLOAT *gain_pit,    /* (o)  :Pitch gain.                    */
   FLOAT *gain_cod,     /* (o) :Code gain.                     */
   int rate,             /* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps*/
   FLOAT gain_pit_mem,
   FLOAT gain_cod_mem,
   FLOAT  *c_muting,
   int count_bfi,
   int stationnary
);

/*--------------------------------------------------------------------------*
 * functions  specific to G729E                                             *
 *--------------------------------------------------------------------------*/
/* backward LPC analysis and switch forward/backward */
void autocorr_hyb_window(
    FLOAT  *x,         /* Input speech signal  */
    FLOAT  *r_bwd,        /* (out)    Autocorrelations    */
    FLOAT *rexp        /* (in/out) */
);
void lag_window_bwd(FLOAT *r_bwd);
void int_bwd(FLOAT *a_bwd, FLOAT *prev_filter, FLOAT *C_int );

void set_lpc_mode(FLOAT *signal_ptr, FLOAT *a_fwd, FLOAT *a_bwd,
  int *mode, FLOAT *lsp_new, FLOAT *lsp_old, int *bwd_dominant,
  int prev_mode, FLOAT *prev_filter, FLOAT *C_int,
  INT16 *glob_stat, INT16 *stat_bwd, INT16 *val_stat_bwd);
void update_bwd(int *mode,        /* O  Backward / forward Indication */
                int *bwd_dominant,/* O   Bwd dominant mode indication */
                FLOAT *C_int,       /*I/O filter interpolation parameter */
                INT16 *glob_stat   /* I/O Mre of global stationnarity */
);
FLOAT ener_dB(FLOAT *synth, int L);
void tst_bwd_dominant(int *high_stat, int mode);

void perc_vare (
  FLOAT *gamma1,   /* Bandwidth expansion parameter */
  FLOAT *gamma2,   /* Bandwidth expansion parameter */
  int  high_stat /* high_stat indication (see file bwfw.c) */
);

/*--------------------------------------------------------------------------*
 * G729E fixed (ACELP) codebook excitation.                                               *
 *--------------------------------------------------------------------------*/

void ACELP_12i40_44bits(
  FLOAT x[],    /* (i)  : target vector                                 */
  FLOAT cn[],   /* (i)  : residual after long term prediction           */
  FLOAT H[],    /* (i) : impulse response of weighted synthesis filter */
  FLOAT code[], /* (o) : algebraic (fixed) codebook excitation         */
  FLOAT y[],    /* (o) : filtered fixed codebook excitation            */
  int indx[]  /* (o)    : index 5 words: 13,10,7,7,7 = 44 bits          */
);
void ACELP_10i40_35bits(
  FLOAT x[],    /* (i)  : target vector                                 */
  FLOAT cn[],   /* (i)  : residual after long term prediction           */
  FLOAT H[],    /* (i) : impulse response of weighted synthesis filter */
  FLOAT code[], /* (o) : algebraic (fixed) codebook excitation         */
  FLOAT y[],    /* (o) : filtered fixed codebook excitation            */
  int indx[]  /* (o)    : index 5 words: 7,7,7,7,7 = 35 bits            */
);
void dec_ACELP_12i40_44bits(
  int *index,  /* (i)     : 5 words index (positions & sign)      */
  FLOAT cod[]    /* (o)  : algebraic (fixed) codebook excitation */
);
void dec_ACELP_10i40_35bits(
  int *index,  /* (i)     : 5 words index (positions & sign)      */
  FLOAT cod[]    /* (o)  : algebraic (fixed) codebook excitation */
);
/* pitch tracking routine */
void track_pit(int *T0, int *T0_frac, int *prev_pitch,
        int *stat_pitch, int *pitch_sta,  int *frac_sta);
/*--------------------------------------------------------------------------*
 * G729D fixed (ACELP) codebook excitation.                                               *
 *--------------------------------------------------------------------------*/
int ACELP_codebook64(   /* (o)     :Index of pulses positions    */
  FLOAT x[],            /* (i)     :Target vector                */
  FLOAT h[],            /* (i)     :Impulse response of filters  */
  int   t0,             /* (i)     :Pitch lag                    */
  FLOAT pitch_sharp,    /* (i)     :Last quantized pitch gain    */
  FLOAT code[],         /* (o)     :Innovative codebook          */
  FLOAT y[],            /* (o)     :Filtered innovative codebook */
  int *sign             /* (o)     :Signs of 4 pulses            */
);
void decod_ACELP64(
 int sign,              /* input : signs of 2 pulses     */
 int index,             /* input : positions of 2 pulses */
 FLOAT cod[]            /* output: innovative codevector */
);
/*--------------------------------------------------------------------------*
 * G729D gain                                                               *
 *--------------------------------------------------------------------------*/
int qua_gain_6k(        /* output: quantizer index                   */
  FLOAT code[],         /* input : fixed codebook vector             */
  FLOAT *g_coeff,       /* input : correlation factors               */
  int l_subfr,          /* input : fcb vector length                 */
  FLOAT *gain_pit,      /* output: quantized acb gain                */
  FLOAT *gain_code,     /* output: quantized fcb gain                */
  int tameflag          /* input : flag set to 1 if taming is needed */
);
void dec_gain_6k(
 int index,             /* input : quantizer index              */
 FLOAT code[],          /* input : fixed code book vector       */
 int l_subfr,           /* input : subframe size                */
 int bfi,               /* input : bad frame indicator good = 0 */
 FLOAT *gain_pit,       /* output: quantized acb gain           */
 FLOAT *gain_code       /* output: quantized fcb gain           */
);
/*--------------------------------------------------------------------------*
 * G729D gain  phase dispersion                                             *
 *--------------------------------------------------------------------------*/
void Update_PhDisp(
    FLOAT ltpGain,   /* (i)  : pitch gain                  */
    FLOAT cbGain     /* (i)  : codebook gain               */
);
void PhDisp(
    FLOAT x[],       /* input : excitation signal                */
    FLOAT x_phdisp[],/* output : excitation signal after phase dispersion */
    FLOAT cbGain,
    FLOAT ltpGainQ,
    FLOAT inno[]
);

/*--------------------------------------------------------------------------*
 * Prototypes for auxiliary functions                                       *
 *--------------------------------------------------------------------------*/
void fwrite16(  FLOAT *data, int length, FILE *fp);
INT16 random_g729c(INT16 *seed);
void dvsub(FLOAT *in1, FLOAT *in2, FLOAT *out, INT16 npts);
FLOAT dvdot(FLOAT *in1, FLOAT *in2, INT16 npts);
void dvwadd(FLOAT *in1, FLOAT scalar1, FLOAT *in2, FLOAT scalar2,
             FLOAT *out, INT16 npts);
void dvsmul(FLOAT *in, FLOAT scalar, FLOAT *out, INT16 npts);

void musdetect( int rate, FLOAT Energy, FLOAT *rc, int *lags, FLOAT *pgains,
             int stat_flg, int frm_count, int prev_vad, int *Vad, 
             FLOAT Energy_db);
