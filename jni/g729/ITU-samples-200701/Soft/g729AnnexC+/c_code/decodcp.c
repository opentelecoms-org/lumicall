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
 File : DECODCP.C
 */

/*-----------------------------------------------------------------*
* Main program of the G.729C+ 6.4/8.0/11.8 kbit/s decoder.        *
*                                                                 *
*    Usage : decodcp  bitstream_file  synth_file                  *
*                                                                 *
*-----------------------------------------------------------------*/

#if defined(__BORLANDC__)
extern unsigned _stklen = 48000U;
#endif

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "dtx.h"
#include "octet.h"

/*-----------------------------------------------------------------*
*            Main decoder routine                                 *
*-----------------------------------------------------------------*/

int main( int argc, char *argv[])
{
    FLOAT  synth_buf[L_ANA_BWD], *synth;   /* Synthesis                   */
    int    parm[PRM_SIZE_E+3];             /* Synthesis parameters        */
    INT16  serial[SERIAL_SIZE_E];          /* Serial stream               */
    FLOAT  Az_dec[M_BWDP1*2];              /* Decoded Az for post-filter  */
    int    T0_first;                       /* Pitch lag in 1st subframe   */
    FLOAT  pst_out[L_FRAME];               /* Postfilter output           */
    
    int  voicing;                        /* voicing from previous frame */
    
    int  long_h_st;
    int  sf_voic;                        /* voicing for subframe        */
    FLOAT *ptr_Az;
    FLOAT  ga1_post, ga2_post, ga_harm;
    int serial_size_prec = RATE_8000;

    int  i;
    int  m_pst;
    int  serial_size;
    int  bwd_dominant;
    FILE    *f_syn, *f_serial;
    
    INT32   frame;
    int     Vad;

    printf("\n");
    printf("*************************************************************************\n");
    printf("****    ITU G.729 ANNEC C+: 6.4, 8.0, and 11.8 KBIT/S SPEECH DECODER ****\n");
    printf("****         INCLUDING OPTIONAL VAD/DTX/CNG (ANNEX B)                ****\n");
    printf("*************************************************************************\n");
    printf("\n");
    printf("------------------ Floating point C simulation ----------------\n");
    printf("\n");
    printf("------------- Version 2.2 (Release 2, November 2006) ----------\n");
    printf("\n");
    printf("                 Bit rates : 6.4, 8.0, or 11.8 kb/s \n");
    printf("\n");
    
    /* Passed arguments */
    if ( argc != 3 ) {
        printf("Usage : decodcp bitstream_file  output_file  \n");
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
        
#ifndef OCTET_TX_MODE
    printf("OCTET TRANSMISSION MODE is disabled\n");
#endif
        
    /*-----------------------------------------------------------------*
    *           Initialization of decoder                             *
    *-----------------------------------------------------------------*/
    for (i=0; i<L_ANA_BWD; i++) synth_buf[i] = (F)0.;
    synth = synth_buf + MEM_SYN_BWD;
    
    init_decod_ld8c();
    init_post_filter();
    init_post_process();
    
    voicing = 60;

    frame = 0;

    ga1_post = GAMMA1_PST_E;
    ga2_post = GAMMA2_PST_E;
    ga_harm  = GAMMA_HARM_E;

    /* for G.729b */
    init_dec_cng();

    /*-----------------------------------------------------------------*
    *            Loop for each "L_FRAME" speech data                  *
    *-----------------------------------------------------------------*/
    while( fread(serial, sizeof(INT16), 2, f_serial) == 2) {

        serial_size = (int)serial[1];
        if(serial_size != 0) {
            if (fread(&serial[2], sizeof(INT16), serial_size, f_serial) !=
                (size_t)serial_size) {
                printf("error reading file %s frame %d\n", argv[1], frame);
                exit(-1);
            }
        }
        frame++;

        printf(" Frame: %ld\r", frame);

        bits2prm_ld8c(&serial[1], parm);

        if( (serial_size ==80) ) {
            parm[5] = check_parity_pitch(parm[4], parm[5]);
        }
        else 
            if (serial_size == 118) {
                /* ------------------------------------------------------------------ */
                /* check parity and put 1 in parm[6] if parity error in Forward mode  */
                /*                  put 1 in parm[4] if parity error in Backward mode */
                /* ------------------------------------------------------------------ */
                if (parm[2] == 0) {
                    i = (parm[5] >> 1) & 1;
                    parm[6] += i;
                    parm[6] = check_parity_pitch(parm[5], parm[6]);
                }
                else {
                    i = (parm[3] >> 1) & 1;
                    parm[4] += i;
                    parm[4] = check_parity_pitch(parm[3], parm[4]);
                }
            }

        /* for speech and SID frames, the hardware detects frame erasures
        by checking if all bits are set to zero */
        /* for untransmitted frames, the hardware detects frame erasures
        by testing serial[0] */

        parm[0] = 0;           /* No frame erasure */
        if(serial[1] != 0) {
            for (i=0; i < serial[1]; i++)
                if (serial[i+2] == 0 ) parm[0] = 1;  /* frame erased     */
        }
        else if(serial[0] != SYNC_WORD) parm[0] = 1;


        if (parm[0] == 1) {
            printf("Frame Erased : %d\n", frame);
            serial_size = serial_size_prec;
            if(serial_size < RATE_6400) {
                serial_size = 0;
            }
        }


        /* ---------- */
        /*  Decoding  */
        /* ---------- */
        decod_ld8c(parm, voicing, synth_buf, Az_dec,
            &T0_first, &bwd_dominant, &m_pst, &Vad);

        /* ---------- */
        /* Postfilter */
        /* ---------- */
        ptr_Az = Az_dec;

        /* Adaptive parameters for postfiltering */
        /* ------------------------------------- */
        if (serial_size != 118) {
            long_h_st = LONG_H_ST;
            ga1_post = GAMMA1_PST;
            ga2_post = GAMMA2_PST;
            ga_harm = GAMMA_HARM;
        }
        else {
            long_h_st = LONG_H_ST_E;
            /* If backward mode is dominant => progressively reduce postfiltering */
            if ((parm[2] == 1) && (bwd_dominant == 1)) {
                ga_harm -= (F)0.0125;
                if (ga_harm < 0) ga_harm = 0;
                ga1_post -= (F)0.035;
                if (ga1_post < 0) ga1_post = 0;
                ga2_post -= (F)0.0325;
                if (ga2_post < 0) ga2_post = 0;
            }
            else {
                ga_harm += (F)0.0125;
                if (ga_harm > GAMMA_HARM_E) ga_harm = GAMMA_HARM_E;
                ga1_post += (F)0.035;
                if (ga1_post > GAMMA1_PST_E) ga1_post = GAMMA1_PST_E;
                ga2_post += (F)0.0325;
                if (ga2_post > GAMMA2_PST_E) ga2_post = GAMMA2_PST_E;
            }
        }
        for(i=0; i<L_FRAME; i++) pst_out[i] = synth[i];

        voicing = 0;
        for(i=0; i<L_FRAME; i+=L_SUBFR) {
            poste(T0_first, &synth[i], ptr_Az, &pst_out[i], &sf_voic,
                ga1_post, ga2_post, ga_harm, long_h_st, m_pst, Vad);
            if (sf_voic != 0) voicing = sf_voic;
            ptr_Az += m_pst+1;
        }

        post_process(pst_out, L_FRAME);
        fwrite16(pst_out, L_FRAME, f_syn);

        serial_size_prec = serial_size;
    }
    printf("\n");
    if(f_serial) fclose(f_serial);
    if(f_syn) fclose(f_syn);
    return(0);
}


