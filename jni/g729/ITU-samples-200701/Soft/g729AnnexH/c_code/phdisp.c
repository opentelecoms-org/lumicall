/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex H  - Reference C code for fixed point
                         implementation of G.729 Annex H
                         (integration of Annexes D and E)
                         Version 1.1 of October 1999
*/
/*
 File : phdisp.c
 */

/* extracted from filterd.c G729 Annex D Version 1.2  Last modified: May 1998 */

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "ld8h.h"
#include "tabld8kd.h"
/*--------------------------------------------------------*
 *         Static memory allocation.                      *
 *--------------------------------------------------------*/
static Word16 gainMem[6] = {0, 0, 0, 0, 0, 0};
static Word16 prevState = 0;
static Word16 prevCbGain = 0;
static Word16 onset = 0;
/*-----------------------------------------------------------*
* Update_PhDisp- Updates state machine for phase dispersion     *
* in 6.4 kbps mode, when running in others modes.          *
*-----------------------------------------------------------*/
void Update_PhDisp(
                   Word16 ltpGain,   /* (i) Q14 : pitch gain                  */
                   Word16 cbGain     /* (i) Q1  : codebook gain               */
                   )
{
    Word16 i;
    for (i = 5; i > 0; i--) gainMem[i] = gainMem[i-1];
    gainMem[0] = ltpGain;
    
    prevState = 2;
    prevCbGain = cbGain;
    onset = 0;
    
    return;
}

/*-----------------------------------------------------------*
* PhDisp - phase dispersion  in 6.4 kbps mode               *
*-----------------------------------------------------------*/
void PhDisp(
            Word16 x[],     /* (i)   Q0  : excitation signal                */
            Word16 x_phdisp[],/*(o)  Q0  : excitation signal after phase dispersion */
            Word16 cbGain,  /* (i)   Q1  : Codebook gain                    */
            Word16 ltpGain, /* (i)   Q14 : LTP gain                         */
            Word16 inno[]   /* (i)   Q13 : Innovation vector                */
            )
{
    Word16 i;
    Word16 tmp1;
    Word16 ScaledLtpEx[L_SUBFR];
    Word16 inno_sav[L_SUBFR];
    Word16 ps_poss[L_SUBFR];
    Word16 nze, iii, nPulse, i1, j1, i2, j2, ppos;
    Word16 dState, testState;
    
    /* phase dispersion: ph_imp_low[] and ph_imp_mid[] are given in
    tab_ld8c.c and All-Pass filter ph_imp_high[] is calculated here */
    Word16 *ph_imp;   /* Pointer to phase dispersion filter */
    Word16 ph_imp_high[L_SUBFR];
    ph_imp_high[0]=32767;
    for (i = 1; i < L_SUBFR;  i++) ph_imp_high[i] = 0;
    
    for (i = 0; i < L_SUBFR;  i++) {
        /* ScaledLtpEx[i] = x[i] - cbGain*inno[i] */
        tmp1 = round(L_shl(L_mult(cbGain, inno[i]), 1));
        ScaledLtpEx[i] = sub(x[i], tmp1);
        inno_sav[i] = inno[i];
        inno[i] = 0;
    }
    
    nze=0;
    for (iii=0; iii<L_SUBFR; iii++) {
        if (inno_sav[iii] != 0) ps_poss[nze++] = iii;
    }
    
    if (sub(ltpGain, 14745) < 0) {    /* if (ltpGain < 0.9) */
        if (sub(ltpGain, 9830) > 0) { /* if (ltpGain > 0.6 */
            dState = 1;
        }
        else {
            dState = 0;
        }
    }
    else {
        dState = 2;
    }
    
    for (i = 5; i > 0; i--) gainMem[i] = gainMem[i-1];
    gainMem[0] = ltpGain;
    
    if (sub(shr(cbGain, 1), prevCbGain) > 0)
        onset = 2;
    else {
        onset--;
        if (onset < 0) onset = 0;
    }
    
    i1 = 0;
    for (i = 0; i < 6; i++) {
        if (sub(gainMem[i], 9830) < 0) i1 += 1;
    }
    if (sub(i1, 2) > 0)
        if (!onset) dState = 0;
        
    testState = add(prevState, 1);
    if (sub(dState, testState) > 0)
        if (!onset) dState -= 1;
        
    if (onset)
        if (sub(dState, 2) < 0) dState++;
        
    prevState = dState;
    prevCbGain = cbGain;
        
    if (sub(dState, 2) < 0) {
        if (dState) {
            ph_imp = ph_imp_mid;
        }
        else {
            ph_imp = ph_imp_low;
        }
    }
    else {
        ph_imp = ph_imp_high;
    }
                
    for (nPulse=0; nPulse<nze; nPulse++) {
        ppos = ps_poss[nPulse];
        
        for (i1=ppos, j1=0; i1<L_SUBFR; i1++, j1++) {
            /* inno[i1] += inno_sav[ppos] * ph_imp[i1-ppos] */
            tmp1 = mult(inno_sav[ppos], ph_imp[j1]);
            inno[i1] = add(inno[i1], tmp1);
        }
        j2=sub(L_SUBFR, ppos);
        for (i2=0; i2 < ppos; i2++, j2++) {
            /* inno[i2] += inno_sav[ppos] * ph_imp[L_SUBFR-ppos+i2] */
            tmp1 = mult(inno_sav[ppos], ph_imp[L_SUBFR-ppos+i2]);
            inno[i2] = add(inno[i2], tmp1);
        }
    }
    for (i = 0; i < L_SUBFR;  i++) {
        /* newTotEx[i] = ScaledLtpEx[i] + cbGain*inno[i]; */
        tmp1 = round(L_shl(L_mult(cbGain, inno[i]), 1));
        x_phdisp[i] = add(ScaledLtpEx[i], tmp1);
    }
                
    return;
}
