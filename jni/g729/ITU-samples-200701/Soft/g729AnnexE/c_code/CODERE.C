/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.3    Last modified: September  1999 */
/* modification of version number */


/* ------------------------------------------------------------------------ */
/*                            MAIN PROGRAM                                  */
/* ------------------------------------------------------------------------

   ITU-T G.729 Speech Coder     ANSI-C Source Code
   Copyright (c) 1995, AT&T, France Telecom, NTT, Universite de Sherbrooke.
   All rights reserved.

   +

  HIGHER BIT RATE EXTENSION AT 11.8 KB/S Version v1.2
   Copyright (c) 1997, France Telecom, Universite de Sherbrooke.
   All rights reserved.

   using a backward / forward lpc structure
   Copyright (c) 1997, France Telecom.
   All rights reserved.

   ----------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8e.h"


/*#define SYNC*/
#define PREPROC

int main(int argc, char *argv[] )
{
  FILE *f_speech;               /* File of speech data                   */
  FILE *f_serial;               /* File of serial bits for transmission  */
  FILE  *f_rate;
  Word16 rate, flag_rate;

  extern Word16 *new_speech;     /* Pointer to new speech data            */

  Word16 prm[PRM_SIZE_E];          /* Analysis parameters.                  */
  Word16 serial[SERIAL_SIZE_E];    /* Output bitstream buffer               */

  Word16 i, frame;               /* frame counter */


  Word16 serial_size[2] = { 82, 120};

  printf("\n");
  printf("*************************************************************************\n");
  printf("****   HIGHER BIT RATE EXTENSION OF ITU G.729 8 KBIT/S SPEECH CODER  ****\n");
  printf("****               MIXED BACKWARD / FORWARD LPC STRUCTURE            ****\n");
  printf("*************************************************************************\n");
  printf("\n");
  printf("------------------- Fixed point C simulation -----------------\n");
  printf("\n");
  printf("---------   Version 1.3 (Release 2, November 2006)   ---------\n");
  printf("\n");
  printf("                 Bit rates : 8.0 kb/s (G729) or 11.8 kb/s\n");
  printf("\n");

/*--------------------------------------------------------------------------*
 * Open speech file and result file (output serial bit stream)              *
 *--------------------------------------------------------------------------*/

  if (( argc != 3 ) && (argc != 4) ){
       printf("Usage : coder speech_file bitstream_file [bitrate or file_bit_rate]\n");
       printf("Format for speech_file:\n");
       printf("  Speech is read from a binary file of 16 bits PCM data.\n");
       printf("\n");
       printf("Format for bitstream_file:\n");
       printf("  One (2-byte) synchronization word \n");
       printf("  One (2-byte) bit-rate word \n");
       printf("\n");
       printf("bitrate = 0 (8 kb/s) or 1 (11.8 kb/s)  (default : 8 kb/s)\n");
       printf("Format for bitrate_file:\n");
       printf("  1 16bit-Word per frame , =0 bit rate 8 kb/s, =1 bit rate 11.8 kb/s \n");
       printf("Forward / Backward structure at 11.8 kb/s \n");
       printf("\n");
       exit(1);
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

    if(argc != 3) {
        if ( (f_rate = fopen(argv[3], "rb")) == NULL) {
            rate  = (Word16)atoi(argv[3]);
            if( rate == 1) printf(" Selected Bitrate      :  11.8 kb/s\n");
            else {
                if( rate == 0) printf(" Selected Bitrate      :  8.0 kb/s\n");
                else {
                    printf(" error bit rate indication\n");
                    printf(" argv[3] = 0 for bit rate 8 kb/s \n");
                    printf(" argv[3] = 1 for bit rate 11.8 kb/s\n");
                    exit(-1);
                }
            }
            flag_rate = 0;
        }
        else {
            printf(" Selected Bitrate  read in file :  %s kb/s\n", argv[3]);
            flag_rate = 1;
        }
    }
    else {
        flag_rate = 0;
        rate = 0;
        printf(" Selected Bitrate      :  8 kb/s\n");
    }


/*--------------------------------------------------------------------------*
 * Initialization of the coder.                                             *
 *--------------------------------------------------------------------------*/

  Init_Pre_Process();
  Init_Coder_ld8e();
  for(i=0; i<PRM_SIZE_E; i++) prm[i] = (Word16)0;

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
#ifdef PREPROC
  Pre_Process(&new_speech[-L_NEXT], L_NEXT);
#endif
#endif

  /* Loop for each "L_FRAME" speech data. */

  frame = 1;

  while( fread(new_speech, sizeof(Word16), L_FRAME, f_speech) == L_FRAME) {
    if( flag_rate == 1) {
        if( fread(&rate, sizeof(Word16), 1, f_rate) != 1) {
            printf("error reading bit_rate in file %s for frame %hd\n",
                    argv[3], frame);
            exit(-1);
        }
        if( (rate != 0) && (rate !=1)) {
            printf("error bit_rate in file %s for frame %hd bit rate non avalaible\n",
                    argv[3], frame);
            exit(-1);
        }
    }
#if defined(__BORLANDC__)
    printf("Frame : %hd\n", frame);
#else
    printf("Frame : %hd\r", frame);
#endif
#ifdef HARDW
    /* set 3 LSB's to zero */
    for(i=0; i < L_FRAME; i++) new_speech[i] = new_speech[i] & 0xFFF8;
#endif

#ifdef PREPROC
    Pre_Process(new_speech, L_FRAME);
#else
    /* Division par 2 si pas de preprocessing */
    for(i=0; i < L_FRAME; i++) new_speech[i] = shr(new_speech[i], 1);
#endif


    Coder_ld8e(prm, rate);

    prm2bits_ld8e( prm, serial, rate);

    if (fwrite(serial, sizeof(Word16), serial_size[rate], f_serial) != (size_t)(serial_size[rate]))
      printf("Write Error for frame %d\n", frame);

    frame++;

  }
  printf("\n");

  return(0);
}
