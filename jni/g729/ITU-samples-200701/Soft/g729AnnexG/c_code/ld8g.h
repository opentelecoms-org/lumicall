/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex G  - Reference C code for fixed point
                         implementation of G.729 Annex G
                         (integration of Annexes B and E)
                         Version 1.1 of October 1999
*/
/*
 File : ld8g.h
 */

/* from ld8e. G.729 Annex E Version 1.2  Last modified: May 1998 */
/* from ld8k.h G.729 Annex B Version 1.3  Last modified: August 1997 */

/*---------------------------------------------------------------------------*
 * constants for bitstream packing                                           *
 *---------------------------------------------------------------------------*/
#define PRM_SIZE_SID    4        /* Size of vector of analysis parameters.    */

#define RATE_8000       80      /* Full rate  (8000 bit/s)       */
#define RATE_11800      118     /* High rate (11800 bit/s)       */
#define RATE_SID        15      /* SID                           */
#define RATE_0          0       /* 0 bit/s rate                  */

#define G729            1      /* Full rate  (8000 bit/s)       */
#define G729E           2      /* High rate (11800 bit/s)       */

/*--------------------------------------------------------------------------*
 * Main coder and decoder functions                                         *
 *--------------------------------------------------------------------------*/
void Init_Coder_ld8g(Word16 dtx_enable);

void Coder_ld8g(
  Word16 ana[],     /* (o)     : analysis parameters                        */
 Word16 frame,                   /* input : frame counter */
  Word16 dtx_enable,               /* input : DTX enable flag */
  Word16 rate           /* input   : rate selector/frame  =0 6.4kbps , =1 8kbps,= 2 11.8 kbps*/
);

void Init_Decod_ld8g(void);

void Decod_ld8g(
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
void Autocorrg(
  Word16 x[],       /* (i)     : input signal                               */
  Word16 m,         /* (i)     : LPC order                                  */
  Word16 r_h[],     /* (o)     : autocorrelations  (msb)                    */
  Word16 r_l[],     /* (o)     : autocorrelations  (lsb)                    */
  Word16 *exp_R0
);

void Levinsong(
  Word16 m,        /* (i)    : LPC order                         */
  Word16 Rh[],      /* (i)     : Rh[m+1] autocorrelation coefficients (msb) */
  Word16 Rl[],      /* (i)     : Rl[m+1] autocorrelation coefficients (lsb) */
  Word16 A[],       /* (o) Q12 : A[m]    LPC coefficients  (m = 10)         */
  Word16 rc[],       /* (o) Q15 : rc[M]   Reflection coefficients.           */
  Word16 old_A[], /* (i/o) Q12 : last stable filter LPC coefficients  */
  Word16 old_rc[], /* (i/o) Q15 : last stable filter Reflection coefficients.         */
  Word16 *Err
);

/*--------------------------------------------------------------------------*
 * Postfilter functions                                                     *
 *--------------------------------------------------------------------------*/
void Postg(
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
 * Bitstream functions                                                      *
 *--------------------------------------------------------------------------*/
void bits2prm_ld8g(
    Word16 bits[],  /* I   serial bits (nb_bits)                          */
    Word16 prm[]   /* O   output: decoded parameters (11 parameters)     */
);


void  prm2bits_ld8g(
  Word16 prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
  Word16 bits[]          /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = nbbits */
);

void set_lpc_modeg(Word16 *signal_ptr, Word16 *a_fwd, Word16 *a_bwd,
  Word16 *lp_mode, Word16 *lspnew, Word16 *lspold,
  Word16 *bwd_dominant, Word16 prev_lp_mode, Word16 *prev_filter,
  Word16 *C_int, Word16 *glob_stat, Word16 *stat_bwd, Word16 *val_stat_bwd);
void update_bwd( Word16 *lp_mode, Word16 *bwd_dominant, Word16 *C_int,
        Word16 *glob_stat);

void Cod_cngg(
  Word16 *exc,          /* (i/o) : excitation array                     */
  Word16 pastVad,       /* (i)   : previous VAD decision                */
  Word16 *lsp_old_q,    /* (i/o) : previous quantized lsp               */
  Word16 old_A[],   /* (i/o) Q12 : last stable filter LPC coefficients  */
  Word16 old_rc[],   /* (i/o) Q15 : last stable filter Reflection coefficients.         */
  Word16 *Aq,           /* (o)   : set of interpolated LPC coefficients */
  Word16 *ana,          /* (o)   : coded SID parameters                 */
  Word16 freq_prev[MA_NP][M],
                        /* (i/o) : previous LPS for quantization        */
  Word16 *seed          /* (i/o) : random generator seed                */
);
void vadg(
         Word16 rc,
         Word16 *lsf, 
         Word16 *r_h,
         Word16 *r_l,
         Word16 exp_R0,
         Word16 *sigpp,
         Word16 frm_count,
         Word16 prev_marker,
         Word16 pprev_marker,
         Word16 *marker,
         Word16 *ENERGY_db);

void musdetect( Word16 rate, Word16 r_h, Word16 r_l, Word16 exp_R0,
    Word16 *rc, Word16 *lags, Word16 *pgains, Word16 stat_flg,
   Word16 frm_count, Word16 prev_vad, Word16 *Vad, Word16 Energy_db);
