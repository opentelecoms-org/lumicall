/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
    ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
                         Version 1.1 of October 1999
*/
/*
 File : MUS_DTCT.C
*/

/* ----------------------------------------------------------------- */
/*                        MUSIC DETECTION MODULE                     */
/*                                                                   */
/*                                                                   */
/*                 (C) Copyright 1999 : Conexant Systems             */
/*                                                                   */
/* ----------------------------------------------------------------- */



#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "vad.h"

void musdetect(
               Word16 rate,
               Word16 r_h,
               Word16 r_l,
               Word16 exp_R0,
               Word16 *rc,
               Word16 *lags,
               Word16 *pgains,
               Word16 stat_flg,
               Word16 frm_count,
               Word16 prev_vad,
               Word16 *Vad,
               Word16 LLenergy)
{

    Word16 i,j;
    static Word16 count_music=0;
    static Word16 Mcount_music=0;
    static Word16 count_consc=0;
    Word16 std;
    static Word16 MeanPgain =8192;
    Word16 PFLAG1, PFLAG2, PFLAG;
    
    static Word16 count_pflag=0;
    static Word16 Mcount_pflag=0;
    static Word16 count_consc_pflag=0;
    static Word16 count_consc_rflag=0;
    static Word16 mrc[10]={0,0, 0, 0, 0, 0, 0, 0,0,0};
    static Word16 MeanSE =0;
    Word16 pderr, Lenergy , SD;
    Word16 Thres, Coeff, C_Coeff;
    Word32 acc0;
    Word16 exp, frac, lagsum;
    
    pderr =32767;
    for (i=0; i<4; i++){
        j = mult(rc[i], rc[i]);
        j = sub(32767, j);
        pderr = mult(pderr, j);
    }
    

    /* compute the frame energy */
    acc0 = Mpy_32_16(r_h, r_l, pderr);
    Log2(acc0, &exp, &frac);
    acc0 = Mpy_32_16(exp, frac, 9864);
    i = sub(exp_R0, 1);
    i = sub(i, 1);
    acc0 = L_mac(acc0, 9864, i);
    acc0 = L_shl(acc0, 11);
    Lenergy = extract_h(acc0);
    Lenergy = sub(Lenergy, 4875);


    acc0 = 0L;
    for (i=0; i<10; i++){
      j = sub(mrc[i], rc[i]);
      acc0 = L_mac(acc0, j, j);
    }
    SD = extract_h(acc0);
    
    if( *Vad == NOISE ){
        for (i=0; i<10; i++){
          acc0 = L_mult(29491, mrc[i]);
          acc0 = L_mac(acc0, 3277, rc[i]);
          mrc[i] = extract_h(acc0);
        } 
        acc0 = L_mult(29491, MeanSE);
        acc0 = L_mac(acc0, 3277, Lenergy); 
        MeanSE = extract_h(acc0);
    }

    /* determine the PFLAG */
     acc0 = 0L;
     lagsum = 0;
     for (i=0; i<5; i++){
       acc0 = L_mac(acc0, pgains[i], 6554); /* 1/5 in Q15 */
       lagsum = add(lagsum, lags[i]);
    }

    acc0 = L_mult(extract_h(acc0), 6554);
    acc0 = L_mac(acc0, MeanPgain, 26214);
    MeanPgain = extract_h(acc0);    /* compute the mean pitch gain */

    acc0 = 0L;
    for (i=0; i<5; i++){
      /* NOTE: the variance of the lag is scaled up by 25 */
      j = shl(lags[i], 2);
      j = add(j,lags[i]); /* j equals 5*lags[i] */
      j = sub(j, lagsum);
      acc0 = L_mac(acc0, j, j);
    }
    acc0 = L_shl(acc0, 22); 
    /* NOTE: the final variance of the lag is scaled up by 25x128 */
    std = extract_h(acc0);

    if ( rate == G729D)
        Thres = 11960; /* 0.73 in Q14 */
    else
        Thres = 10322; /* 0.63 in Q14 */
    
    if ( sub(MeanPgain,Thres) > 0)
        PFLAG2 =1;
    else
        PFLAG2 =0;
    
    /* 21632 = 1.3*1.3*25*4*128*/ 
    if ( (sub(std, 21632) < 0)  && (sub(MeanPgain, 7373) > 0))
        PFLAG1 =1;
    else
        PFLAG1 =0;
    
    PFLAG= (Word16)( ((Word16)prev_vad & (Word16)(PFLAG1 | PFLAG2))| (Word16)(PFLAG2));
    
    
    if( (sub(rc[1], 14746) <= 0) && (rc[1] >= 0) && (sub(MeanPgain,8192) < 0))
        count_consc_rflag = add(count_consc_rflag,1); 
    else
        count_consc_rflag =0;
    
    if( (stat_flg == 1) && (*Vad == VOICE))
        count_music = add(count_music,256);  /* Q8 */
    
     if( (frm_count & 0x003f) == 0){
        if( frm_count == 64)
            Mcount_music = count_music;
        else{
            acc0 = L_mult(29491, Mcount_music);
            acc0 = L_mac(acc0, 3277, count_music); 
            Mcount_music = extract_h(acc0);
            }
    }
    
    if( count_music == 0)
        count_consc = add(count_consc,1); 
    else
        count_consc = 0;
    
    if(  ((sub(count_consc, 500)>0) || (sub(count_consc_rflag , 150)>0))) Mcount_music = 0;
    
    if( (frm_count & 0x003f) == 0){
        count_music = 0;
    }
    
    if( PFLAG== 1 )
        count_pflag = add(count_pflag,256); /* Q8 */ 
    
    if( (frm_count & 0x003f) == 0){
        if( frm_count == 64)
            Mcount_pflag = count_pflag;
        else{
            if( sub(count_pflag , 6400)> 0){
                Coeff = 32113;
                C_Coeff = 655;
            }
            else if( sub(count_pflag , 5120)> 0){
                Coeff = 31130;
                C_Coeff = 1638;
            }
            else {
                Coeff = 29491;
                C_Coeff = 3277;
            }
            acc0 = L_mult(Coeff, Mcount_pflag);
            acc0 = L_mac(acc0, C_Coeff, count_pflag); 
            Mcount_pflag = extract_h(acc0);
        }
    }
    
    if( count_pflag == 0)
        count_consc_pflag = add(count_consc_pflag,1); 
    else
        count_consc_pflag = 0;
    
    if(  ((sub(count_consc_pflag, 100)>0) || (sub(count_consc_rflag , 150)>0))) Mcount_pflag = 0;
    

    if( (frm_count & 0x003f) == 0)
        count_pflag = 0;
    
    if (rate == G729E){
        if( (sub(SD,4915) > 0) && (sub(Lenergy ,MeanSE)> 819) && (sub(LLenergy,10240) >0 ) )
            *Vad =VOICE;
        else if( ((sub(SD,12452) > 0) || (sub(Lenergy ,MeanSE)> 819)) && 
                  (sub(LLenergy,10240) >0 ) )
            *Vad =VOICE;
        else if( ( (sub(Mcount_pflag ,2560) >0) || (sub(Mcount_music ,280)>0) || (sub(frm_count,64) < 0))
            && (sub(LLenergy,1433) >0))
            *Vad =VOICE;
    }

    return;
}
