/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* ITU-T G.729 - Reference C code for fixed point implementation */
/* Version 3.3    Last modified: December 26, 1995 */

/*
   ITU-T G.729 Speech Coder     ANSI-C Source Code
   Copyright (c) 1995, AT&T, France Telecom, NTT, Universite de Sherbrooke.
   All rights reserved.
*/

/*-----------------------------------------------------------------*
 * Main program of the ITU-T G.729  8 kbit/s decoder.              *
 *                                                                 *
 *    Usage : decoder  bitstream_file  synth_file                  *
 *                                                                 *
 *-----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main(int argc, char *argv[] )
{
  Word16  synth_buf[L_FRAME+M], *synth; /* Synthesis                   */
  Word16  parm[PRM_SIZE+1];             /* Synthesis parameters        */
  Word16  serial[SERIAL_SIZE];          /* Serial stream               */
  Word16  Az_dec[MP1*2], *ptr_Az;       /* Decoded Az for post-filter  */
  Word16  T0_first;                     /* Pitch lag in 1st subframe   */
  Word16  pst_out[L_FRAME];             /* Postfilter output           */

  Word16  voicing;                      /* voicing from previous frame */
  Word16  sf_voic;                      /* voicing for subframe        */

  Word16  i, frame;
  FILE   *f_syn, *f_serial;

  printf("\n");
  printf("***********     ITU G.729 8 KBIT/S SPEECH CODER    ***********\n");
  printf("\n");
  printf("------------------- Fixed point C simulation -----------------\n");
  printf("\n");
  printf("------------ Version 3.3 (Release 2, November 2006) --------\n");
  printf("\n");

   /* Passed arguments */

   if ( argc != 3 )
     {
        printf("Usage :%s bitstream_file  outputspeech_file\n",argv[0]);
        printf("\n");
        printf("Format for bitstream_file:\n");
        printf("  One (2-byte) synchronization word,\n");
        printf("  One (2-byte) size word,\n");
        printf("  80 words (2-byte) containing 80 bits.\n");
        printf("\n");
        printf("Format for outputspeech_file:\n");
        printf("  Output is written to a binary file of 16 bits data.\n");
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

  for (i=0; i<M; i++) synth_buf[i] = 0;
  synth = synth_buf + M;

  Init_Decod_ld8k();
  Init_Post_Filter();
  Init_Post_Process();
  voicing = 60;

/*-----------------------------------------------------------------*
 *            Loop for each "L_FRAME" speech data                  *
 *-----------------------------------------------------------------*/

  frame = 0;
  while( fread(serial, sizeof(Word16), SERIAL_SIZE, f_serial) == SERIAL_SIZE)
  {

    bits2prm_ld8k( &serial[2], &parm[1]);

    /* the hardware detects frame erasures by checking if all bits
       are set to zero
     */
    parm[0] = 0;           /* No frame erasure */
    for (i=2; i < SERIAL_SIZE; i++)
      if (serial[i] == 0 ) parm[0] = 1; /* frame erased     */

    /* check parity and put 1 in parm[4] if parity error */

    parm[4] = Check_Parity_Pitch(parm[3], parm[4]);

    Decod_ld8k(parm, voicing, synth, Az_dec, &T0_first);

    /* Postfilter */

    voicing = 0;
    ptr_Az = Az_dec;
    for(i=0; i<L_FRAME; i+=L_SUBFR) {
       Post(T0_first, &synth[i], ptr_Az, &pst_out[i], &sf_voic);
       if (sf_voic != 0) { voicing = sf_voic;}
       ptr_Az += MP1;
    }
    Copy(&synth_buf[L_FRAME], &synth_buf[0], M);

    Post_Process(pst_out, L_FRAME);

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
    frame++;
    printf("Frame =%d\r", frame);
  }
  return(0);
}


