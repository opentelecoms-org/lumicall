/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.

   Appendix III ANSI-C Source Code version 1.0
   Copyright (c) 2005, Mindspeed Technologies Inc.
   All rights reserved.
*/

#include <stdio.h>
#include "typedef.h"
#include "ld8a.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "tab_ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"


/* local function */
static Word16 MakeDec(
               Word16 dSLE,    /* (i)  : differential low band energy */
               Word16 dSE,     /* (i)  : differential full band energy */
               Word16 SD,      /* (i)  : differential spectral distortion */
               Word16 dSZC     /* (i)  : differential zero crossing rate */
);

/* static variables */
static Word16 MeanLSF[M];
static Word16 Min_buffer[16];
static Word16 Prev_Min, Next_Min, Min;
static Word16 MeanE, MeanSE, MeanSLE, MeanSZC;
static Word16 prev_energy;
static Word16 count_sil, count_update, count_ext;
static Word16 flag, v_flag, less_count;


/*---------------------------------------------------------------------------*
 * Function  vad_init                                                        *
 * ~~~~~~~~~~~~~~~~~~                                                        *
 *                                                                           *
 * -> Initialization of variables for voice activity detection               *
 *                                                                           *
 *---------------------------------------------------------------------------*/
void vad_init(void)
{
  /* Static vectors to zero */
  Set_zero(MeanLSF, M);

  /* Initialize VAD parameters */
  MeanSE = 0;
  MeanSLE = 0;
  MeanE = 0;
  MeanSZC = 0;
  count_sil = 0;
#ifdef VAD_VOIP_APP_III  
  v_flag = 1;
#endif    
  count_update = 0;
  count_ext = 0;
  less_count = 0;
  flag = 1;
  Min = MAX_16;
}


/*-----------------------------------------------------------------*
 * Functions vad                                                   *
 *           ~~~                                                   *
 * Input:                                                          *
 *   rc            : reflection coefficient                        *
 *   lsf[]         : unquantized lsf vector                        *
 *   r_h[]         : upper 16-bits of the autocorrelation vector   *
 *   r_l[]         : lower 16-bits of the autocorrelation vector   *
 *   exp_R0        : exponent of the autocorrelation vector        *
 *   sigpp[]       : preprocessed input signal                     *
 *   frm_count     : frame counter                                 *
 *   prev_marker   : VAD decision of the last frame                *
 *   pprev_marker  : VAD decision of the frame before last frame   *
 *                                                                 *
 * Output:                                                         *
 *                                                                 *
 *   marker        : VAD decision of the current frame             * 
 *                                                                 *
 *-----------------------------------------------------------------*/
void vad(
         Word16 rc,
         Word16 *lsf, 
         Word16 *r_h,
         Word16 *r_l, 
         Word16 exp_R0,
         Word16 *sigpp,
         Word16 frm_count,
         Word16 prev_marker,
         Word16 pprev_marker,
         Word16 *marker)
{
 /* scalar */
  Word32 acc0;
  Word16 i, j, exp, frac;
  Word16 ENERGY, ENERGY_low, SD, ZC, dSE, dSLE, dSZC;
  Word16 COEF, C_COEF, COEFZC, C_COEFZC, COEFSD, C_COEFSD;

  /* compute the frame energy */
  acc0 = L_Comp(r_h[0], r_l[0]); 
  Log2(acc0, &exp, &frac);
  acc0 = Mpy_32_16(exp, frac, 9864);
  i = sub(exp_R0, 1);  
  i = sub(i, 1);
  acc0 = L_mac(acc0, 9864, i);
  acc0 = L_shl(acc0, 11);
  ENERGY = extract_h(acc0);
  ENERGY = sub(ENERGY, 4875);

  /* compute the low band energy */
  acc0 = 0;
  for (i=1; i<=NP; i++)
    acc0 = L_mac(acc0, r_h[i], lbf_corr[i]);
  acc0 = L_shl(acc0, 1);
  acc0 = L_mac(acc0, r_h[0], lbf_corr[0]);
  Log2(acc0, &exp, &frac);
  acc0 = Mpy_32_16(exp, frac, 9864);
  i = sub(exp_R0, 1);  
  i = sub(i, 1);
  acc0 = L_mac(acc0, 9864, i);
  acc0 = L_shl(acc0, 11);
  ENERGY_low = extract_h(acc0);
  ENERGY_low = sub(ENERGY_low, 4875);
  
  /* compute SD */
  acc0 = 0;
  for (i=0; i<M; i++){
    j = sub(lsf[i], MeanLSF[i]);
    acc0 = L_mac(acc0, j, j);
  }
  SD = extract_h(acc0);      /* Q15 */
  
  /* compute # zero crossing */
  ZC = 0;
  for (i=ZC_START+1; i<=ZC_END; i++)
    if (mult(sigpp[i-1], sigpp[i]) < 0)
      ZC = add(ZC, 410);     /* Q15 */

  /* Initialize and update Mins */
  if(sub(frm_count, 129) < 0){
    if (sub(ENERGY, Min) < 0){
      Min = ENERGY;
      Prev_Min = ENERGY;
    }
    
    if((frm_count & 0x0007) == 0){
      i = sub(shr(frm_count,3),1);
      Min_buffer[i] = Min;
      Min = MAX_16;
    }
  }

  if ((frm_count & 0x0007) == 0) {
    Prev_Min = Min_buffer[0];
    for (i=1; i<16; i++){
      if (sub(Min_buffer[i], Prev_Min) < 0)
        Prev_Min = Min_buffer[i];
    }
  }
  
  if(sub(frm_count, 129) >= 0){
    if(((frm_count & 0x0007) ^ (0x0001)) == 0){
      Min = Prev_Min;
      Next_Min = MAX_16;
    }
    if (sub(ENERGY, Min) < 0)
      Min = ENERGY;
    if (sub(ENERGY, Next_Min) < 0)
      Next_Min = ENERGY;
    
    if((frm_count & 0x0007) == 0){
      for (i=0; i<15; i++)
        Min_buffer[i] = Min_buffer[i+1]; 
      Min_buffer[15] = Next_Min; 
      Prev_Min = Min_buffer[0];
      for (i=1; i<16; i++) 
        if (sub(Min_buffer[i], Prev_Min) < 0)
          Prev_Min = Min_buffer[i];
     }
  }

  if (sub(frm_count, INIT_FRAME) <= 0)
    if(sub(ENERGY, 3072) < 0) {  
      *marker = NOISE;
      less_count++;
    }
    else{
      *marker = VOICE;   
      acc0 = L_deposit_h(MeanE);
      acc0 = L_mac(acc0, ENERGY, 1024);
      MeanE = extract_h(acc0);      
      acc0 = L_deposit_h(MeanSZC);
      acc0 = L_mac(acc0, ZC, 1024);
      MeanSZC = extract_h(acc0);
      for (i=0; i<M; i++){
        acc0 = L_deposit_h(MeanLSF[i]);
        acc0 = L_mac(acc0, lsf[i], 1024);
        MeanLSF[i] = extract_h(acc0);
      }
    }
  
  if (sub(frm_count, INIT_FRAME) >= 0) {
    if (sub(frm_count, INIT_FRAME) == 0) {
      acc0 = L_mult(MeanE, factor_fx[less_count]);
      acc0 = L_shl(acc0, shift_fx[less_count]);
      MeanE = extract_h(acc0);

      acc0 = L_mult(MeanSZC, factor_fx[less_count]);
      acc0 = L_shl(acc0, shift_fx[less_count]);
      MeanSZC = extract_h(acc0);

      for (i=0; i<M; i++){
        acc0 = L_mult(MeanLSF[i], factor_fx[less_count]);
        acc0 = L_shl(acc0, shift_fx[less_count]);
        MeanLSF[i] = extract_h(acc0);
      }

      MeanSE = sub(MeanE, 2048);   /* Q11 */
      MeanSLE = sub(MeanE, 2458);  /* Q11 */
    }

    dSE = sub(MeanSE, ENERGY);
    dSLE = sub(MeanSLE, ENERGY_low);
    dSZC = sub(MeanSZC, ZC);

    if(sub(ENERGY, 3072) < 0)
      *marker = NOISE;
    else 
      *marker = MakeDec(dSLE, dSE, SD, dSZC);

#ifdef VAD_VOIP_APP_III

    if (*marker==VOICE) count_ext=0;
    else count_ext++; 
    if (prev_marker == NOISE) flag=frm_count;
    if ( sub(frm_count, flag)>8 || add(dSE, 3000)<0 ) v_flag=1;
    if (count_ext>8) v_flag=0;     
    if (prev_marker == VOICE) {
        if (count_ext<=2) *marker = VOICE;  
        if ( (v_flag==1) && (ENERGY<3000 || count_ext<=5) ) *marker = VOICE;
	}
 
    dSLE = sub(ENERGY,prev_energy);    
    if ((SD<70) && (add(dSE, 1200)>0) ) {      
	if ( (sub(count_sil, 12) >= 0) && (sub(dSLE, 800)<0) )  *marker = NOISE;
	if ( (sub(count_sil, 6) >= 0) && (sub(dSLE, 400)<0) ) *marker = NOISE; 
	}    		
    if (count_ext>0) count_sil++;
    if (*marker==VOICE) count_sil=0;   
  
#else
     
    v_flag = 0;      	    
    if((prev_marker==VOICE) && (*marker==NOISE) && (add(dSE,410) < 0) 
       && (sub(ENERGY, 3072)>0)) {
      *marker = VOICE;
      v_flag = 1;
    }

    if(flag == 1){
      if((pprev_marker == VOICE) && (prev_marker == VOICE) && (*marker == NOISE)  
	  && (sub(abs_s(sub(prev_energy,ENERGY)), 614) <= 0) 
	 ) {
        count_ext++;
        *marker = VOICE;
        v_flag = 1;	
        if(sub(count_ext, 4) <= 0)
          flag=1;
        else{
          count_ext=0;
          flag=0;
        }
      }
    }
    else
      flag=1;

    if (*marker == NOISE) 
	count_sil++;

    if((*marker == VOICE) && (sub(count_sil, 10) >= 0) &&  
      (sub(sub(ENERGY,prev_energy), 614) <= 0)) {
      *marker = NOISE;  
      count_sil=0;  
     }    

    if (*marker == VOICE) 
	count_sil=0; 
#endif   

    if ((sub(sub(ENERGY, 614), MeanSE) < 0) && (sub(frm_count, 128) > 0) 
        && (!v_flag) && (sub(rc, 19661) < 0) 
#ifdef VAD_VOIP_APP_III
        && (prev_marker == NOISE) && (sub(dSLE, 614)<0) && (SD<60)
#endif 		
	)
      *marker = NOISE;

    if ( (sub(sub(ENERGY,614), MeanSE) < 0) && (sub(rc, 24576) < 0) 
#ifdef VAD_VOIP_APP_III    
          ) 
          {    
	  flag=frm_count;
#else    
	   && (sub(SD, 83) < 0)  ) 
          { 	   	  
#endif	 
      count_update++;
      if (sub(count_update, INIT_COUNT) < 0) {
        COEF = 24576;
        C_COEF = 8192;
        COEFZC = 26214;
        C_COEFZC = 6554;
        COEFSD = 19661;
        C_COEFSD = 13017;
      } 
      else
        if (sub(count_update, INIT_COUNT+10) < 0) {
          COEF = 31130;
          C_COEF = 1638;
          COEFZC = 30147;
          C_COEFZC = 2621;
          COEFSD = 21299;
          C_COEFSD = 11469;
        }
        else
          if (sub(count_update, INIT_COUNT+20) < 0){
            COEF = 31785;
            C_COEF = 983;
            COEFZC = 30802;
            C_COEFZC = 1966;
            COEFSD = 22938;
            C_COEFSD = 9830;
          }
          else
            if (sub(count_update, INIT_COUNT+30) < 0){
              COEF = 32440;
              C_COEF = 328;
              COEFZC = 31457;
              C_COEFZC = 1311;
              COEFSD = 24576;
              C_COEFSD = 8192;
            }
            else
              if (sub(count_update, INIT_COUNT+40) < 0){
                COEF = 32604;
                C_COEF = 164;
                COEFZC = 32440;
                C_COEFZC = 328;
                COEFSD = 24576;
                C_COEFSD = 8192;
              }
              else{
                COEF = 32604;
                C_COEF = 164;
                COEFZC = 32702;
                C_COEFZC = 66;
                COEFSD = 24576;
                C_COEFSD = 8192;
              }
      
      /* compute MeanSE */
      acc0 = L_mult(COEF, MeanSE);
      acc0 = L_mac(acc0, C_COEF, ENERGY);
      MeanSE = extract_h(acc0);

      /* compute MeanSLE */
      acc0 = L_mult(COEF, MeanSLE);
      acc0 = L_mac(acc0, C_COEF, ENERGY_low);
      MeanSLE = extract_h(acc0);

      /* compute MeanSZC */
      acc0 = L_mult(COEFZC, MeanSZC);
      acc0 = L_mac(acc0, C_COEFZC, ZC);
      MeanSZC = extract_h(acc0);
      
      /* compute MeanLSF */
      for (i=0; i<M; i++){
        acc0 = L_mult(COEFSD, MeanLSF[i]);
        acc0 = L_mac(acc0, C_COEFSD, lsf[i]);
        MeanLSF[i] = extract_h(acc0);
      }
    }


    if( (sub(frm_count, 128) > 0) && (	  
#ifdef VAD_VOIP_APP_III
	    (sub(MeanSE, Min) < 0) ||  
	    (sub(MeanSE, Min) > 1600) ||
	    (sub(frm_count, flag)>300) 
#else
	    ( (sub(MeanSE, Min) < 0) && (sub(SD, 83) < 0) ) ||  
	    (sub(MeanSE, Min) > 2048)  
#endif
	    ))
	    {  	    
      MeanSE = Min;
      count_update = 0;  
    }
    
  }
  
#ifdef VAD_VOIP_APP_III
  /* Tone detector */
  if (ENERGY>15500 || rc>30000) { *marker = VOICE; count_ext=0; }
#endif

  prev_energy = ENERGY;

}

/* local function */  
static Word16 MakeDec(
               Word16 dSLE,    /* (i)  : differential low band energy */
               Word16 dSE,     /* (i)  : differential full band energy */
               Word16 SD,      /* (i)  : differential spectral distortion */
               Word16 dSZC     /* (i)  : differential zero crossing rate */
               )
{
  Word32 acc0;

  /* SD vs dSZC */
  acc0 = L_mult(dSZC, -14680);          /* Q15*Q23*2 = Q39 */  
  acc0 = L_mac(acc0, 8192, -28521);     /* Q15*Q23*2 = Q39 */
  acc0 = L_shr(acc0, 8);                /* Q39 -> Q31 */
  acc0 = L_add(acc0, L_deposit_h(SD));
  if (acc0 > 0) return(VOICE);

  acc0 = L_mult(dSZC, 19065);           /* Q15*Q22*2 = Q38 */
  acc0 = L_mac(acc0, 8192, -19446);     /* Q15*Q22*2 = Q38 */
  acc0 = L_shr(acc0, 7);                /* Q38 -> Q31 */
  acc0 = L_add(acc0, L_deposit_h(SD));
  if (acc0 > 0) return(VOICE);

  /* dSE vs dSZC */
  acc0 = L_mult(dSZC, 20480);           /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 8192, 16384);      /* Q13*Q15*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return(VOICE);

  acc0 = L_mult(dSZC, -16384);          /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 8192, 19660);      /* Q13*Q15*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return(VOICE);

  acc0 = L_mult(dSE, 32767);            /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 1024, 30802);      /* Q10*Q16*2 = Q27 */
  if (acc0 < 0) return(VOICE);
  
  /* dSE vs SD */
  acc0 = L_mult(SD, -28160);            /* Q15*Q5*2 = Q22 */
  acc0 = L_mac(acc0, 64, 19988);        /* Q6*Q14*2 = Q22 */
  acc0 = L_mac(acc0, dSE, 512);         /* Q11*Q9*2 = Q22 */
  if (acc0 < 0) return(VOICE);

  acc0 = L_mult(SD, 32767);             /* Q15*Q15*2 = Q31 */
  acc0 = L_mac(acc0, 32, -30199);       /* Q5*Q25*2 = Q31 */
  if (acc0 > 0) return(VOICE);

  /* dSLE vs dSZC */
  acc0 = L_mult(dSZC, -20480);          /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 8192, 22938);      /* Q13*Q15*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return(VOICE);

  acc0 = L_mult(dSZC, 23831);           /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 4096, 31576);      /* Q12*Q16*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return(VOICE);
 
  acc0 = L_mult(dSE, 32767);            /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 2048, 17367);      /* Q11*Q15*2 = Q27 */
  if (acc0 < 0) return(VOICE);

  /* dSLE vs SD */
  acc0 = L_mult(SD, -22400);            /* Q15*Q4*2 = Q20 */
  acc0 = L_mac(acc0, 32, 25395);        /* Q5*Q14*2 = Q20 */
  acc0 = L_mac(acc0, dSLE, 256);        /* Q11*Q8*2 = Q20 */
  if (acc0 < 0) return(VOICE);
  
  /* dSLE vs dSE */
  acc0 = L_mult(dSE, -30427);           /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 256, -29959);      /* Q8*Q18*2 = Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSLE));
  if (acc0 > 0) return(VOICE);

  acc0 = L_mult(dSE, -23406);           /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 512, 28087);       /* Q19*Q17*2 = Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSLE)); 
  if (acc0 < 0) return(VOICE);
 
  acc0 = L_mult(dSE, 24576);            /* Q11*Q14*2 = Q26 */
  acc0 = L_mac(acc0, 1024, 29491);      /* Q10*Q15*2 = Q26 */
  acc0 = L_mac(acc0, dSLE, 16384);      /* Q11*Q14*2 = Q26 */
  if (acc0 < 0) return(VOICE);

  return (NOISE);
}


