/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX D   Version 1.3    Last modified: May 1998 */

/* 
   ITU-T G.729 Annex D 6.4 kbps Speech Coder Extension written
   by Ericsson and NTT in ANSI-C Source Code.
 
   Copyright (c) 1998, All rights reserved.
 
   Based on ITU-T G.729 Speech Coder Fixed point Version 3.3 by
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies.
*/
 
/*------------------------------------------------------------------*
 * Main program of the ITU-T G.729 8 kbit/s with annex D 6.4 kbit/s *
 * decoder.                                                         *
 *                                                                  *
 *    Usage : decoderd CODEC_MODE bitstream_file synth_file          *
 *                                                                  *
 *------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "tabld8kd.h"

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main(int argc, char *argv[] )
{
  Word16  synth_buf[L_FRAME+M], *synth; /* Synthesis                   */
  Word16  parm[max(PRM_SIZE, PRM_SIZE_6K)+1]; /* Synthesis parameters  */
  Word16  serial[max(SERIAL_SIZE, SERIAL_SIZE_6K)]; /* Serial stream   */
  Word16  Az_dec[MP1*2], *ptr_Az;       /* Decoded Az for post-filter  */
  Word16  T0_first;                     /* Pitch lag in 1st subframe   */
  Word16  pst_out[L_FRAME];             /* Postfilter output           */

  Word16  voicing;                      /* voicing from previous frame */
  Word16  sf_voic;                      /* voicing for subframe        */

  Word16 serial_size;

  Word16  i, frame;
  FILE   *f_syn, *f_serial;

  printf("\n");
  printf("**********     ITU G.729 8 KBIT/S SPEECH CODER      *********\n");
  printf("**********   WITH ANNEX D 6.4 KBIT/S SPEECH CODER   *********\n");
  printf("\n");
  printf("------------------- Fixed point C simulation -----------------\n");
  printf("\n");
  printf("---------   Version 1.3 (Release 2, November 2006)   ---------\n");
  printf("\n");

  /* Passed arguments */

  if ( argc != 4)
  {
      printf("Usage : %s CODEC_MODE bitstream_file outputspeech_file\n",argv[0]);
      printf("\n");
      printf("where CODEC_MODE is 1 for G729 annex D \n");
      printf("                    2 for G729\n");
      printf("                    3 for Toggle Mode\n");
      printf("\n");
      printf("Format for bitstream_file:\n");
      printf("  One (2-byte) synchronization word \n");
      printf("  One (2-byte) size word,\n");
      printf("  80 (or 64) words (2-byte) containing 80 (or 64) bits.\n");
      printf("\n");
      printf("Format for outputspeech_file:\n");
      printf("  Synthesis is written to a binary file of 16 bits data.\n");
      exit( 1 );
  }
  
  CODEC_MODE = atoi(argv[1]);
  if ( CODEC_MODE < 1 || CODEC_MODE > 3 ) {
     printf("%s - Requested Mode %s not supported!!\n", argv[0], argv[1]);
     exit(0);
  }

  /* Open file for synthesis and packed serial stream */

   if( (f_serial = fopen(argv[2],"rb") ) == NULL )
     {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
        exit(0);
     }

   if( (f_syn = fopen(argv[3], "wb") ) == NULL )
     {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[3]);
        exit(0);
     }

   toggleFlag=0;
   printf("Input bitstream file  :   %s\n",argv[2]);
   printf("Synthesis speech file :   %s\n",argv[3]);
   if (CODEC_MODE==1) 
      printf(" Codec mode           :   G729@6.4 (6.4 kbps)\n");
   if (CODEC_MODE==2) 
      printf(" Codec mode           :   G729 (8.0 kbps)\n");
   if (CODEC_MODE==3) {
      printf(" Codec mode           :   Toggle (6.4/8.0 kbps)\n");
      toggleFlag=1;
      CODEC_MODE=1;  /* starting mode */
   }

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
   while( 1 )
   {
      /* readout header only */
      if ( fread(serial, sizeof(Word16), 2, f_serial) != 2 ) 
         break;
        
      /* bit rate decision */
      switch ( CODEC_MODE ) {
      case 1:  /* 6.4kbps */
        serial_size = SERIAL_SIZE_6K;
        break;
      case 2:  /* 8kbps */
        serial_size = SERIAL_SIZE;
        break;
      default:
        exit(1);
      }

      if (toggleFlag==1) {
         switch ( serial[1] ) {
         case SIZE_WORD_6K:
           serial_size = SERIAL_SIZE_6K;
           CODEC_MODE = 1;
           break;
         case SIZE_WORD:
           serial_size = SERIAL_SIZE;
           CODEC_MODE = 2;
           break;
         default:
           printf("!!!!! Illegal bitstream !!!!!\n");
           exit(1);
         }
      }
       
      /* readout stream body */
      if ( fread(&serial[2], sizeof(Word16), serial_size-2, f_serial) != (unsigned)serial_size-2 )
         break;

      bits2prm_ld8kD( &serial[2], &parm[1]);

      /* the hardware detects frame erasures by checking if all bits
         are set to zero
         */
      parm[0] = 0;           /* No frame erasure */
      for (i=2; i < serial_size; i++)
          if (serial[i] == 0 ) parm[0] = 1; /* frame erased     */
       
      /* check parity and put 1 in parm[4] if parity error */

      if (sub(CODEC_MODE, 2) == 0) {
         parm[4] = Check_Parity_Pitch(parm[3], parm[4]);
      }

      Decod_ld8kD(parm, voicing, synth, Az_dec, &T0_first);
       
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
   fprintf(stderr, "Frame =%d\r", frame);
   }
   return(0);
}
