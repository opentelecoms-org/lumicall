/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
File : CALCEXC.C
*/

/* Computation of Comfort Noise excitation             */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "dtx.h"

/* Local functions */
static FLOAT gauss(INT16 *seed);

/*-----------------------------------------------------------*
* procedure calc_exc_rand                                   *
*           ~~~~~~~~~~~~~                                   *
*   Computes comfort noise excitation                       *
*   for SID and not-transmitted frames                      *
*-----------------------------------------------------------*/
void calc_exc_rand(
    FLOAT cur_gain,      /* (i)   :   target sample gain                 */
    FLOAT *exc,          /* (i/o) :   excitation array                   */
    INT16 *seed,         /* (i)   :   current Vad decision               */
    int flag_cod         /* (i)   :   encoder/decoder flag               */
)
{
    FLOAT excg[L_SUBFR];
    int   pos[4];
    FLOAT sign[4];
    FLOAT *cur_exc;
    FLOAT gp, ener, fact, inter_exc, k, delta, x1, x2, g;
    int   i, i_subfr, t0, frac;
    INT16 Gp, temp1, temp2;

    if(cur_gain == (F)0.) {

        for(i=0; i<L_FRAME; i++) {
            exc[i] = (F)0.;
        }
        gp = (F)0.;
        t0 = L_SUBFR+1;
        if(flag_cod != FLAG_DEC) {
            for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR) {
                update_exc_err(gp, t0);
            }
        }
        else {
            Update_PhDisp(gp,cur_gain);
            Update_PhDisp(gp,cur_gain);
        }
        return;
    }
    
    
    /* Loop on subframes */
    
    cur_exc = exc;
    
    for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR) {
        
        /* generate random adaptive codebook & fixed codebook parameters */
        /*****************************************************************/
        temp1   = random_g729c(seed);
        frac    = (int)(temp1 & (INT16)0x0003) - 1;
        if(frac == 2) frac = 0;
        temp1 >>= 2;
        t0      = (int)(temp1 & (INT16)0x003F) + 40;
        temp1 >>= 6;
        temp2   = (INT16)(temp1 & (INT16)0x0007);
        pos[0]  = 5 * (int)temp2;
        temp1 >>= 3;
        temp2   = (INT16)(temp1 & (INT16)0x0001);
        sign[0] = (F)2. * (FLOAT)temp2 - (F)1.;
        temp1 >>= 1;
        temp2   = (INT16)(temp1 & (INT16)0x0007);
        pos[1]  = 5 * (int)temp2 + 1;
        temp1 >>= 3;
        temp2   = (INT16)(temp1 & (INT16)0x0001);
        sign[1] = (F)2. * (FLOAT)temp2 - (F)1.;
        temp1   = random_g729c(seed);
        temp2   = (INT16)(temp1 & (INT16)0x0007);
        pos[2]  = 5 * (int)temp2 + 1;
        temp1 >>= 3;
        temp2   = (INT16)(temp1 & (INT16)0x0001);
        sign[2] = (F)2. * (FLOAT)temp2 - (F)1.;
        temp1 >>= 1;
        temp2   = (INT16)(temp1 & (INT16)0x000F);
        pos[3]  = (int)(temp2 & (INT16)0x0001) + 3; /* j+3*/
        temp2 >>= 1;
        temp2  &= (INT16)0x0007;
        pos[3] += 5 * (int)temp2;
        temp1 >>= 4;
        temp2   = (INT16)(temp1 & (INT16)0x0001);
        sign[3] = (F)2. * (FLOAT)temp2 - (F)1.;
        Gp      = (INT16)(random_g729c(seed) & (INT16)0x1FFF); /* < 0.5  */
        gp      = (FLOAT)Gp / (F)16384.;

        /* Generate gaussian excitation */
        /********************************/
        ener = (F)0.;
        for(i=0; i<L_SUBFR; i++) {
            excg[i] = gauss(seed);
            ener += excg[i] * excg[i];
        }
        
        /*  Compute fact = alpha x cur_gain * sqrt(L_SUBFR / ener) */
        /*  with alpha = 0.5, and multiply excg[] by fact          */
        fact = NORM_GAUSS * cur_gain;
        fact /= (FLOAT)sqrt(ener);
        for(i=0; i<L_SUBFR; i++) {
            excg[i] *= fact;
        }
        
        /* generate random  adaptive excitation */
        /****************************************/
        pred_lt_3(cur_exc, t0, frac, L_SUBFR);
        
        
        /* compute adaptive + gaussian exc -> cur_exc */
        /**********************************************/
        ener = (F)0.;
        for(i=0; i<L_SUBFR; i++) {
            cur_exc[i] *= gp;
            cur_exc[i] += excg[i];
            ener += cur_exc[i] * cur_exc[i];
        }
        
        
        /* Compute fixed code gain */
        /***************************/
        
        /**********************************************************/
        /*** Solve EQ(X) = 4 X**2 + 2 b X + c                     */
        /**********************************************************/
        
        /* Compute b = inter_exc */
        inter_exc = (F)0.;
        for(i=0; i<4; i++) {
            inter_exc += cur_exc[pos[i]] * sign[i];
        }
        
        /* Compute k = cur_gain x cur_gain x L_SUBFR */
        k = cur_gain * cur_gain * (F)L_SUBFR;
        
        /* Compute delta = b^2 - 4 c    */
        /* with c = ener - k            */
        delta = inter_exc * inter_exc - (F)4. * (ener - k);
        
        if(delta < (F)0.) {
            
            /* adaptive excitation = 0 */
            copy(excg, cur_exc, L_SUBFR);
            inter_exc = (F)0.;
            for(i=0; i<4; i++) {
                inter_exc += cur_exc[pos[i]] * sign[i];
            }
            /* Compute delta = b^2 - 4 c      */
            /* with c = - k x (1- alpha^2)    */
            delta = inter_exc * inter_exc + K0 * k;
            gp = (F)0.;
        }

        delta = (FLOAT)sqrt(delta);
        x1 = (delta - inter_exc) * (F)0.25;
        x2 = - (delta + inter_exc) * (F)0.25;
        g = ((FLOAT)fabs(x1) < (FLOAT)fabs(x2)) ? x1 : x2;
        if(g >= (F)0.) {
            if(g > G_MAX) g = G_MAX;
        }
        else {
            if(g < (-G_MAX)) g = -G_MAX;
        }

        /* Update cur_exc with ACELP excitation */
        for(i=0; i<4; i++) {
            cur_exc[pos[i]] += g * sign[i];
        }

        if(flag_cod != FLAG_DEC) update_exc_err(gp, t0);
        else {
            if(g >= (F)0.) Update_PhDisp(gp,g);
            else Update_PhDisp(gp,-g);
        }
        cur_exc += L_SUBFR;
  } /* end of loop on subframes */
  
  return;
}

/*-----------------------------------------------------------*
*         Local procedures                                  *
*         ~~~~~~~~~~~~~~~~                                  *
*-----------------------------------------------------------*/

/* Gaussian generation */
/***********************/
static FLOAT gauss(INT16 *seed)
{
        
    /****  Xi = uniform v.a. in [-32768, 32767]       ****/
    /****  Z = SUM(i=1->12) Xi / 2 x 32768 is N(0,1)  ****/
    /****  output : Z                                 ****/
        
    int i;
    INT16 temp;
    INT32 L_acc, L_temp;
    
    L_acc = 0L;
    for(i=0; i<12; i++) {
        L_temp = (INT32)random_g729c(seed);
        L_acc += L_temp;
    }
    L_acc >>= 7;
    temp = (INT16)L_acc;  /* Z x 512 */
    return((FLOAT)temp * (F)0.001953125);  /* Z */
}

