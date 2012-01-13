/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/


/* ----------------------------------------------------------------- */
/*                        MUSIC DETECTION MODULE                     */
/*                                                                   */
/*                                                                   */
/*                 (C) Copyright 1999 : Conexant Systems             */
/*                                                                   */
/* ----------------------------------------------------------------- */

/*
 File : MUS_DTCT.C
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "vad.h"


#define         sqr(a)          ((a)*(a))

void musdetect(
               int rate,
               FLOAT Energy,
               FLOAT *rc,
               int *lags,
               FLOAT *pgains,
               int stat_flg,
               int frm_count,
               int prev_vad,
               int *Vad,
               FLOAT LLenergy)
{

    int i;
    static int count_music=0;
    static FLOAT Mcount_music=(F)0.0;
    static int count_consc=0;
    FLOAT sum1, sum2,std;
    static FLOAT MeanPgain =(F)0.5;
    short PFLAG1, PFLAG2, PFLAG;
    
    static int count_pflag=0;
    static FLOAT Mcount_pflag=(F)0.0;
    static int count_consc_pflag=0;
    static int count_consc_rflag=0;
    static FLOAT mrc[10]={(F)0.0,(F)0.0, (F)0.0, (F)0.0, (F)0.0,
        (F)0.0, (F)0.0, (F)0.0,(F)0.0,(F)0.0};
    static FLOAT MeanSE =(F)0.0;
    FLOAT pderr, Lenergy , SD, tmp_vec[10];
    FLOAT Thres;
    
    pderr =(F)1.0;
    for (i=0; i< 4; i++) pderr *= ((F)1.0 - rc[i]*rc[i]);
    dvsub(mrc,rc,tmp_vec,10);
    SD = dvdot(tmp_vec, tmp_vec,10);
    
    Lenergy = (F)10.0*(FLOAT)log10(pderr*Energy/(F)240.0 +EPSI);
    
    if( *Vad == NOISE ){
        dvwadd(mrc,(F)0.9,rc,(F)0.1,mrc,10);
        MeanSE = (F)0.9*MeanSE + (F)0.1*Lenergy;
    }
    
    sum1 =(F)0.0;
    sum2 =(F)0.0;
    for(i=0; i<5; i++){
        sum1 += (FLOAT) lags[i];
        sum2 +=  pgains[i];
    }
    
    sum1 = sum1/(F)5.0;
    sum2 = sum2/(F)5.0;
    std =(F)0.0;
    for(i=0; i<5; i++) std += sqr(((FLOAT) lags[i] - sum1));
    std = (FLOAT)sqrt(std/(F)4.0);
    
    MeanPgain = (F)0.8*MeanPgain + (F)0.2*sum2;
    
    if ( rate == G729D)
        Thres = (F)0.73;
    else
        Thres = (F)0.63;
    
    if ( MeanPgain > Thres)
        PFLAG2 =1;
    else
        PFLAG2 =0;
    
    if ( std < (F)1.30 && MeanPgain > (F)0.45 )
        PFLAG1 =1;
    else
        PFLAG1 =0;
    
    PFLAG= (INT16)( ((INT16)prev_vad & (INT16)(PFLAG1 | PFLAG2))| (INT16)(PFLAG2));
    
    if( rc[1] <= (F)0.45 && rc[1] >= (F)0.0 && MeanPgain < (F)0.5)
        count_consc_rflag++;
    else
        count_consc_rflag =0;
    
    if( stat_flg== 1 && (*Vad == VOICE))
        count_music++;
    
    if ((frm_count%64) == 0 ){
        if( frm_count == 64)
            Mcount_music = (FLOAT)count_music;
        else
            Mcount_music = (F)0.9*Mcount_music + (F)0.1*(FLOAT)count_music;
    }
    
    if( count_music == 0)
        count_consc++;
    else
        count_consc = 0;
    
    if( count_consc > 500 || count_consc_rflag > 150) Mcount_music = (F)0.0;
    
    if ((frm_count%64) == 0)
        count_music = 0;
    
    if( PFLAG== 1 )
        count_pflag++;
    
    if ((frm_count%64) == 0 ){
        if( frm_count == 64)
            Mcount_pflag = (FLOAT)count_pflag;
        else{
            if( count_pflag > 25)
                Mcount_pflag = (F)0.98*Mcount_pflag + (F)0.02*(FLOAT)count_pflag;
            else if( count_pflag > 20)
                Mcount_pflag = (F)0.95*Mcount_pflag + (F)0.05*(FLOAT)count_pflag;
            else
                Mcount_pflag = (F)0.90*Mcount_pflag + (F)0.10*(FLOAT)count_pflag;
        }
    }
    
    if( count_pflag == 0)
        count_consc_pflag++;
    else
        count_consc_pflag = 0;
    
    if( count_consc_pflag > 100 || count_consc_rflag > 150) Mcount_pflag = (F)0.0;
    
    if ((frm_count%64) == 0)
        count_pflag = 0;
    
    if (rate == G729E){
        if( SD > (F)0.15 && (Lenergy -MeanSE)> (F)4.0 && (LLenergy> 50.0) )
            *Vad =VOICE;
        else if( (SD > (F)0.38 || (Lenergy -MeanSE)> (F)4.0  ) && (LLenergy> 50.0))
            *Vad =VOICE;
        else if( (Mcount_pflag >= (F)10.0 || Mcount_music >= (F)5.0 || frm_count < 64)
            && (LLenergy> 7.0))
            *Vad =VOICE;
    }
    return;
}
