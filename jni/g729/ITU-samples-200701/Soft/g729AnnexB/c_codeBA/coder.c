/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.5    Last modified: October 2006

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*-------------------------------------------------------------------*
 * Main program of the ITU-T G.729A  8 kbit/s encoder.               *
 *                                                                   *
 *    Usage : coder speech_file  bitstream_file                      *
 *-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8a.h"
#include "dtx.h"
#include "octet.h"

int main(int argc, char *argv[] )
{
  FILE *f_speech;               /* File of speech data                   */
  FILE *f_serial;               /* File of serial bits for transmission  */

  extern Word16 *new_speech;     /* Pointer to new speech data            */

  Word16 prm[PRM_SIZE+1];        /* Analysis parameters + frame type      */
  Word16 serial[SERIAL_SIZE];    /* Output bitstream buffer               */

  Word16 frame;                  /* frame counter */
  Word32 count_frame;

  /* For G.729B */
  Word16 nb_words;
  Word16 vad_enable;

  printf("\n");
  printf("***********    ITU G.729A 8 KBIT/S SPEECH CODER    ***********\n");
  printf("                        (WITH ANNEX B)                        \n");
  printf("\n");
  printf("------------------- Fixed point C simulation -----------------\n");
  printf("\n");
  printf("------------ Version 1.5 (Release 2, November 2006) ----------\n");
  printf("\n");


/*--------------------------------------------------------------------------*
 * Open speech file and result file (output serial bit stream)              *
 *--------------------------------------------------------------------------*/

  if ( argc != 4 ){
    printf("Usage :%s speech_file  bitstream_file  VAD_flag\n", argv[0]);
    printf("\n");
    printf("Format for speech_file:\n");
    printf("  Speech is read from a binary file of 16 bits PCM data.\n");
    printf("\n");
    printf("Format for bitstream_file:\n");
    printf("  One (2-byte) synchronization word \n");
    printf("  One (2-byte) size word,\n");
    printf("  80 words (2-byte) containing 80 bits.\n");
    printf("\n");
    printf("VAD flag:\n");
    printf("  0 to disable the VAD\n");
    printf("  1 to enable the VAD\n");
    exit(1);
  }

  if ( (f_speech = fopen(argv[1], "rb")) == NULL) {
     printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
     exit(0);
   }
  printf(" Input speech file:  %s\n", argv[1]);

  if ( (f_serial = fopen(argv[2], "wb")) == NULL) {
     printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
     exit(0);
  }
  printf(" Output bitstream file:  %s\n", argv[2]);

  vad_enable = (Word16)atoi(argv[3]);
  if (vad_enable == 1)
    printf(" VAD enabled\n");
  else
    printf(" VAD disabled\n");

#ifndef OCTET_TX_MODE
  printf(" OCTET TRANSMISSION MODE is disabled\n");
#endif

/*--------------------------------------------------------------------------*
 * Initialization of the coder.                                             *
 *--------------------------------------------------------------------------*/

  Init_Pre_Process();
  Init_Coder_ld8a();
  Set_zero(prm, PRM_SIZE+1);

  /* for G.729B */
  Init_Cod_cng();


  /* Loop for each "L_FRAME" speech data. */

  frame = 0;
  count_frame = 0L;
  while( fread(new_speech, sizeof(Word16), L_FRAME, f_speech) == L_FRAME)
  {
    printf("Frame = %ld\r", count_frame++);

    if (frame == 32767) frame = 256;
    else frame++;

    Pre_Process(new_speech, L_FRAME);
    Coder_ld8a(prm, frame, vad_enable);
    prm2bits_ld8k( prm, serial);
    nb_words = serial[1] +  (Word16)2;
    fwrite(serial, sizeof(Word16), nb_words, f_serial);
  }

  printf("%ld frames processed\n", count_frame);

  return (0);
}


