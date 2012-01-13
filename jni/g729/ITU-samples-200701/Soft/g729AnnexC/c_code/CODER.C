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
 File : CODER.C
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/

/*-------------------------------------------------------------------*
 * Main program of the ITU-T G.729   8 kbit/s encoder.               *
 *                                                                   *
 *    Usage : coder speech_file  bitstream_file                      *
 *-------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "ld8k.h"
#if defined(__BORLANDC__)
extern unsigned _stklen = 48000U;
#endif

int main( int argc, char *argv[])
{
   FILE *f_speech;                     /* Speech data        */
   FILE *f_serial;                     /* Serial bit stream  */

   extern FLOAT *new_speech;           /* Pointer to new speech data   */

   int prm[PRM_SIZE];           /* Transmitted parameters        */
   INT16 serial[SERIAL_SIZE];   /* Output bit stream buffer      */
   INT16 sp16[L_FRAME];         /* Buffer to read 16 bits speech */

   int i;
   INT32   frame;

   printf("\n");
   printf("************  ITU G.729  8 Kbit/S SPEECH CODER  **************\n");
   printf("\n");
   printf("----------------- Floating point C simulation ----------------\n");
   printf("\n");
   printf("------------ Version 1.01 (Release 2, November 2006) --------\n");
   printf("\n");


   /*-----------------------------------------------------------------------*
    * Open speech file and result file (output serial bit stream)           *
    *-----------------------------------------------------------------------*/

   if ( argc != 3 )
     {
        printf("Usage : coder  speech_file  bitstream_file \n");
        printf("\n");
        printf("Format for speech_file:\n");
        printf("  Speech is read form a binary file of 16 bits data.\n");
        printf("\n");
        printf("Format for bitstream_file:\n");
        printf("  One word (2-bytes) to indicate erasure.\n");
        printf("  One word (2 bytes) to indicate bit rate\n");
        printf("  80 words (2-bytes) containing 80 bits.\n");
        printf("\n");
        exit( 1 );
     }

   if ( (f_speech = fopen(argv[1], "rb")) == NULL) {
      printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
      exit(0);
   }
   printf(" Input speech file     :  %s\n", argv[1]);

   if ( (f_serial = fopen(argv[2], "wb")) == NULL) {
      printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
      exit(0);
   }
   printf(" Output bitstream file :  %s\n", argv[2]);

  /*-------------------------------------------------*
   * Initialization of the coder.                    *
   *-------------------------------------------------*/

   init_pre_process();
   init_coder_ld8k();           /* Initialize the coder             */

   /*-------------------------------------------------------------------------*
    * Loop for every analysis/transmission frame.                             *
    * -New L_FRAME data are read. (L_FRAME = number of speech data per frame) *
    * -Conversion of the speech data from 16 bit integer to real              *
    * -Call cod_ld8k to encode the speech.                                    *
    * -The compressed serial output stream is written to a file.              *
    * -The synthesis speech is written to a file                              *
    *-------------------------------------------------------------------------*
    */

   frame=0;
   while( fread((void *)sp16, sizeof(INT16), L_FRAME, f_speech) == L_FRAME){
      frame++;
      printf(" Frame: %ld\r", frame);

      for (i = 0; i < L_FRAME; i++)  new_speech[i] = (FLOAT) sp16[i];

      pre_process( new_speech, L_FRAME);

      coder_ld8k(prm);

      prm2bits_ld8k(prm, serial);

      fwrite( (void *)serial, sizeof(INT16), SERIAL_SIZE,  f_serial);
   }

   return(0);

} /* end of main() */
