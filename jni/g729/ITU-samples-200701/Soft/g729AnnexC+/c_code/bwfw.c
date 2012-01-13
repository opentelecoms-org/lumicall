/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/
                                   
/* ----------------------------------------------------------------- */
/*               DUAL BACKWARD / FORWARD LPC STRUCTURE               */
/*                                                                   */
/*                 switching and decision procedures                 */
/*                                                                   */
/*                                                                   */
/*                 (C) Copyright 1997 : France Telecom               */
/*                                                                   */
/* ----------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Procedures :             1) SET_LPC_MODE                               */
/*                           3) CALC_STAT                                  */
/*                                                                         */
/* ----------------------------------------------------------------------- */
#include <stdio.h>
#include <math.h>

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

static void calc_stat(FLOAT gpred_b, FLOAT gpred_f, int mode,
 int prev_mode, INT16 *glob_stat, INT16 *stat_bwd, INT16 *val_stat_bwd);

/* ---------------------------------------------------------------------- */
/*                              SET_LPC_MODE                              */
/*                                                                        */
/*                    BACKWARD <--> FORWARD DECISION                      */
/* ---------------------------------------------------------------------- */

void set_lpc_mode ( FLOAT *signal_ptr,  /* I   Input signal */
                    FLOAT *a_fwd,       /* I   Forward LPC filter */
                    FLOAT *a_bwd,       /* I   Backward LPC filter */
                    int *mode,          /* O  Backward / forward Indication */
                    FLOAT *lsp_new,     /* I   LSP vector current frame */
                    FLOAT *lsp_old,     /* I   LSP vector previous frame */
                    int *bwd_dominant,  /* O   Bwd dominant mode indication */
                    int prev_mode,      /* I   previous frame Backward / forward Indication */
                    FLOAT *prev_filter, /* I   previous frame filter */
                    FLOAT *C_int,       /*I/O filter interpolation parameter */
                    INT16 *glob_stat,   /* I/O Mre of global stationnarity */
                    INT16 *stat_bwd,    /* I/O Number of consecutive backward frames */
                    INT16 *val_stat_bwd /* I/O Value associated with stat_bwd */
)
{

    int     i;
    FLOAT  res[L_FRAME];
    FLOAT  *pa_bwd;
    FLOAT  gap;
    FLOAT  gpred_f, gpred_b, gpred_bint;
    FLOAT  tmp;
    FLOAT  thresh_lpc;
    FLOAT  dist_lsp, energy;

    pa_bwd = a_bwd + M_BWDP1;
    /* Backward filter prediction gain (no interpolation ) */
    /* --------------------------------------------------- */
    residue(M_BWD,pa_bwd, signal_ptr, res, L_FRAME);
    gpred_b = ener_dB(signal_ptr, L_FRAME) - ener_dB(res, L_FRAME);

    /* Interpolated LPC filter for transition forward -> backward */
    /* (used during 10 frames) ( for the second sub-frame )       */
    /* Interpolated backward filter for the first sub-frame       */
    /* ---------------------------------------------------------- */
    int_bwd(a_bwd, prev_filter, C_int);

    /* Interpolated backward filter prediction gain */
    /* -------------------------------------------- */
    residue(M_BWD,a_bwd, signal_ptr, res, L_SUBFR);
    residue(M_BWD,pa_bwd, &signal_ptr[L_SUBFR], &res[L_SUBFR], L_SUBFR);
    gpred_bint = ener_dB(signal_ptr, L_FRAME) - ener_dB(res, L_FRAME);

    /* Forward filter prediction gain */
    /* ------------------------------ */
    residue(M, a_fwd, signal_ptr, res, L_SUBFR);
    residue(M, &a_fwd[MP1], &signal_ptr[L_SUBFR], &res[L_SUBFR], L_SUBFR);
    gpred_f = ener_dB(signal_ptr, L_FRAME) - ener_dB(res, L_FRAME);

    /* -------------------------------------------------------------- */
    /*                  BACKWARD / FORWARD DECISION                   */
    /*                                                                */
    /* The Main criterion is based on the prediction gains :          */
    /* The global stationnarity index is used to adapt the threshold  */
    /* value " GAP ".                                                 */
    /* This adaptation is used to favour one mode according to the    */
    /* stationnarity of the input signal (backward for music and      */
    /* forward for speech) which avoids too many switches.            */
    /*                                                                */
    /* A second criterion based on the LSPs is used to avoid switches */
    /* if the successive LPC forward filters are very stationnary     */
    /* (which is measured a Euclidean distance on LSPs).              */
    /*                                                                */
    /* -------------------------------------------------------------- */

    /* 1st criterion with prediction gains */
    /* ----------------------------------- */

    /* Threshold adaptation according to the global stationnarity indicator */
    gap = (FLOAT)(*glob_stat) * GAP_FACT;
    gap += (F)1.;

    if ( (gpred_bint > gpred_f - gap)&& (gpred_b > gpred_f - gap)&&
         (gpred_b > (F)0.)               && (gpred_bint > (F)0.) ) *mode = 1;
    else *mode = 0;

    if (*glob_stat < 13000) *mode = 0; /* => Forward mode imposed */

    /* 2nd criterion with a distance between 2 successive LSP vectors */
    /* -------------------------------------------------------------- */
    /* Computation of the LPC distance */
    dist_lsp = 0;
    for(i=0; i<M; i++){
        tmp = lsp_old[i] - lsp_new[i];
        dist_lsp += tmp * tmp;
    }

    /* Adaptation of the LSPs thresholds */
    if (*glob_stat < 32000) {
        thresh_lpc = (F)0.;
    }
    else {
        thresh_lpc = (F)0.03;
    }

    /* Switching backward -> forward forbidden in case of a LPC stationnary */
    if ((dist_lsp < thresh_lpc) &&(*mode == 0)&&(prev_mode == 1)
         &&(gpred_b > (F)0.)&&(gpred_bint > (F)0.)) {
        *mode = 1;
    }

    /* Low energy frame => Forward mode chosen */
    /* --------------------------------------- */
    energy = ener_dB(signal_ptr, L_FRAME);

    if (energy < THRES_ENERGY) {
        *mode = 0;
        if (*glob_stat > 13000) *glob_stat = 13000;
    }
    else tst_bwd_dominant(bwd_dominant, *mode);

    /* Adaptation of the global stationnarity indicator */
    /* ------------------------------------------------ */
    if (energy >= THRES_ENERGY) calc_stat(gpred_b, gpred_f, *mode,
                            prev_mode, glob_stat, stat_bwd, val_stat_bwd);
    if(*mode == 0) *C_int = (F)1.1;
    return;
}
void update_bwd(int *mode,        /* O  Backward / forward Indication */
                int *bwd_dominant,/* O   Bwd dominant mode indication */
                FLOAT *C_int,       /*I/O filter interpolation parameter */
                INT16 *glob_stat   /* I/O Mre of global stationnarity */
)
{
    /* BIT RATES IN FORWARD MODE ONLY */
    /* ------------------------------ */
    if (*glob_stat > 10000) {
        *glob_stat -= 2621;
        if( *glob_stat < 10000) *glob_stat = 10000 ;
    }
    *mode = 0;
/*  tst_bwd_dominant(bwd_dominant, *mode);*/
    *bwd_dominant = 0;
    *C_int = (F)1.1;
    return;
}


/* ---------------------------------------------------------------------- */
/*                             CALC_STAT                                  */
/*                                                                        */
/* Adaptation of the global stationnarity indicator "stat" in [0 ; 32000] */
/*                                                                        */
/*                       0 => very low stationnariy                       */
/*                   32000 => very high stationnarity                     */
/*                                                                        */
/*            This indicator is based on the performances of              */
/*               the backward and forward LPC predictions                 */
/*                                                                        */
/* ---------------------------------------------------------------------- */

static void calc_stat( FLOAT gpred_b, /* I  Backward prediction gain */
                FLOAT gpred_f, /* I  Forward prediction gain */
                int mode,      /* I  LPC mode indication */
                int prev_mode, /* I  previous frame LPC mode */
                INT16 *glob_stat,   /* I/O Mre of global stationnarity  */
                INT16 *stat_bwd,   /* I/O Number of consecutive backward frames */
                INT16 *val_stat_bwd/* I/O Value associated with stat_bwd */
)
{
    INT16 s_temp;
    /* --------------------------------------------------------------- */
    /* First adaptation based on previous backward / forward decisions */
    /* --------------------------------------------------------------- */
    if (mode == 1) { /* Backward stationnary mode */

        (*stat_bwd)++;
        if(*stat_bwd > 21) *stat_bwd = 21;
        if(*val_stat_bwd < 32517) *val_stat_bwd  += 250;
        else *val_stat_bwd = 32767;

        /* after 20 backward frames => increase stat */
        if (*stat_bwd == 20) {
            if(*glob_stat < 30267) *glob_stat += 2500;
            else *glob_stat = 32767;
        }
        else if (*stat_bwd > 20) *glob_stat += 500;

    }

    else if ((mode == 0)&&(prev_mode == 1)) { /* Backward -> Forward transition */

        /* Transition occurs after less than 20 backward frames => decrease stat */
        if (*stat_bwd < 20) {
            s_temp = (INT16)(5000 - *val_stat_bwd);
            *glob_stat = (INT16)(*glob_stat-s_temp);
        }

        /* Reset consecutive backward frames counter */
        *stat_bwd = 0;
        *val_stat_bwd = 0;

    }


    /* ------------------------------------------- */
    /* Second adaptation based on prediction gains */
    /* ------------------------------------------- */

    if (*glob_stat < 13000) {

        if      (gpred_b > gpred_f + TH4) *glob_stat += 3200;
        else if (gpred_b > gpred_f + TH3) *glob_stat += 2400;
        else if (gpred_b > gpred_f + TH2) *glob_stat += 1600;
        else if (gpred_b > gpred_f + TH1) *glob_stat +=  800;
        else if (gpred_b > gpred_f +   (F)0.) *glob_stat +=  400;

    }

    if      (gpred_b < gpred_f -  TH5) *glob_stat -= 6400;
    else if (gpred_b < gpred_f -  TH4) *glob_stat -= 3200;
    else if (gpred_b < gpred_f -  TH3) *glob_stat -= 1600;
    else if (gpred_b < gpred_f -  TH2) *glob_stat -=  800;
    else if (gpred_b < gpred_f -  TH1) *glob_stat -=  400;

    if( *glob_stat > 32000) *glob_stat = 32000;
    else {
        if(*glob_stat < 0) *glob_stat = 0;
    }
    return;
}
