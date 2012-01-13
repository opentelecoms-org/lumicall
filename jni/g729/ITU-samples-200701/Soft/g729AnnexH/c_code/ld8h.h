/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex H   - Reference C code for fixed point
                         implementation of G.729 Annex H
                         (integration of Annexes D and E)
                         Version 1.1 of October 1999
*/
/*
 File : ld8h.h
 */

/* from ld8e. G.729 Annex E Version 1.2  Last modified: May 1998 */
/* from ld8kd.h G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from ld8k.h G.729 Version 3.3*/

#define RATE_6400       64      /* Low  rate  (6400 bit/s)       */
#define RATE_8000       80      /* Full rate  (8000 bit/s)       */
#define RATE_11800      118     /* High rate (11800 bit/s)       */

#define G729D           0      /* Low  rate  (6400 bit/s)       */
#define G729            1      /* Full rate  (8000 bit/s)       */
#define G729E           2      /* High rate (11800 bit/s)       */

/*--------------------------------------------------------------------------*
 * Main coder and decoder functions                                         *
 *--------------------------------------------------------------------------*/
void Init_Coder_ld8h(void);

void Coder_ld8h(
  Word16 ana[],  /* (o)     : analysis parameters                        */
  Word16 rate    /* input   : rate selector/frame  =0 6.4kbps , =1 8kbps,= 2 11.8 kbps*/
);

void Init_Decod_ld8h(void);

void Decod_ld8h(
  Word16 parm[],   /* (i)     : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)      */
  Word16 voicing,  /* (i)     : voicing decision from previous frame       */
  Word16 synth[],  /* (o)     : synthesized speech                         */
  Word16  A_t[],    /* (o)     : decoded LP filter for 2 subframes          */
  Word16 *T0_first, /* (o)     : decoded pitch lag in first subframe        */
  Word16 *stationnary,  /* output:  stationnarity indicator */
  Word16 *m_pst         /* output:  LPC order */
);

/*--------------------------------------------------------------------------*
 * Bitstream functions                                                      *
 *--------------------------------------------------------------------------*/
void bits2prm_ld8h(
    Word16 bits[],  /* I   serial bits (nb_bits)                          */
    Word16 prm[]   /* O   output: decoded parameters (11 parameters)     */
);


void  prm2bits_ld8h(
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
void Dec_gain_D(
                 Word16 index,        /* (i)     :Index of quantization.         */
                 Word16 code[],       /* (i) Q13 :Innovative vector.             */
                 Word16 L_subfr,      /* (i)     :Subframe length.               */
                 Word16 bfi,          /* (i)     :Bad frame indicator            */
                 Word16 *gain_pit,    /* (o) Q14 :Pitch gain.                    */
                 Word16 *gain_cod     /* (o) Q1  :Code gain.                     */
                 );

/*--------------------------------------------------------------------------*
 * taming functions                                                         *
 *--------------------------------------------------------------------------*/
void Init_exc_err(void);
Word16 test_err(Word16 t0, Word16 t0_frac);
void update_exc_err(Word16 gain_pit, Word16 t0);


void musdetect( Word16 rate, Word16 r_h, Word16 r_l, Word16 exp_R0, Word16 *rc,
               Word16 *lags, Word16 *pgains, Word16 stat_flg,
               Word16 frm_count, Word16 prev_vad, Word16 *Vad, Word16 Energy_db);

