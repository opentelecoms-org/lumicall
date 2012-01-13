/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* version 1.3 modified september 1999 */
/* G.729 with ANNEX D   Version 1.1    Last modified: March 1998 */
void Update64(
  Word16 ltpGain,   /* (i) Q14 : pitch gain                                 */
  Word16 cbGain     /* (i) Q1  : codebook gain                              */
);

void Coder_ld8kD(
  Word16 ana[],     /* (o)     : analysis parameters                        */
  Word16 synth[]    /* (o)     : local synthesis                            */
);

void Decod_ld8kD(
  Word16 parm[],   /* (i)     : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)       */
  Word16 voicing,  /* (i)     : voicing decision from previous frame       */
  Word16 synth[],  /* (o)     : synthesized speech                         */
  Word16 A_t[],    /* (o)     : decoded LP filter for 2 subframes          */
  Word16 *T0_first /* (o)     : decoded pitch lag in first subframe        */
);

void Syn_filt64(
  Word16 a[],       /* (i) Q12 : a[m+1] prediction coefficients   (m=10)    */
  Word16 x[],       /* (i)     : input signal                               */
  Word16 y[],       /* (o)     : output signal                              */
  Word16 lg,        /* (i)     : size of filtering                          */
  Word16 mem[],     /* (i/o)   : memory associated with this filtering.     */
  Word16 update,    /* (i)     : 0=no update, 1=update of memory.           */
  Word16 cbGain,    /* (i) Q1  : codebook gain                              */
  Word16 ltpGain,   /* (i) Q14 : pitch gain                                 */
  Word16 inno[]     /* (i/o)   : innovation vector                          */
);

Word16 ACELP_CodebookD( /*(o) : index of pulses positions                  */
  Word16 x[],       /* (i)     : Target vector                              */
  Word16 h[],       /* (i) Q12 : Impulse response of filters                */
  Word16 T0,        /* (i)     : Pitch lag                                  */
  Word16 pitch_sharp,/* (i) Q14: Last quantized pitch gain                  */
  Word16 i_subfr,   /* (i)     : Indicator of 1st subframe,                 */
  Word16 code[],    /* (o) Q13 : Innovative codebook                        */
  Word16 y[],       /* (o) Q12 : Filtered innovative codebook               */
  Word16 *sign      /* (o)     : Signs of 4 pulses                          */
);

Word16 Pitch_fr3D( /* (o)     : pitch period.                              */
  Word16 exc[],     /* (i)     : excitation buffer                          */
  Word16 xn[],      /* (i)     : target vector                              */
  Word16 h[],       /* (i) Q12 : impulse response of filters.               */
  Word16 L_subfr,   /* (i)     : length of subframe                         */
  Word16 t0_min,    /* (i)     : minimum value in the searched range.       */
  Word16 t0_max,    /* (i)     : maximum value in the searched range.       */
  Word16 i_subfr,   /* (i)     : indicator for first subframe.              */
  Word16 *pit_frac  /* (o)     : chosen fraction.                           */
);

Word16 Enc_lag3D(    /* (o)     : Return index of encoding                   */
  Word16 T0,        /* (i)     : Pitch delay                                */
  Word16 T0_frac,   /* (i)     : Fractional pitch delay                     */
  Word16 *T0_min,   /* (i/o)   : Minimum search delay                       */
  Word16 *T0_max,   /* (i/o)   : Maximum search delay                       */
  Word16 pit_min,   /* (i)     : Minimum pitch delay                        */
  Word16 pit_max,   /* (i)     : Maximum pitch delay                        */
  Word16 pit_flag   /* (i)     : Flag for 1st subframe                      */
);

void Dec_lag3D(      /* (o)     : return integer pitch lag                   */
  Word16 index,     /* (i)     : received pitch index                       */
  Word16 pit_min,   /* (i)     : minimum pitch lag                          */
  Word16 pit_max,   /* (i)     : maximum pitch lag                          */
  Word16 i_subfr,   /* (i)     : subframe flag                              */
  Word16 *T0,       /* (o)     : integer part of pitch lag                  */
  Word16 *T0_frac   /* (o)     : fractional part of pitch lag               */
);

void  prm2bits_ld8kD(
  Word16 prm[],     /* (i)     : coder parameters                           */
  Word16 bits[]     /* (o)     : bit stream                                 */
);
void  bits2prm_ld8kD(
  Word16 bits[],    /* (i)     : bit stream                                 */
  Word16 prm[]      /* (o)     : coder parameters                           */
);

void Decod_ACELP64(
  Word16 sign,      /* (i)     : signs of 4 pulses.                         */
  Word16 index,     /* (i)     : Positions of the 4 pulses.                 */
  Word16 cod[]      /* (o) Q13 : algebraic (fixed) codebook excitation      */
);
/*--------------------------------------------------------------------------*
 *       6.4kbps                                                            *
 *--------------------------------------------------------------------------*/
#ifndef max
#define max(a, b)     ((a) > (b) ? (a) : (b))
#endif
#define SIZE_WORD_6K (short)64 /* number of speech bits                    */
#define PRM_SIZE_6K   10       /* Size of vector of analysis parameters.    */
#define SERIAL_SIZE_6K (64+2)  /* Bits/frame + bfi+ number of speech bits   */

#define NCODE1_B_6K  3             /* number of Codebook-bit                */
#define NCODE2_B_6K  3             /* number of Codebook-bit                */
#define NCODE1_6K (1<<NCODE1_B_6K) /* Codebook 1 size                       */
#define NCODE2_6K (1<<NCODE2_B_6K) /* Codebook 2 size                       */
#define NCAN1_6K  6                /* Pre-selecting order for #1            */
#define NCAN2_6K  6                /* Pre-selecting order for #2            */
#define INV_COEF_6K  -28940        /* Q20                                   */

Word16 Qua_gain_8k(
  Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  Word16 g_coeff[], /* (i)     : Correlations <xn y1> -2<y1 y1>             */
                   /*            <y2,y2>, -2<xn,y2>, 2<y1,y2>              */
  Word16 exp_coeff[],/* (i)    : Q-Format g_coeff[]                         */
  Word16 L_subfr,   /* (i)     : Subframe length.                           */
  Word16 *gain_pit, /* (o) Q14 : Pitch gain.                                */
  Word16 *gain_cod, /* (o) Q1  : Code gain.                                 */
  Word16 tameflag,   /* (i)     : flag set to 1 if taming is needed          */
  Word16 past_qua_en[] /* Gain predictorPast quantized energies in Q10 */
);

Word16 Qua_gain_6k(
  Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  Word16 g_coeff[], /* (i)     : Correlations <xn y1> -2<y1 y1>             */
                   /*            <y2,y2>, -2<xn,y2>, 2<y1,y2>              */
  Word16 exp_coeff[],/* (i)    : Q-Format g_coeff[]                         */
  Word16 L_subfr,   /* (i)     : Subframe length.                           */
  Word16 *gain_pit, /* (o) Q14 : Pitch gain.                                */
  Word16 *gain_cod, /* (o) Q1  : Code gain.                                 */
  Word16 tameflag,   /* (i)     : flag set to 1 if taming is needed          */
  Word16 past_qua_en[] /* Gain predictorPast quantized energies in Q10 */
);

void Dec_gain_6k(
  Word16 index,     /* (i)     : Index of quantization.                     */
  Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  Word16 L_subfr,   /* (i)     : Subframe length.                           */
  Word16 bfi,       /* (i)     : Bad frame indicator                        */
  Word16 *gain_pit, /* (o) Q14 : Pitch gain.                                */
  Word16 *gain_cod,  /* (o) Q1  : Code gain.                                 */
  Word16 past_qua_en[] /* Gain predictorPast quantized energies in Q10 */
);

void Dec_gain_8k(
  Word16 index,     /* (i)     : Index of quantization.                     */
  Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  Word16 L_subfr,   /* (i)     : Subframe length.                           */
  Word16 bfi,       /* (i)     : Bad frame indicator                        */
  Word16 *gain_pit, /* (o) Q14 : Pitch gain.                                */
  Word16 *gain_cod,  /* (o) Q1  : Code gain.                                 */
  Word16 past_qua_en[] /* Gain predictorPast quantized energies in Q10 */
);

