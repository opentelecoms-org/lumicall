/*
   ITU-T G.729 Annex F Software Package Release 2 (November 2006)

   ITU-T G.729 Annex F  - Reference C code for fixed point
                         implementation of G.729 Annex F
                         (integration of Annexes B and D)
   Version 1.2    Last modified: October 2006 
*/
/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex F fixed point ANSI C source code
   Copyright (C) 1999, AT&T, France Telecom, NTT, University of
   Sherbrooke, Conexant, Ericsson. All rights reserved.
----------------------------------------------------------------------
*/

/*
 File : CODERF.C
 */
/* from coderd.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from coder.c G.729 Annex B Version 1.3  Last modified: August 1997 */
/* from coder.c G.729 Version 3.3  */

/*--------------------------------------------------------------------------------------*
 * Main program of the ITU-T G.729F   8/6.4 kbit/s encoder.
 *
 *    Usage : coderf speech_file  bitstream_file  DTX_flag [bit_rate or file_bit_rate]
 *--------------------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------ */
/*                            MAIN PROGRAM                                  */
/* ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "ld8f.h"
#include "tabld8kd.h"
#include "dtx.h"
#include "octet.h"
#if defined(__BORLANDC__)
extern unsigned _stklen = 48000U;
#endif

/*#define SYNC*/
#define PREPROC

int main(int argc, char *argv[] )
{
    FILE *f_speech;               /* File of speech data                   */
    FILE *f_serial;               /* File of serial bits for transmission  */
    FILE  *f_rate;
    Word16 rate, flag_rate;
    
    extern Word16 *new_speech;     /* Pointer to new speech data            */

    Word16 prm[PRM_SIZE+1];          /* Analysis parameters.                  */
    Word16 serial[SERIAL_SIZE];    /* Output bitstream buffer               */
    
    Word16 i, frame;               /* frame counter */
    Word32 count_frame;
    
    Word16 nb_words, dtx_enable;

    for(i = 0; i < argc; i++){
        printf("argument %d : %s\n", i, argv[i]);
    }
    printf("\n");
    printf("*************************************************************************\n");
    printf("****    ITU G.729 ANNEX F : 6.4, 8.0 KBIT/S SPEECH CODER   ****\n");
    printf("****         INCLUDING OPTIONAL VAD/DTX/CNG (ANNEX B)                ****\n");
    printf("*************************************************************************\n");
    printf("\n");
    printf("------------------ Fixed point C simulation ----------------\n");
    printf("\n");
    printf("------------ Version 1.2 (Release 2, November 2006) --------\n");
    printf("\n");
    printf("                 Bit rates : 6.4 or 8.0kb/s \n");
    printf("\n");

    /*--------------------------------------------------------------------------*
    * Open speech file and result file (output serial bit stream)              *
    *--------------------------------------------------------------------------*/
    if (( argc != 4 ) && (argc != 5) ){
        printf("Usage : coderf speech_file bitstream_file DTX_flag [bitrate or file_bit_rate]\n");
        printf("Format for speech_file:\n");
        printf("  Speech is read from a binary file of 16 bits PCM data.\n");
        printf("\n");
        printf("Format for bitstream_file:\n");
        printf("  One (2-byte) synchronization word \n");
        printf("  One (2-byte) bit-rate word \n");
        printf("\n");
        printf("bitrate = 0 (6.4 kb/s), 1 (8 kb/s) (default : 8 kb/s)\n");
        printf("Format for bitrate_file:\n");
        printf("  1 16bit-Word per frame , =0 bit rate 6.4 kb/s, =1 bit rate 8 kb/s \n");
        printf("DTX flag:\n");
        printf("  0 to disable the DTX\n");
        printf("  1 to enable the DTX\n");
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
    
    dtx_enable = (Word16)atoi(argv[3]);
    if (dtx_enable == 1)
        printf(" DTX enabled\n");
    else
        printf(" DTX disabled\n");
    f_rate = NULL; /* to avoid  visual warning */
    rate = G729;  /* to avoid  visual warning */
    
    if(argc != 4) {
        if ( (f_rate = fopen(argv[4], "rb")) == NULL) {
            rate  = (Word16)atoi(argv[4]);
            if( rate == 1) printf(" Selected Bitrate      :  8.0 kb/s\n");
            else {
                if( rate == 0) printf(" Selected Bitrate      :  6.4 kb/s\n");
                else {
                    printf(" error bit rate indication\n");
                    printf(" argv[4] = 0 for bit rate 6.4 kb/s \n");
                    printf(" argv[4] = 1 for bit rate 8 kb/s \n");
                    exit(-1);
                }
            }
            flag_rate = 0;
        }
        else {
            printf(" Selected Bitrate  read in file :  %s kb/s\n", argv[4]);
            flag_rate = 1;
        }
    }
    else {
        flag_rate = 0;
        rate = G729;
        printf(" Selected Bitrate      :  8 kb/s\n");
    }
#ifndef OCTET_TX_MODE
    printf(" OCTET TRANSMISSION MODE is disabled\n");
#endif
    
    /*--------------------------------------------------------------------------*
    * Initialization of the coder.                                             *
    *--------------------------------------------------------------------------*/
    
    Init_Pre_Process();
    Init_Coder_ld8f(dtx_enable);

    for(i=0; i<PRM_SIZE; i++) prm[i] = (Word16)0;
    

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
    frame=0;
    count_frame = 0L;

    while( fread(new_speech, sizeof(Word16), L_FRAME, f_speech) == L_FRAME) {
        if( flag_rate == 1) {
            if( fread(&rate, sizeof(Word16), 1, f_rate) != 1) {
                printf("error reading bit_rate in file %s for frame %hd\n",
                    argv[4], frame);
                exit(-1);
            }
            if( (rate < 0) || (rate > 1) ) {
                printf("error bit_rate in file %s for frame %ld bit rate non avalaible\n", argv[4], count_frame);
                exit(-1);
            }
        }
#ifdef HARDW
        /* set 3 LSB's to zero */
        for(i=0; i < L_FRAME; i++) new_speech[i] = new_speech[i] & 0xFFF8;
#endif

        if (frame == 32767) frame = 256;
        else frame++;

#ifdef PREPROC
        Pre_Process(new_speech, L_FRAME);
#else
        /* Division par 2 si pas de preprocessing */
        for(i=0; i < L_FRAME; i++) new_speech[i] = shr(new_speech[i], 1);
#endif

        count_frame++;
        printf(" Frame: %ld\r", count_frame);
        CODEC_MODE = add(rate,1); /* used by G729D routine */
        Coder_ld8f(prm, frame, dtx_enable, rate);

        prm2bits_ld8f( prm, serial);

        nb_words = (Word16)serial[1] +  (Word16)2;

        if (fwrite(serial, sizeof(Word16), nb_words, f_serial) != (size_t)nb_words)
            printf("Write Error for frame %ld\n", count_frame);

    }
    printf("\n");
    printf("%ld frames processed\n", count_frame);

    if(f_serial) fclose(f_serial);
    if(f_speech) fclose(f_speech);
    if(f_rate) fclose(f_rate);

    return(0);
    
} /* end of main() */

