/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex F  - Reference C code for fixed point
                         implementation of G.729 Annex F
                         (integration of Annexes B and D)
                         Version 1.1 of October 1999
*/
/*
 File : ld8f.h
 */

/*---------------------------------------------------------------------------*
 * constants for bitstream packing                                           *
 *---------------------------------------------------------------------------*/
#define RATE_6400       64      /* Low  rate  (6400 bit/s)       */
#define RATE_8000       80      /* Full rate  (8000 bit/s)       */
#define RATE_SID        15      /* SID                           */
#define RATE_0          0       /* 0 bit/s rate                  */

#define PRM_SIZE_SID    4        /* Size of vector of analysis parameters.    */
#define G729D           0      /* Low  rate  (6400 bit/s)       */
#define G729            1      /* Full rate  (8000 bit/s)       */

/*--------------------------------------------------------------------------*
 * Main coder and decoder functions                                         *
 *--------------------------------------------------------------------------*/
void Init_Coder_ld8f(Word16 dtx_enable);

void Coder_ld8f(
  Word16 ana[],     /* (o)     : analysis parameters                        */
  Word16 frame,                   /* input : frame counter */
  Word16 dtx_enable,               /* input : DTX enable flag */
  Word16 rate           /* input   : rate selector/frame  =0 6.4kbps , =1 8kbps,= 2 11.8 kbps*/
);

void Init_Decod_ld8f(void);

void Decod_ld8f(
  Word16 parm[],   /* (i)     : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)      */
  Word16 voicing,  /* (i)     : voicing decision from previous frame       */
  Word16 synth[],  /* (o)     : synthesized speech                         */
  Word16  A_t[],    /* (o)     : decoded LP filter for 2 subframes          */
  Word16 *T0_first, /* (o)     : decoded pitch lag in first subframe        */
  Word16 *Vad
);
/*--------------------------------------------------------------------------*
 * Bitstream functions                                                      *
 *--------------------------------------------------------------------------*/
void bits2prm_ld8f(
    Word16 bits[],  /* I   serial bits (nb_bits)                          */
    Word16 prm[]   /* O   output: decoded parameters (11 parameters)     */
);


void  prm2bits_ld8f(
  Word16 prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
  Word16 bits[]          /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = nbbits */
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
