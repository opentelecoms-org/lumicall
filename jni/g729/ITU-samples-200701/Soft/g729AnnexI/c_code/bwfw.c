/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
                         Version 1.1 of October 1999
*/
/*
 File : bwfw.c
 */
/* from bwfw.c G.729 Annex E Version 1.2  Last modified: May 1998 */

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
#include "basic_op.h"
#include "oper_32b.h"

static void calc_stat(Word16 gpred_b, Word16 gpred_f, Word16 mode,
 Word16 prev_mode, Word16 *glob_stat, Word16 *stat_bwd, Word16 *val_stat_bwd);

/* ---------------------------------------------------------------------- */
/*                              SET_LPC_MODE                              */
/*                                                                        */
/*                    BACKWARD <--> FORWARD DECISION                      */
/* ---------------------------------------------------------------------- */

void set_lpc_mode ( Word16 *signal_ptr,  /* I   Input signal */
                    Word16 *a_fwd,       /* I   Forward LPC filter */
                    Word16 *a_bwd,       /* I   Backward LPC filter */
                    Word16 *mode,        /* O  Backward / forward Indication */
                    Word16 *lsp_new,     /* I   LSP vector current frame */
                    Word16 *lsp_old,     /* I   LSP vector previous frame */
                    Word16 *bwd_dominant,/* O   Bwd dominant mode indication */
                    Word16 prev_mode,     /* I   previous frame Backward / forward Indication */
                    Word16 *prev_filter,  /* I   previous frame filter */
                    Word16 *C_int,       /*I/O filter interpolation parameter */
                    Word16 *glob_stat,   /* I/O Mre of global stationnarity Q8 */
                    Word16 *stat_bwd,   /* I/O Number of consecutive backward frames */
                    Word16 *val_stat_bwd/* I/O Value associated with stat_bwd */
)
{

    Word16  i;
    Word16  res[L_FRAME];
    Word16  *pa_bwd;
    Word16  gap;
    Word16  gpred_f, gpred_b, gpred_bint;
    Word16  tmp;
    Word32  thresh_lpc_L;
    Word32  tmp_L;
    Word32  dist_lsp, energy;


    pa_bwd = a_bwd + M_BWDP1;
    /* Backward filter prediction gain (no interpolation ) */
    /* --------------------------------------------------- */
    Residue(M_BWD,pa_bwd, signal_ptr, res, L_FRAME);
    gpred_b = ener_dB(signal_ptr, L_FRAME) - ener_dB(res, L_FRAME);
    
    /* Interpolated LPC filter for transition forward -> backward */
    /* (used during 10 frames) ( for the second sub-frame )       */
    /* Interpolated backward filter for the first sub-frame       */
    /* ---------------------------------------------------------- */
    Int_bwd(a_bwd, prev_filter, C_int);
    
    /* Interpolated backward filter prediction gain */
    /* -------------------------------------------- */
    Residue(M_BWD,a_bwd, signal_ptr, res, L_SUBFR);
    Residue(M_BWD,pa_bwd, &signal_ptr[L_SUBFR], &res[L_SUBFR], L_SUBFR);
    gpred_bint = ener_dB(signal_ptr, L_FRAME) - ener_dB(res, L_FRAME);
    
    
    /* Forward filter prediction gain */
    /* ------------------------------ */
    Residue(M, a_fwd, signal_ptr, res, L_SUBFR);
    Residue(M, &a_fwd[MP1], &signal_ptr[L_SUBFR], &res[L_SUBFR], L_SUBFR);
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
    tmp = shr(*glob_stat, 7);
    tmp_L = L_mult(tmp, 3);
    tmp_L = L_shr(tmp_L, 1);
    tmp = extract_l(tmp_L);
    gap = add(tmp, 205);
    
    if ( (gpred_bint > gpred_f - gap)&&
        (gpred_b > gpred_f - gap)&&
        (gpred_b > 0)&&
        (gpred_bint > 0) ) *mode = 1;
    
    else *mode = 0;
    
    if (*glob_stat < 13000) *mode = 0; /* => Forward mode imposed */
    
    /* 2nd criterion with a distance between 2 successive LSP vectors */
    /* -------------------------------------------------------------- */
    /* Computation of the LPC distance */
    dist_lsp = 0;
    for(i=0; i<M; i++){
        tmp = sub(lsp_old[i],lsp_new[i]);
        dist_lsp = L_mac(dist_lsp, tmp, tmp);
    }
    
    /* Adaptation of the LSPs thresholds */
    if (*glob_stat < 32000) {
        thresh_lpc_L = 0L;
    }
    else {
        thresh_lpc_L = 64424509L;
    }
    
    /* Switching backward -> forward forbidden in case of a LPC stationnary */
    if ((dist_lsp < thresh_lpc_L)&&(*mode == 0)&&(prev_mode == 1)&&(gpred_b > 0)&&(gpred_bint > 0)) {
        
        *mode = 1;
    }
    
    /* Low energy frame => Forward mode chosen */
    /* --------------------------------------- */
    energy = ener_dB(signal_ptr, L_FRAME);
    
    if (energy < 8192) {
        *mode = 0;
        if (*glob_stat > 13000) *glob_stat = 13000;
    }
    else tst_bwd_dominant(bwd_dominant, *mode);
    
    /* Adaptation of the global stationnarity indicator */
    /* ------------------------------------------------ */
    if (energy >= 8192) calc_stat(gpred_b, gpred_f, *mode, prev_mode,
        glob_stat, stat_bwd, val_stat_bwd);
    if(*mode == 0) *C_int = 4506;
    return;
    
}
void update_bwd(Word16 *mode,        /* O  Backward / forward Indication */
                Word16 *bwd_dominant,/* O   Bwd dominant mode indication */
                Word16 *C_int,       /*I/O filter interpolation parameter */
                Word16 *glob_stat   /* I/O Mre of global stationnarity */
)
{

    /* BIT RATES IN FORWARD MODE ONLY */
    /* ------------------------------ */
   if (*glob_stat > 10000) {
        *glob_stat = sub(*glob_stat, 2621);
        if( *glob_stat < 10000) *glob_stat = 10000 ;
    }
    *mode = 0;
    *bwd_dominant = 0;
    *C_int = 4506;
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

static void calc_stat( Word16 gpred_b, /* I  Backward prediction gain */
                Word16 gpred_f, /* I  Forward prediction gain */
                Word16 mode,      /* I  LPC mode indication */
                Word16 prev_mode, /* I  previous frame LPC mode */
                Word16 *glob_stat,   /* I/O Mre of global stationnarity Q8 */
                Word16 *stat_bwd,   /* I/O Number of consecutive backward frames */
                Word16 *val_stat_bwd/* I/O Value associated with stat_bwd */
)
{

    Word16 tmp;

    /* --------------------------------------------------------------- */
    /* First adaptation based on previous backward / forward decisions */
    /* --------------------------------------------------------------- */

    if (mode == 1) { /* Backward stationnary mode */

        *stat_bwd = add(*stat_bwd, 1);
        *val_stat_bwd = add(*val_stat_bwd, 250);

        /* after 20 backward frames => increase stat */
        if (*stat_bwd == 20) *glob_stat = add(*glob_stat, 2500);
        else if (*stat_bwd > 20) *glob_stat = add(*glob_stat, 500);

    }

    else if ((mode == 0)&&(prev_mode == 1)) { /* Backward -> Forward transition */

        /* Transition occurs after less than 20 backward frames => decrease stat */
        if (*stat_bwd < 20) {
            tmp = sub(5000, *val_stat_bwd);
            *glob_stat = sub(*glob_stat, tmp);
        }

        /* Reset consecutive backward frames counter */
        *stat_bwd = 0;
        *val_stat_bwd = 0;

    }


    /* ------------------------------------------- */
    /* Second adaptation based on prediction gains */
    /* ------------------------------------------- */

    if (*glob_stat < 13000) {

        if      (gpred_b > gpred_f + 819) *glob_stat += 3200;
        else if (gpred_b > gpred_f + 614) *glob_stat += 2400;
        else if (gpred_b > gpred_f + 410) *glob_stat += 1600;
        else if (gpred_b > gpred_f + 205) *glob_stat +=  800;
        else if (gpred_b > gpred_f +   0) *glob_stat +=  400;

    }

    if      (gpred_b < gpred_f -  955) *glob_stat -= 6400; /* < 0.47 */
    else if (gpred_b < gpred_f -  819) *glob_stat -= 3200; /* < 0.4 */
    else if (gpred_b < gpred_f -  614) *glob_stat -= 1600; /* < 0.3 */
    else if (gpred_b < gpred_f -  410) *glob_stat -=  800; /* < 0.2 */
    else if (gpred_b < gpred_f -  205) *glob_stat -=  400; /* < 0.2 */

    if( *glob_stat > 32000) *glob_stat = 32000;
    else {
        if(*glob_stat < 0) *glob_stat = 0;
    }
    return;
}
