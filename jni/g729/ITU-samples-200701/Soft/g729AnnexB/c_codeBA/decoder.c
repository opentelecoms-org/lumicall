/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.5    Last modified: October 2006

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*-----------------------------------------------------------------*
 * Main program of the G.729A 8.0 kbit/s decoder.                  *
 *                                                                 *
 *    Usage : decoder  bitstream_file  synth_file                  *
 *                                                                 *
 *-----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8a.h"
#include "dtx.h"
#include "octet.h"

Word16 bad_lsf;        /* bad LSF indicator   */


/*
   This variable should be always set to zero unless transmission errors
   in LSP indices are detected.
   This variable is useful if the channel coding designer decides to
   perform error checking on these important parameters. If an error is
   detected on the  LSP indices, the corresponding flag is
   set to 1 signalling to the decoder to perform parameter substitution.
   (The flags should be set back to 0 for correct transmission).
*/

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main(int argc, char *argv[] )
{
  Word16  synth_buf[L_FRAME+M], *synth; /* Synthesis                   */
  Word16  parm[PRM_SIZE+2];             /* Synthesis parameters        */
  Word16  Az_dec[MP1*2];                /* Decoded Az for post-filter  */
  Word16  T2[2];                        /* Pitch lag for 2 subframes   */


  Word16  i, Vad;
  Word32  count_frame;
  FILE   *f_syn, *f_serial;

  printf("\n");
  printf("************  ITU G.729A 8.0 KBIT/S SPEECH DECODER  ************\n");
  printf("                       (WITH ANNEX B)                           \n");
  printf("\n");
  printf("------------------ Fixed point C simulation --------------------\n");
  printf("\n");
  printf("------------- Version 1.5 (Release 2, November 2006) ------------\n");
  printf("\n");

   /* Passed arguments */

  if ( argc != 3){
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

#ifndef OCTET_TX_MODE
  printf("OCTET TRANSMISSION MODE is disabled\n");
#endif

/*-----------------------------------------------------------------*
 *           Initialization of decoder                             *
 *-----------------------------------------------------------------*/

  for (i=0; i<M; i++) synth_buf[i] = 0;
  synth = synth_buf + M;

  bad_lsf = 0;          /* Initialize bad LSF indicator */
  Init_Decod_ld8a();
  Init_Post_Filter();
  Init_Post_Process();

  /* for G.729b */
  Init_Dec_cng();

/*-----------------------------------------------------------------*
 *            Loop for each "L_FRAME" speech data                  *
 *-----------------------------------------------------------------*/

  count_frame = 0L;
  while(read_frame(f_serial, parm) != 0)
  {
    printf("Frame = %ld\r", count_frame++);

    Decod_ld8a(parm, synth, Az_dec, T2, &Vad);
    Post_Filter(synth, Az_dec, T2, Vad);        /* Post-filter */
    Post_Process(synth, L_FRAME);

    fwrite(synth, sizeof(short), L_FRAME, f_syn);

  }

  printf("%ld frames decoded\n", count_frame);
  return(0);
}




