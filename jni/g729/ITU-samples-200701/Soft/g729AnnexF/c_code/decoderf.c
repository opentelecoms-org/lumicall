/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
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
 File : DECODERF.C
 */
/* from decoderd.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from decoder.c G.729 Annex B Version 1.3  Last modified: August 1997 */
/* from decoder.c G.729 Version 3.3  */

/*--------------------------------------------------------------------------------------*
 * Main program of the ITU-T G.729F   8/6.4 kbit/s encoder.
 *
 *    Usage : decoderf bitstream_file  output_file
 *--------------------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------ */
/*                            MAIN PROGRAM                                  */
/* ------------------------------------------------------------------------ */

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8f.h"
#include "dtx.h"
#include "octet.h"

#define POSTPROC

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main(int argc, char *argv[] )
{
    Word16 Vad;
    Word16  synth_buf[L_FRAME+M], *synth; /* Synthesis                   */
    Word16  parm[PRM_SIZE+3];             /* Synthesis parameters        */
    Word16  serial[SERIAL_SIZE];            /* Serial stream               */
    Word16  Az_dec[MP1*2], *ptr_Az;       /* Decoded Az for post-filter  */
    Word16  T0_first;                         /* Pitch lag in 1st subframe   */
    Word16  pst_out[L_FRAME];                 /* Postfilter output           */

    Word16  voicing;                          /* voicing from previous frame */
    Word16  sf_voic;                          /* voicing for subframe        */

    Word16  i;
    Word32 frame;
    Word16  serial_size;
    FILE    *f_syn, *f_serial;

    printf("\n");
    printf("*************************************************************************\n");
    printf("****    ITU G.729 ANNEX F : 6.4, 8.0 KBIT/S SPEECH DECODER   ****\n");
    printf("****         INCLUDING OPTIONAL VAD/DTX/CNG (ANNEX B)                ****\n");
    printf("*************************************************************************\n");
    printf("\n");
    printf("------------------ Fixed point C simulation ----------------\n");
    printf("\n");
    printf("------------ Version 1.2 (Release 2, November 2006) --------\n");
    printf("\n");
    printf("                 Bit rates : 6.4 or 8.0 kb/s \n");
    printf("\n");

    /* Passed arguments */
    if ( argc != 3 ) {
        printf("Usage : decoderf bitstream_file  output_file  \n");
        printf("\n");
        printf("Format for bitstream_file:\n");
        printf("  One (2-byte) synchronization word,\n");
        printf("  One (2-byte) bit-rate word,\n");
        printf("\n");
        printf("Format for outputspeech_file:\n");
        printf("  Output is written to a binary file of 16 bits data.\n");
        exit( 1 );
    }
    
    /* Open file for synthesis and packed serial stream */
    if( (f_serial = fopen(argv[1],"rb") ) == NULL ) {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
        exit(0);
    }
    if( (f_syn = fopen(argv[2], "wb") ) == NULL ) {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
        exit(0);
    }
    
    printf("Input bitstream file    :   %s\n\n",argv[1]);
    printf("Synthesis speech file   :   %s\n\n",argv[2]);
    
    /*-----------------------------------------------------------------*
    *           Initialization of decoder                             *
    *-----------------------------------------------------------------*/
    for (i=0; i<M; i++) synth_buf[i] = 0;
    synth = synth_buf + M;
    
    Init_Decod_ld8f();
    Init_Post_Filter();
    Init_Post_Process();

    voicing = 60;

    /* for G.729b */
    Init_Dec_cng();
    
    frame = 0L;
    /*-----------------------------------------------------------------*
    *            Loop for each "L_FRAME" speech data                  *
    *-----------------------------------------------------------------*/
    while( fread(serial, sizeof(Word16), 2, f_serial) == 2) {
        
        serial_size = serial[1];
        if(serial_size != 0) {
            if(serial[0] != SYNC_WORD ) {
                printf("error reading synchro word file %s frame %ld\n",
                    argv[1], frame);
                exit(-1);
            }
            if( (serial_size !=  RATE_6400) && (serial_size != RATE_8000) ) {
#ifndef OCTET_TX_MODE
                if(serial_size != RATE_SID)
#else
                if(serial_size != RATE_SID_OCTET) 
#endif
                {
                    printf("error uncorrect value of serial_size (%hd) in %s frame %ld\n",
                            serial_size, argv[1], frame);
                    exit(-1);
                }
            }
            if (fread(&serial[2], sizeof(Word16), serial_size, f_serial) !=
                (size_t)serial_size) {
                printf("error reading file %s frame %ld\n", argv[1], frame);
                exit(-1);
            }
        }
        frame++;
        printf(" Frame: %ld\r", frame);
        bits2prm_ld8f(&serial[1], parm);

        /* for speech and SID frames, the hardware detects frame erasures
        by checking if all bits are set to zero */
        /* for untransmitted frames, the hardware detects frame erasures
        by testing serial[0] */
        parm[0] = 0;           /* No frame erasure */
        if(serial[1] != 0) {
            for (i=0; i < serial[1]; i++)
                if (serial[i+2] == 0 ) parm[0] = 1; /* frame erased     */
        }
        else if(serial[0] != SYNC_WORD) parm[0] = 1;
        
        if (parm[0] == 1) printf("Frame Erased : %ld\n", frame);
        if( (serial_size ==80) ) {
            parm[5] = Check_Parity_Pitch(parm[4], parm[5]);
        }

        /* ---------- */
        /*  Decoding  */
        /* ---------- */
        Decod_ld8f(parm, voicing, synth, Az_dec, &T0_first, &Vad);

        /* ---------- */
        /* Postfilter */
        /* ---------- */
        for(i=0; i<L_FRAME; i++) pst_out[i] = synth[i];

        voicing = 0;
        ptr_Az = Az_dec;
        if(sub(Vad, 1) > 0) Vad = 1;
        else Vad = 0;
        for(i=0; i<L_FRAME; i+=L_SUBFR) {
            Post(T0_first, &synth[i], ptr_Az, &pst_out[i], &sf_voic, Vad);
            if (sf_voic != 0) voicing = sf_voic;
            ptr_Az += MP1;
        }
      Copy(&synth_buf[L_FRAME], &synth_buf[0], M);
        
#ifdef POSTPROC
        Post_Process(pst_out, L_FRAME);
#else
        for (i=0; i<L_FRAME; i++) pst_out[i] = shl(pst_out[i],1);
#endif
        
#ifdef HARDW
        {
            Word16 *my_pt;
            Word16 my_temp;
            Word16 my_i;
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
    }
    printf("\n");
    if(f_serial) fclose(f_serial);
    if(f_syn) fclose(f_syn);
    
    return(0);
}


