/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
   Version 2.2    Last modified: October 2006 
*/
/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C+ floating point ANSI C source code
   Copyright (C) 1999, AT&T, France Telecom, NTT, University of
   Sherbrooke, Conexant, Ericsson. All rights reserved.
----------------------------------------------------------------------
*/

/*
 File : CODERCP.C
 */

/*--------------------------------------------------------------------------------------*
 * Main program of the ITU-T G.729C+   11.8/8/6.4 kbit/s encoder.
 *
 *    Usage : codercp speech_file  bitstream_file  DTX_flag [bit_rate or file_bit_rate]
 *--------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "dtx.h"
#include "octet.h"
#if defined(__BORLANDC__)
extern unsigned _stklen = 48000U;
#endif

int main( int argc, char *argv[])
{
    FILE *f_speech;                     /* Speech data        */
    FILE *f_serial;                     /* Serial bit stream  */
    FILE  *f_rate;
    int rate, flag_rate;
    
    extern FLOAT *new_speech;           /* Pointer to new speech data   */
    INT32  count_frame;
    
    int prm[PRM_SIZE_E+1];          /* Analysis parameters.                  */
    INT16 serial[SERIAL_SIZE_E];  /* Output bitstream buffer               */
    INT16 sp16[L_FRAME];          /* Buffer to read 16 bits speech */
    
    int i;
    int frame;               /* frame counter for VAD*/
    INT16 temp16;
    
    /* For G.729B */
    int nb_words;
    int dtx_enable;
    
    for(i = 0; i < argc; i++){
        printf("argument %d : %s\n", i, argv[i]);
    }
    printf("\n");
    printf("*************************************************************************\n");
    printf("****    ITU G.729 ANNEC C+: 6.4, 8.0, and 11.8 KBIT/S SPEECH CODER   ****\n");
    printf("****         INCLUDING OPTIONAL VAD/DTX/CNG (ANNEX B)                ****\n");
    printf("*************************************************************************\n");
    printf("\n");
    printf("------------------ Floating point C simulation ----------------\n");
    printf("\n");
    printf("------------- Version 2.2 (Release 2, November 2006) ----------\n");
    printf("\n");
    printf("                 Bit rates : 6.4, 8.0, or 11.8 kb/s \n");
    printf("\n");
    
    /*-----------------------------------------------------------------------*
    * Open speech file and result file (output serial bit stream)           *
    *-----------------------------------------------------------------------*/
    if (( argc != 4 ) && (argc != 5) ){
        printf("Usage : codercp speech_file bitstream_file DTX_flag [bitrate or file_bit_rate]\n");
        printf("Format for speech_file:\n");
        printf("  Speech is read from a binary file of 16 bits PCM data.\n");
        printf("\n");
        printf("Format for bitstream_file:\n");
        printf("  One (2-byte) synchronization word \n");
        printf("  One (2-byte) bit-rate word \n");
        printf("\n");
        printf("bitrate = 0 (6.4 kb/s), 1 (8 kb/s)  or 2 (11.8 kb/s)  (default : 8 kb/s)\n");
        printf("Format for bitrate_file:\n");
        printf("  1 16bit-Word per frame , =0 bit rate 6.4 kb/s, =1 bit rate 8 kb/s, or =2 bit rate 11.8 kb/s \n");
        printf("Forward / Backward structure at 11.8 kb/s \n");
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

    dtx_enable = (int)atoi(argv[3]);
    if (dtx_enable == 1)
        printf(" DTX enabled\n");
    else
        printf(" DTX disabled\n");

    f_rate = NULL; /* to avoid  visual warning */
    rate = G729;  /* to avoid  visual warning */
    if(argc != 4) {
        if ( (f_rate = fopen(argv[4], "rb")) == NULL) {
            rate  = atoi(argv[4]);
            if( rate == G729E) printf(" Selected Bitrate   :  11.8 kb/s (G.729 Annex E)\n");
            else 
                if( rate == G729)  printf(" Selected Bitrate   :  8.0 kb/s  (G.729 Main Recommendation)\n");
                else 
                    if( rate == G729D) printf(" Selected Bitrate   :  6.4 kb/s  (G.729 Annec D)\n");
                    else {
                        printf(" error bit rate indication\n");
                        printf(" argv[4] = 0 for bit rate 6.4 kb/s (G.729D)\n");
                        printf(" argv[4] = 1 for bit rate 8 kb/s (G.729)\n");
                        printf(" argv[4] = 2 for bit rate 11.8 kb/s (G.729E)\n");
                        exit(-1);
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
        printf(" Selected Bitrate   :  8.0 kb/s  (G.729 Main Recommendation)\n");
    }
    
#ifndef OCTET_TX_MODE
        printf(" OCTET TRANSMISSION MODE is disabled\n");
#endif
        
    /*-------------------------------------------------*
    * Initialization of the coder.                    *
    *-------------------------------------------------*/
        
    init_pre_process();
    init_coder_ld8c(dtx_enable);           /* Initialize the coder             */

    /* for G.729B */
    if(dtx_enable == 1) init_cod_cng();

    for(i=0; i<PRM_SIZE_E; i++) prm[i] = 0;

    /* To force the input and output to be time-aligned the variable SYNC
    has to be defined. Note: the test vectors were generated with this option
    disabled
    */

#ifdef SYNC
    /* Read L_NEXT first speech data */
    fread(sp16, sizeof(INT16), L_NEXT, f_speech);

    for (i = 0; i < L_NEXT; i++)  new_speech[-L_NEXT+i] = (FLOAT) sp16[i];
    pre_process(&new_speech[-L_NEXT], L_NEXT);
#endif

   /*-------------------------------------------------------------------------*
    * Loop for every analysis/transmission frame.                             *
    * -New L_FRAME data are read. (L_FRAME = number of speech data per frame) *
    * -Conversion of the speech data from 16 bit integer to real              *
    * -Call cod_ld8c to encode the speech.                                    *
    * -The compressed serial output stream is written to a file.              *
    *-------------------------------------------------------------------------*
    */
        
    frame=0;
    count_frame = 0L;
    
    while( fread((void *)sp16, sizeof(INT16), L_FRAME, f_speech) == L_FRAME){
        if( flag_rate == 1) {
            if( fread(&temp16, sizeof(INT16), 1, f_rate) != 1) {
                printf("error reading bit_rate in file %s for frame %ld\n", argv[4], count_frame);
                exit(-1);
            }
            rate = (int)temp16;
            if( (rate < 0) || (rate > 2) ) {
                printf("error bit_rate in file %s for frame %ld bit rate non avalaible\n", argv[4], count_frame);
                exit(-1);
            }
        }
        
        count_frame++;
        printf(" Frame: %ld\r", count_frame);
        
        if (frame == 32767) frame = 256;
        else frame++;
        
        for (i = 0; i < L_FRAME; i++)  new_speech[i] = (FLOAT) sp16[i];
        
        pre_process( new_speech, L_FRAME);
        
        coder_ld8c(prm, frame, dtx_enable, rate);
        
        prm2bits_ld8c(prm, serial);
        
        nb_words = (int)serial[1] +  2;
        fwrite( (void *)serial, sizeof(INT16), nb_words,  f_serial);
    }
    
    printf("%ld frames processed\n", count_frame);

    if(f_serial) fclose(f_serial);
    if(f_speech) fclose(f_speech);

    return(0);

} /* end of main() */
