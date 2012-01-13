/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.3    Last modified: September  1999 */

/* ------------------------------------------------------------------------ */
/*                             MAIN PROGRAM                                 */
/* ------------------------------------------------------------------------

   ITU-T G.729 Speech Decoder     ANSI-C Source Code
   Copyright (c) 1995, AT&T, France Telecom, NTT, Universite de Sherbrooke.
   All rights reserved.

   +

   HIGHER BIT RATE EXTENSION AT 11.8 KB/S Version v1.2
   Copyright (c) 1997, France Telecom, Universite de Sherbrooke.
   All rights reserved.

   using a backward / forward lpc structure
   Copyright (c) 1997, France Telecom.
   All rights reserved.

   -----------------------------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8e.h"

#define POSTPROC

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main(int argc, char *argv[] )
{
    Word16 rate;
  Word16  synth_buf[L_ANA_BWD], *synth; /* Synthesis                   */
  Word16  parm[PRM_SIZE_E+2];             /* Synthesis parameters        */
  Word16  serial[SERIAL_SIZE_E];            /* Serial stream               */
  Word16  Az_dec[M_BWDP1*2], *ptr_Az;       /* Decoded Az for post-filter  */
  Word16  T0_first;                         /* Pitch lag in 1st subframe   */
  Word16  pst_out[L_FRAME];                 /* Postfilter output           */

  Word16  voicing;                          /* voicing from previous frame */
  Word16  sf_voic;                          /* voicing for subframe        */

  Word16  i, frame = 0;
  Word16  ga1_post, ga2_post, ga_harm;
  Word16  long_h_st, m_pst;
  Word16  serial_size;
  Word16  bwd_dominant;
  FILE    *f_syn, *f_serial;

  printf("\n");
  printf("*************************************************************************\n");
  printf("****  HIGHER BIT RATE EXTENSION OF ITU G.729 8 KBIT/S SPEECH DECODER ****\n");
  printf("****               MIXED BACKWARD / FORWARD LPC STRUCTURE            ****\n");
  printf("*************************************************************************\n");
  printf("\n");
  printf("------------------- Fixed point C simulation -----------------\n");
  printf("\n");
  printf("---------   Version 1.3 (Release 2, November 2006)   ---------\n");
  printf("\n");
  printf("                 Bit rates : 8.0 kb/s (G729) or 11.8 kb/s\n");
  printf("\n");

  /* Passed arguments */
  if ( argc != 3 ) {
    printf("Usage : decoder bitstream_file  output_file  \n");
    printf("\n");
    printf("Format for bitstream_file:\n");
    printf("  One (2-byte) synchronization word,\n");
    printf("  One (2-byte) bit-rate word,\n");
    printf("\n");
    printf("Format for outputspeech_file:\n");
    printf("  Output is written to a binary file of 16 bits data.\n");
    exit( 1 );
  }


  /* Open file for synthesis and packed serial stream */
  if( (f_serial = fopen(argv[1],"rb") ) == NULL ) {
    printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
    exit(0);
  }
  if( (f_syn = fopen(argv[2], "wb") ) == NULL ) {
    printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
    exit(0);
  }

  printf("Input bitstream file    :   %s\n\n",argv[1]);
  printf("Synthesis speech file   :   %s\n\n",argv[2]);

/*-----------------------------------------------------------------*
 *           Initialization of decoder                             *
 *-----------------------------------------------------------------*/

  for (i=0; i<L_ANA_BWD; i++) synth_buf[i] = 0;
  synth = synth_buf + MEM_SYN_BWD;

  Init_Decod_ld8e();
  Init_Post_Filter();
  Init_Post_Process();

  voicing = 60;

  ga1_post = GAMMA1_PST_E;
  ga2_post = GAMMA2_PST_E;
  ga_harm = GAMMA_HARM_E;

/*-----------------------------------------------------------------*
 *            Loop for each "L_FRAME" speech data                  *
 *-----------------------------------------------------------------*/

  while( fread(serial, sizeof(Word16), 2, f_serial) == 2) {

    if(serial[0] != SYNC_WORD ) {
        printf("error reading synchro word file %s frame %hd\n",
            argv[1], frame);
        exit(-1);
    }
    serial_size = serial[1];

    if( (serial_size != 118) && (serial_size != 80) ){
        printf("error uncorrect value of serial_size (%hd) in %s frame %hd\n",
            serial_size, argv[1], frame);
        exit(-1);
    }


    if (fread(&serial[2], sizeof(Word16), serial_size, f_serial) != (size_t)serial_size) {
        printf("error reading file %s frame %hd\n",
            argv[1], frame);
        exit(-1);
    }


    frame++;
#if defined(__BORLANDC__)
    printf("Frame : %4d\r", frame);
#endif
    if(serial_size == 118) rate = 1;
    else rate = 0;
    bits2prm_ld8e(&serial[2], &parm[1], rate);

    /* the hardware detects frame erasures by checking if all bits
       are set to zero
     */
    parm[0] = 0;           /* No frame erasure */
    for (i=2; i < serial_size; i++)
      if (serial[i] == 0 ) parm[0] = 1; /* frame erased     */

    if (parm[0] == 1) printf("Frame Erased : %d\n", frame);

    if(rate == 0) {
        parm[4] = Check_Parity_Pitch(parm[3], parm[4]);
    }
    else {
        /* ------------------------------------------------------------------ */
        /* check parity and put 1 in parm[5] if parity error in Forward mode  */
        /*                  put 1 in parm[3] if parity error in Backward mode */
        /* ------------------------------------------------------------------ */
        if (parm[1] == 0) {
            i = shr(parm[4], 1);
            i &= (Word16)1;
            parm[5] = add(parm[5], i);
            parm[5] = Check_Parity_Pitch(parm[4], parm[5]);
        }
        else {
            i = shr(parm[2], 1);
            i &= (Word16)1;
            parm[3] = add(parm[3], i);
            parm[3] = Check_Parity_Pitch(parm[2], parm[3]);
        }
    }

    /* ---------- */
    /*  Decoding  */
    /* ---------- */
    Decod_ld8e(parm, rate, voicing, synth_buf, Az_dec, &T0_first, &bwd_dominant,
        &m_pst);

    /* ---------- */
    /* Postfilter */
    /* ---------- */
    voicing = 0; 
    ptr_Az = Az_dec;

    /* Adaptive parameters for postfiltering */
    /* ------------------------------------- */
    if (serial_size == 80) {
        long_h_st = LONG_H_ST;
        ga1_post = GAMMA1_PST;
        ga2_post = GAMMA2_PST;
        ga_harm = GAMMA_HARM;
    }
    else {
        long_h_st = LONG_H_ST_E;
        /* If backward mode is dominant => progressively reduce postfiltering */
        if ((parm[1] == 1) && (bwd_dominant == 1)) {
            ga_harm = sub(ga_harm, 410);
            if (ga_harm < 0) ga_harm = 0;
            ga1_post = sub(ga1_post, 1147);
            if (ga1_post < 0) ga1_post = 0;
            ga2_post = sub(ga2_post, 1065);
            if (ga2_post < 0) ga2_post = 0;
        }
        else {
            ga_harm = add( ga_harm, 410);
            if (ga_harm > GAMMA_HARM_E) ga_harm = GAMMA_HARM_E;
            ga1_post = add(ga1_post, 1147);
            if (ga1_post > GAMMA1_PST_E) ga1_post = GAMMA1_PST_E;
            ga2_post = add(ga2_post, 1065);
            if (ga2_post > GAMMA2_PST_E) ga2_post = GAMMA2_PST_E;
        }
    }

    for(i=0; i<L_FRAME; i++) pst_out[i] = synth[i];

	for(i=0; i<L_FRAME; i+=L_SUBFR) {
	   Poste(T0_first, &synth[i], ptr_Az, &pst_out[i], &sf_voic,
		   ga1_post, ga2_post, ga_harm, long_h_st, m_pst);
	   if (sf_voic != 0) voicing = sf_voic;
	   ptr_Az += m_pst+1;
	}


#ifdef POSTPROC
  Post_Process(pst_out, L_FRAME);
#else
  for (i=0; i<L_FRAME; i++) pst_out[i] = shl(pst_out[i],1);
#endif

#ifdef HARDW
  {
  Word16 *my_pt;
  Word16 my_temp;
  int my_i;
  my_pt = pst_out;
       for(my_i=0; my_i < L_FRAME; my_i++) {
          my_temp = *my_pt;
          my_temp = add( my_temp, (Word16) 4); /* rounding on 13 bit */
          my_temp = my_temp & 0xFFF8; /* mask on 13 bit */
          *my_pt++ = my_temp;
       }
    }
#endif

    fwrite(pst_out, sizeof(Word16), L_FRAME, f_syn);
  }
  printf("\n");
  return(0);
}


