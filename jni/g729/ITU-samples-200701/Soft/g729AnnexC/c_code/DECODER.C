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
 File : DECODER.C
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/

/*-----------------------------------------------------------------*
 * Main program of the G.729  8.0 kbit/s decoder.                  *
 *                                                                 *
 *    Usage : decoder  bitstream_file  synth_file                  *
 *                                                                 *
 *-----------------------------------------------------------------*/

#if defined(__BORLANDC__)
extern unsigned _stklen = 48000U;
#endif

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "ld8k.h"


/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main( int argc, char *argv[])
{
   FLOAT  synth_buf[L_FRAME+M];            /* Synthesis                  */
   FLOAT  *synth;
   int    parm[PRM_SIZE+1];                /* Synthesis parameters + BFI */
   INT16  serial[SERIAL_SIZE];             /* Serial stream              */
   FLOAT  Az_dec[2*MP1], *ptr_Az;          /* Decoded Az for post-filter */
   int    t0_first;
   FLOAT  pst_out[L_FRAME];                /* postfilter output          */

   int voicing;                    /* voicing for previous subframe */
   int sf_voic;                    /* voicing for subframe */

   INT32   frame;
   FILE   *f_syn, *f_serial;

   int   i;


   printf("\n");
   printf("**************    G.729  8 KBIT/S SPEECH DECODER    ************\n");
   printf("\n");
   printf("----------------- Floating point C simulation ----------------\n");
   printf("\n");
   printf("------------ Version 1.01 (Release 2, November 2006) --------\n");
   printf("\n");

   /* Passed arguments */

   if ( argc != 3)
     {
        printf("Usage :%s bitstream_file  outputspeech_file\n",argv[0]);
        printf("\n");
        printf("Format for bitstream_file:\n");
        printf("  One (2-byte) synchronization word \n");
        printf("  One (2-byte) size word,\n");
        printf("  80 words (2-byte) containing 80 bits.\n");
        printf("\n");
        printf("Format for outputspeech_file:\n");
        printf("  Synthesis is written to a binary file of 16 bits data.\n");
        exit( 1 );
     }

   /* Open file for synthesis and packed serial stream */

   if( (f_serial = fopen(argv[1],"rb") ) == NULL )
     {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
        exit(0);
     }

   if( (f_syn = fopen(argv[2], "wb") ) == NULL )
     {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
        exit(0);
     }

   printf("Input bitstream file  :   %s\n",argv[1]);
   printf("Synthesis speech file :   %s\n",argv[2]);

/*-----------------------------------------------------------------*
 *           Initialization of decoder                             *
 *-----------------------------------------------------------------*/

  for (i=0; i<M; i++) synth_buf[i] = (F)0.0;
  synth = synth_buf + M;

  init_decod_ld8k();
  init_post_filter();
  init_post_process();
  voicing = 60;

/*-----------------------------------------------------------------*
 *            Loop for each "L_FRAME" speech data                  *
 *-----------------------------------------------------------------*/

   frame =0;
   while( fread(serial, sizeof(INT16), SERIAL_SIZE, f_serial) == SERIAL_SIZE)
   {
      frame++;
      printf(" Frame: %ld\r", frame);

      bits2prm_ld8k( &serial[2], &parm[1]);

      /* the hardware detects frame erasures by checking if all bits
         are set to zero
      */
      parm[0] = 0;           /* No frame erasure */
      for (i=2; i < SERIAL_SIZE; i++)
        if (serial[i] == 0 ) parm[0] = 1; /* frame erased     */

      /* check parity and put 1 in parm[4] if parity error */

      parm[4] = check_parity_pitch(parm[3], parm[4] );

      decod_ld8k(parm, voicing, synth, Az_dec, &t0_first);  /* Decoder */

      /* Post-filter and decision on voicing parameter */
      voicing = 0;
      ptr_Az = Az_dec;
      for(i=0; i<L_FRAME; i+=L_SUBFR) {
        post(t0_first, &synth[i], ptr_Az, &pst_out[i], &sf_voic);
        if (sf_voic != 0) { voicing = sf_voic;}
        ptr_Az += MP1;
      }
      copy(&synth_buf[L_FRAME], &synth_buf[0], M);

      post_process(pst_out, L_FRAME);

      fwrite16(pst_out, L_FRAME, f_syn);
   }

   return(0);
}
