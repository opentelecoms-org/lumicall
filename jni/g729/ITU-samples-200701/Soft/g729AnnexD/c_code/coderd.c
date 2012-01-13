/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX D   Version 1.3    Last modified: May 1998 */

/* 
   ITU-T G.729 Annex D 6.4 kbps Speech Coder Extension written
   by Ericsson and NTT in ANSI-C Source Code.
 
   Copyright (c) 1998, All rights reserved.
 
   Based on ITU-T G.729 Speech Coder Fixed point Version 3.3 by
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies.
*/
 
/*-------------------------------------------------------------------*
 * Main program of the ITU-T G.729 8 kbit/s with annex D 6.4 kbit/s  *
 *                                                                   *
 *    Usage : coderd CODEC_MODE speech_file bitstream_file           *
 *-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "tabld8kd.h"

int main(int argc, char *argv[] )
{
  FILE *f_speech;                /* File of speech data                   */
  FILE *f_serial;                /* File of serial bits for transmission  */

  extern Word16 *new_speech;     /* Pointer to new speech data            */

  Word16 prm[max(PRM_SIZE, PRM_SIZE_6K)];   /* Analysis parameters.       */
  Word16 serial[max(SERIAL_SIZE, SERIAL_SIZE_6K)]; /* Output bitstream buffer */
  Word16 syn[L_FRAME];           /* Buffer for synthesis speech           */

  Word16 i, frame;               /* frame counter */

  Word16 serial_size;

  printf("\n");
  printf("**********     ITU G.729 8 KBIT/S SPEECH CODER      *********\n");
  printf("**********   WITH ANNEX D 6.4 KBIT/S SPEECH CODER   *********\n");
  printf("\n");
  printf("------------------ Fixed point C simulation -----------------\n");
  printf("\n");
  printf("---------   Version 1.3 (Release 2, November 2006)   --------\n");
  printf("\n");


/*--------------------------------------------------------------------------*
 * Open speech file and result file (output serial bit stream)              *
 *--------------------------------------------------------------------------*/

  if ( argc != 4 )
    {
       printf("Usage : %s CODEC_MODE speech_file bitstream_file\n",argv[0]);
       printf("\n");
       printf("where CODEC_MODE is 1 for G729 annex D \n");
       printf("                    2 for G729\n");
       printf("                    3 for Toggle Mode\n");
       printf("\n");
       printf("Format for speech_file:\n");
       printf("  Speech is read from a binary file of 16 bits PCM data.\n");
       printf("\n");
       printf("Format for bitstream_file:\n");
       printf("  One (2-byte) synchronization word \n");
       printf("  One (2-byte) size word,\n");
       printf("  80 (or 64) words (2-byte) containing 80 (or 64) bits.\n");
       printf("\n");
       exit(1);
    }

  CODEC_MODE = atoi(argv[1]);
  if ( CODEC_MODE < 1 || CODEC_MODE > 3 ) {
     printf("%s - Requested Mode %s not supported!!\n", argv[0], argv[1]);
     exit(0);
  }

  if ( (f_speech = fopen(argv[2], "rb")) == NULL) {
     printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
     exit(0);
  }
  printf(" Input speech file     :  %s\n", argv[2]);

  if ( (f_serial = fopen(argv[3], "wb")) == NULL) {
     printf("%s - Error opening file  %s !!\n", argv[0], argv[3]);
     exit(0);
  }
  printf(" Output bitstream file :  %s\n", argv[3]);

  toggleFlag=0;
  if (CODEC_MODE==1)
     printf(" Codec mode            :  G729@6.4 (6.4 kbps)\n");
  if (CODEC_MODE==2)
     printf(" Codec mode            :  G729 (8.0 kbps)\n");
  if (CODEC_MODE==3) {
     printf(" Codec mode            :  Toggle (6.4/8.0 kbps)\n");
     toggleFlag=1;
     CODEC_MODE=1;  /* starting mode */
  }
  
/*--------------------------------------------------------------------------*
 * Initialization of the coder.                                             *
 *--------------------------------------------------------------------------*/

  Init_Pre_Process();
  Init_Coder_ld8k();
  for(i=0; i<max(PRM_SIZE, PRM_SIZE_6K); i++) prm[i] = (Word16)0;

 /* To force the input and output to be time-aligned the variable SYNC
    has to be defined. Note: the test vectors were generated with this option
    disabled
  */

#ifdef SYNC
  /* Read L_NEXT first speech data */

  fread(&new_speech[-L_NEXT], sizeof(Word16), L_NEXT, f_speech);
#ifdef HARDW
    /* set 3 LSB's to zero */
    for(i=0; i < L_NEXT; i++)
      new_speech[-L_NEXT+i] = new_speech[-L_NEXT+i] & 0xFFF8;
#endif
  Pre_Process(&new_speech[-L_NEXT], L_NEXT);
#endif

  /* Loop for each "L_FRAME" speech data. */

  frame =0;
  while( fread(new_speech, sizeof(Word16), L_FRAME, f_speech) == L_FRAME)
  {
#ifdef HARDW
    /* set 3 LSB's to zero */
    for(i=0; i < L_FRAME; i++) new_speech[i] = new_speech[i] & 0xFFF8;
#endif

    if (toggleFlag==1) {
        if (CODEC_MODE==1) {
            CODEC_MODE=2;
        }
        else {
            CODEC_MODE=1;
        }
    }
    Pre_Process(new_speech, L_FRAME);

    Coder_ld8kD(prm, syn);

    prm2bits_ld8kD( prm, serial);

    if (sub(CODEC_MODE, 1) == 0) {
       serial_size = SERIAL_SIZE_6K;
    }
    else if (sub(CODEC_MODE, 2) == 0) {
       serial_size = SERIAL_SIZE;
    }
    else {
       fprintf(stderr, "CODEC mode invalid\n");
       exit(-1);
    }

    if (fwrite(serial, sizeof(Word16), serial_size, f_serial) != (unsigned)serial_size)
      printf("Write Error for frame %d\n", frame);
    frame++;
    fprintf(stderr, "Frame =%d\r", frame);

  }
  return (0);
}
