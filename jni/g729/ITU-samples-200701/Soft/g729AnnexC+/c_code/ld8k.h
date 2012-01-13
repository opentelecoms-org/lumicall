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
 File : LD8K.H
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/


/*---------------------------------------------------------------------------
 * ld8k.h - include file for all ITU-T 8 kb/s CELP coder routines
 *---------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef PI
#undef PI
#endif
#ifdef PI2
#undef PI2
#endif
#define PI              (F)3.14159265358979323846
#define PI2             (F)6.283185307
#define FLT_MAX_G729         (F)1.e38   /* largest floating point number             */
#define FLT_MIN_G729         -FLT_MAX_G729    /* largest floating point number             */

#define L_TOTAL         240     /* Total size of speech buffer               */
#define L_FRAME         80      /* LPC update frame size                     */
#define L_SUBFR         40      /* Sub-frame size                            */

/*---------------------------------------------------------------------------*
 * constants for bitstream packing                                           *
 *---------------------------------------------------------------------------*/
#define BIT_1     (INT16)0x0081 /* definition of one-bit in bit-stream      */
#define BIT_0     (INT16)0x007f /* definition of zero-bit in bit-stream      */
#define SYNC_WORD (INT16)0x6b21 /* definition of frame erasure flag          */
#define SIZE_WORD       80  /* size of bitstream frame */
#define PRM_SIZE        11      /* number of parameters per 10 ms frame      */
#define SERIAL_SIZE     82      /* bits per frame                            */

/*---------------------------------------------------------------------------*
 * constants for lpc analysis and lsp quantizer                              *
 *---------------------------------------------------------------------------*/
#define L_WINDOW        240     /* LPC analysis window size                  */
#define L_NEXT          40      /* Samples of next frame needed for LPC ana. */

#define M               10      /* LPC order                                 */
#define MP1            (M+1)    /* LPC order+1                               */
#define GRID_POINTS     60      /* resolution of lsp search                  */

#define MA_NP           4       /* MA prediction order for LSP               */
#define MODE            2       /* number of modes for MA prediction         */
#define NC0_B           7       /* number of bits in first stage             */
#define NC0          (1<<NC0_B) /* number of entries in first stage          */
#define NC1_B           5       /* number of bits in second stage            */
#define NC1          (1<<NC1_B) /* number of entries in second stage         */
#define NC              (M/2)   /* LPC order / 2                            */

#define L_LIMIT         (F)0.005   /*  */
#define M_LIMIT         (F)3.135   /*  */
#define GAP1            (F)0.0012  /*  */
#define GAP2            (F)0.0006  /*  */
#define GAP3            (F)0.0392  /*  */
#define PI04            PI*(F)0.04   /* pi*0.04 */
#define PI92            PI*(F)0.92   /* pi*0.92 */
#define CONST12         (F)1.2

/*-------------------------------------------------------------------------
 *  pwf constants
 *-------------------------------------------------------------------------
 */

#define THRESH_L1   (F)-1.74
#define THRESH_L2   (F)-1.52
#define THRESH_H1   (F)0.65
#define THRESH_H2   (F)0.43
#define GAMMA1_0    (F)0.98
#define GAMMA2_0_H  (F)0.7
#define GAMMA2_0_L  (F)0.4
#define GAMMA1_1    (F)0.94
#define GAMMA2_1    (F)0.6
#define ALPHA       (F)-6.0
#define BETA        (F)1.0

/*----------------------------------------------------------------------------
 *  Constants for long-term predictor
 *----------------------------------------------------------------------------
 */
#define PIT_MIN         20      /* Minimum pitch lag in samples              */
#define PIT_MAX         143     /* Maximum pitch lag in samples              */
#define L_INTERPOL      (10+1)  /* Length of filter for interpolation.       */
#define L_INTER10       10      /* Length for pitch interpolation            */
#define L_INTER4        4       /* upsampling ration for pitch search        */
#define UP_SAMP         3       /* resolution of fractional delays           */
#define THRESHPIT    (F)0.85    /* Threshold to favor smaller pitch lags     */
#define GAIN_PIT_MAX (F)1.2     /* maximum adaptive codebook gain            */
#define FIR_SIZE_ANA (UP_SAMP*L_INTER4+1)
#define FIR_SIZE_SYN (UP_SAMP*L_INTER10+1)

/*---------------------------------------------------------------------------*
 * constants for fixed codebook                                              *
 *---------------------------------------------------------------------------*/
#define DIM_RR  616 /* size of correlation matrix                            */
#define NB_POS  8   /* Number of positions for each pulse                    */
#define STEP    5   /* Step betweem position of the same pulse.              */
#define MSIZE   64  /* Size of vectors for cross-correlation between 2 pulses*/

#define SHARPMAX        (F)0.7945  /* Maximum value of pitch sharpening */
#define SHARPMIN        (F)0.2     /* minimum value of pitch sharpening */

 /*--------------------------------------------------------------------------*
  * Example values for threshold and approximated worst case complexity:     *
  *                                                                          *
  *     threshold=0.40   maxtime= 75   extra=30   Mips =  6.0                *
  *--------------------------------------------------------------------------*/
#define THRESHFCB       (F)0.40    /*  */
#define MAX_TIME        75      /*  */

/*--------------------------------------------------------------------------*
 * Constants for taming procedure.                           *
 *--------------------------------------------------------------------------*/
#define GPCLIP      (F)0.95     /* Maximum pitch gain if taming is needed */
#define GPCLIP2     (F)0.94     /* Maximum pitch gain if taming is needed */
#define GP0999      (F)0.9999   /* Maximum pitch gain if taming is needed    */
#define THRESH_ERR  (F)60000.   /* Error threshold taming    */
#define INV_L_SUBFR (FLOAT) ((F)1./(FLOAT)L_SUBFR) /* =0.025 */
/*-------------------------------------------------------------------------
 *  gain quantizer  constants
 *-------------------------------------------------------------------------
 */
#define MEAN_ENER        (F)36.0   /* average innovation energy */
#define NCODE1_B  3                /* number of Codebook-bit                */
#define NCODE2_B  4                /* number of Codebook-bit                */
#define NCODE1    (1<<NCODE1_B)    /* Codebook 1 size                       */
#define NCODE2    (1<<NCODE2_B)    /* Codebook 2 size                       */
#define NCAN1            4      /* Pre-selecting order for #1 */
#define NCAN2            8      /* Pre-selecting order for #2 */
#define INV_COEF   (F)-0.032623

/*---------------------------------------------------------------------------
 * Constants for postfilter
 *---------------------------------------------------------------------------
 */
         /* INT16 term pst parameters :  */
#define GAMMA1_PST      (F)0.7     /* denominator weighting factor           */
#define GAMMA2_PST      (F)0.55    /* numerator  weighting factor            */
#define LONG_H_ST       20      /* impulse response length                   */
#define GAMMA3_PLUS     (F)0.2     /* tilt weighting factor when k1>0        */
#define GAMMA3_MINUS    (F)0.9     /* tilt weighting factor when k1<0        */

/* long term pst parameters :   */
#define L_SUBFRP1 (L_SUBFR + 1) /* Sub-frame size + 1                        */
#define F_UP_PST        8       /* resolution for fractionnal delay          */
#define LH2_S           4       /* length of INT16 interp. subfilters        */
#define LH2_L           16      /* length of long interp. subfilters         */
#define THRESCRIT       (F)0.5     /* threshold LT pst switch off            */
#define GAMMA_G         (F)0.5     /* LT weighting factor                    */
#define AGC_FAC         (F)0.9875  /* gain adjustment factor                 */

#define AGC_FAC1         ((F)1. - AGC_FAC)    /* gain adjustment factor                 */
#define LH_UP_S         (LH2_S/2)
#define LH_UP_L         (LH2_L/2)
#define LH2_L_P1    (LH2_L + 1)
#define MIN_GPLT    ((F)1. / ((F)1. + GAMMA_G))  /* LT gain minimum          */

/* Array sizes */
#define MEM_RES2 (PIT_MAX + 1 + LH_UP_L)
#define SIZ_RES2 (MEM_RES2 + L_SUBFR)
#define SIZ_Y_UP  ((F_UP_PST-1) * L_SUBFRP1)
#define SIZ_TAB_HUP_L ((F_UP_PST-1) * LH2_L)
#define SIZ_TAB_HUP_S ((F_UP_PST-1) * LH2_S)


/*--------------------------------------------------------------------------*
 * Main coder and decoder functions                                         *
 *--------------------------------------------------------------------------*/
void  init_coder_ld8k(void);
void  coder_ld8k(int *);

void  init_decod_ld8k(void);
void  decod_ld8k( int parm[], int voi, FLOAT synth[], FLOAT Az_dec[], int *t0);

/*--------------------------------------------------------------------------*
 * Pre and post-process functions                                           *
 *--------------------------------------------------------------------------*/
void init_pre_process( void);
void pre_process(FLOAT signal[], int lg);

void init_post_process( void);
void post_process(FLOAT signal[], int lg );


/*--------------------------------------------------------------------------*
 * LPC analysis and filtering                                               *
 *--------------------------------------------------------------------------*/
void autocorr( FLOAT *x, int m, FLOAT *r);
void lag_window( int m, FLOAT r[]);
FLOAT levinson (FLOAT *r, FLOAT *a, FLOAT *r_c);
void az_lsp( FLOAT a[], FLOAT lsp[], FLOAT old_lsp[]);
void qua_lsp( FLOAT lsp[], FLOAT lsp_q[], int ana[]);

void lsf_lsp( FLOAT *lsf, FLOAT *lsp, int m);
void lsp_lsf(FLOAT *lsp, FLOAT *lsf, int m);
void int_lpc(FLOAT lsp_old[], FLOAT lsp_new[], FLOAT lsf_int[],
    FLOAT lsf_new[], FLOAT A_t[]);
void int_qlpc( FLOAT lsp_old[], FLOAT lsp_new[], FLOAT Az[]);

/*--------------------------------------------------------------------------*
 * Prototypes of LSP VQ functions                                           *
 *--------------------------------------------------------------------------*/
void lsp_expand_1( FLOAT buf[], FLOAT c);
void lsp_expand_2( FLOAT buf[], FLOAT c);
void lsp_expand_1_2( FLOAT buf[], FLOAT c);
void lsp_decw_reset(void);
void lsp_encw_reset(void);
void lsp_prev_update( FLOAT lsp_ele[M], FLOAT freq_prev[MA_NP][M]);
void lsp_prev_extract( FLOAT lsp[M], FLOAT lsp_ele[M], FLOAT fg[MA_NP][M],
  FLOAT freq_prev[MA_NP][M], FLOAT fg_sum_inv[M]);
void lsp_get_quant( FLOAT lspcb1[][M], FLOAT lspcb2[][M], int code0,
    int code1, int code2, FLOAT fg[][M], FLOAT freq_prev1[][M],
    FLOAT lspq[],FLOAT fg_sum[] );
void d_lsp( int index[], FLOAT lsp_new[], int bfi);

/*--------------------------------------------------------------------------*
 *       PWF prototypes                                                     *
 *--------------------------------------------------------------------------*/
void  perc_var(FLOAT *gamma1, FLOAT *gamma2, FLOAT *lsfint, FLOAT *lsfnew,
                  FLOAT *r_c);
void  weight_az(FLOAT *a, FLOAT gamma, int m, FLOAT *ap );

/*-------------------------------------------------------------------------
 * Prototypes of general signal processing routines.
 *-------------------------------------------------------------------------
 */
void  convolve(FLOAT x[], FLOAT h[], FLOAT y[], int L);
void  residu(FLOAT *a, FLOAT *x, FLOAT *y, int l);
void  syn_filt(FLOAT a[], FLOAT x[], FLOAT y[],
                int l, FLOAT mem[], int update_m);

/*--------------------------------------------------------------------------*
 *       LTP prototypes                                                     *
 *--------------------------------------------------------------------------*/
int   pitch_fr3(FLOAT exc[], FLOAT xn[], FLOAT h[], int l_subfr,
    int t0_min, int t0_max, int i_subfr, int *pit_frac);
int   pitch_ol(FLOAT signal[], int pit_min, int pit_max, int L_frame);
int enc_lag3( int T0, int T0_frac, int *T0_min, int *T0_max, int pit_min,
      int pit_max, int pit_flag);
void dec_lag3( int index, int pit_min, int pit_max, int i_subfr,
  int *T0, int *T0_frac);
void  pred_lt_3(FLOAT exc[], int t0, int frac, int L);
int   parity_pitch(int pitch_i);
int   check_parity_pitch(int pitch_i, int parity);
FLOAT g_pitch( FLOAT xn[], FLOAT y1[], FLOAT g_coeff[], int L_subfr);

/*--------------------------------------------------------------------------*
 * fixed codebook excitation.                                               *
 *--------------------------------------------------------------------------*/
void cor_h_x( FLOAT h[], FLOAT X[], FLOAT D[]);
int ACELP_codebook( FLOAT x[], FLOAT h[], int T0, FLOAT pitch_sharp,
    int i_subfr, FLOAT code[], FLOAT y[], int *sign);
void  decod_ACELP(int signs, int positions, FLOAT cod[]);


/*--------------------------------------------------------------------------*
 * gain VQ functions.                                                       *
 *--------------------------------------------------------------------------*/
int   qua_gain(FLOAT code[], FLOAT *coeff, int lcode, FLOAT *gain_pit,
    FLOAT *gain_code, int taming);
void  dec_gain(int indice, FLOAT code[], int lcode, int bfi,
        FLOAT *gain_pit, FLOAT *gain_code);
void gain_predict( FLOAT past_qua_en[], FLOAT code[], int l_subfr,
    FLOAT *gcode0);
void gain_update( FLOAT past_qua_en[], FLOAT g_code);
void gain_update_erasure(FLOAT *past_qua_en);
void  corr_xy2(FLOAT xn[], FLOAT y1[], FLOAT y2[], FLOAT g_coeff[]);

/*--------------------------------------------------------------------------*
 * bitstream packing VQ functions.                                          *
 *--------------------------------------------------------------------------*/
void  prm2bits_ld8k(int prm[], INT16 bits[]);
void  bits2prm_ld8k(INT16 bits[], int prm[]);

/*--------------------------------------------------------------------------*
 * postfilter  functions.                                                   *
 *--------------------------------------------------------------------------*/
void init_post_filter(void);
void post(int  t0, FLOAT *syn, FLOAT *a_t, FLOAT *pst, int *sf_voic);

/*------------------------------------------------------------*
 * prototypes for taming procedure.                           *
 *------------------------------------------------------------*/
void   init_exc_err(void);
void   update_exc_err(FLOAT gain_pit, int t0);
int test_err(int t0, int t0_frac);

/*--------------------------------------------------------------------------*
 * Prototypes for auxiliary functions                                       *
 *--------------------------------------------------------------------------*/
void fwrite16(  FLOAT *data, int length, FILE *fp);
INT16 random_g729( void);
void set_zero( FLOAT  x[], int l );
void copy( FLOAT  x[], FLOAT  y[], int L);

