/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : PHDISP.C
 */
#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

/*-----------------------------------------------------------*
* Static memory allocation.                                 *
*-----------------------------------------------------------*/

static int prevDispState = 0;
static FLOAT gainMem[6]={(F)0.0, (F)0.0, (F)0.0, (F)0.0, (F)0.0, (F)0.0};
static FLOAT prevCbGain = (F)0.0;
static int onset = 0;

/*-----------------------------------------------------------*
* Update_PhDisp- Updates state machine for phase dispersion     *
* in 6.4 kbps mode, when running in others modes.          *
*-----------------------------------------------------------*/
void Update_PhDisp(
    FLOAT ltpGain,   /* (i)  : pitch gain                  */
    FLOAT cbGain     /* (i)  : codebook gain               */
)
{
    int i;

    for (i = 5; i > 0; i--) gainMem[i] = gainMem[i-1];
    gainMem[0] = ltpGain;
    prevDispState = 2;
    prevCbGain = cbGain;
    onset = 0;

    return;
}

/*-----------------------------------------------------------*
* PhDisp - phase dispersion  in 6.4 kbps mode               *
*-----------------------------------------------------------*/
void PhDisp(
    FLOAT x[],       /* input : excitation signal                */
    FLOAT x_phdisp[],/* output : excitation signal after phase dispersion */
    FLOAT cbGain,
    FLOAT ltpGainQ,
    FLOAT inno[]
)
{
    int  i;

    FLOAT ScaledLtpEx[L_SUBFR];
    FLOAT inno_sav[L_SUBFR];
    int ps_poss[L_SUBFR];
    int nze, nPulse, i1, i2, ppos;
    int dispState;

    /* anti-sparseness post-processing */
    for (i = 0; i < L_SUBFR;  i++) {
        ScaledLtpEx[i] = x[i] - cbGain*inno[i];
        inno_sav[i] = inno[i];
        inno[i] = (F)0.0;
    }

    nze=0;
    for (i=0; i<L_SUBFR; i++) {
        if (inno_sav[i])
            ps_poss[nze++] = i;
    }
    if (ltpGainQ <= (F)0.6) {
        dispState = 0;
    }
    else if ( (ltpGainQ > (F)0.6)&&(ltpGainQ < (F)0.9) ) {
        dispState = 1;
    }
    else {
        dispState = 2;
    }

    for (i = 5; i > 0; i--) {
        gainMem[i]=gainMem[i-1];
    }
    gainMem[0] = ltpGainQ;
    
    if (cbGain > (F)2.0 * prevCbGain)
        onset = 2;
    else {
        if (onset) onset -= 1;
    }

    i1=0;
    for (i = 0; i < 6; i++) {
        if (gainMem[i] < (F)0.6) i1 += 1;
    }
    if (i1 > 2 && !onset) dispState = 0;

    if (dispState - prevDispState > 1 && !onset) dispState -= 1;

    if (onset) {
        if (dispState < 2) dispState++;
    }

    prevDispState=dispState;
    prevCbGain = cbGain;

    if (dispState == 0) {
        for (nPulse=0; nPulse<nze; nPulse++) {
            ppos = ps_poss[nPulse];
            for (i1=ppos; i1<L_SUBFR; i1++)
                inno[i1] += inno_sav[ppos] * ph_imp_low[i1-ppos];
            for (i2=0; i2 < ppos; i2++)
                inno[i2] += inno_sav[ppos] * ph_imp_low[L_SUBFR-ppos+i2];
        }
    }

    if (dispState == 1) {
        for (nPulse=0; nPulse<nze; nPulse++) {
            ppos = ps_poss[nPulse];
            for (i1=ppos; i1<L_SUBFR; i1++)
                inno[i1] += inno_sav[ppos] * ph_imp_mid[i1-ppos];
            for (i2=0; i2 < ppos; i2++)
                inno[i2] += inno_sav[ppos] * ph_imp_mid[L_SUBFR-ppos+i2];
        }
    }

    if (dispState == 2) {
        for (nPulse=0; nPulse<nze; nPulse++) {
            ppos = ps_poss[nPulse];
            for (i1=ppos; i1<L_SUBFR; i1++)
                inno[i1] += inno_sav[ppos] * ph_imp_high[i1-ppos];
            for (i2=0; i2 < ppos; i2++)
                inno[i2] += inno_sav[ppos] * ph_imp_high[L_SUBFR-ppos+i2];
        }
    }

    for (i = 0; i < L_SUBFR;  i++) {
        x_phdisp[i] = ScaledLtpEx[i] + cbGain*inno[i];
    }
    return;
}

