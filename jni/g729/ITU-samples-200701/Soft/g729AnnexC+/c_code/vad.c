/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : VAD.C
*/

/* Voice Activity Detector functions */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"

/* Local functions */
static int MakeDec(
    FLOAT dSLE,    /* (i)  : differential low band energy */
    FLOAT dSE,     /* (i)  : differential full band energy */
    FLOAT SD,      /* (i)  : differential spectral distortion  */
    FLOAT dSZC     /* (i)  : differential zero crossing rate */
);

/* static variables */
static FLOAT MeanLSF[M];
static FLOAT Min_buffer[16];
static FLOAT Prev_Min, Next_Min, Min;
static FLOAT MeanE, MeanSE, MeanSLE, MeanSZC;
static FLOAT prev_energy;
static int count_sil, count_update, count_ext;
static int flag, v_flag, less_count;

/*---------------------------------------------------------------------------*
 * Function  vad_init                                                                                                            *
 * ~~~~~~~~~~~~~~~~~~                                                                                                            *
 *                                                                                                                                                       *
 * -> Initialization of variables for voice activity detection                           *
 *                                                                                                                                                       *
*---------------------------------------------------------------------------*/
void vad_init(void)
{
    /* Static vectors to zero */
    set_zero(MeanLSF, M);
    
    /* Initialize VAD parameters */
    MeanSE = (F)0.0;
    MeanSLE = (F)0.0;
    MeanE = (F)0.0;
    MeanSZC = (F)0.0;
    count_sil = 0;
    count_update = 0;
    count_ext = 0;
    less_count = 0;
    flag = 1;
    Min = FLT_MAX_G729;
    return;
}

/*-----------------------------------------------------------------*
* Functions vad                                                   *
*           ~~~                                                   *
* Input:                                                          *
*   rc            : reflection coefficient                        *
*   lsf[]         : unquantized lsf vector                        *
*   rxx[]         : autocorrelation vector                        *
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
    FLOAT  rc,
    FLOAT *lsf,
    FLOAT *rxx, 
    FLOAT *sigpp,
    int frm_count,
    int prev_marker,
    int pprev_marker,
    int *marker,
    FLOAT *Energy_db)
{
    FLOAT tmp[M];
    FLOAT SD;
    FLOAT E_low;
    FLOAT  dtemp;
    FLOAT  dSE;
    FLOAT  dSLE;
    FLOAT   ZC;
    FLOAT  COEF;
    FLOAT COEFZC;
    FLOAT COEFSD;
    FLOAT  dSZC;
    FLOAT norm_energy;
    int i;
    
    /* compute the frame energy */
    norm_energy = (F)10.0*(FLOAT) log10((FLOAT)( rxx[0]/(F)240.0 +EPSI));
    *Energy_db = norm_energy ;

    /* compute the low band energy */
    E_low = (F)0.0;
    for( i=1; i<= NP; i++)
        E_low = E_low + rxx[i]*lbf_corr[i];

    E_low= rxx[0]*lbf_corr[0] + (F)2.0*E_low;
    if (E_low < (F)0.0) E_low = (F)0.0;
    E_low= (F)10.0*(FLOAT) log10((FLOAT) (E_low/(F)240.0+EPSI));

    /* compute SD */
    /* Normalize lsfs */
    for(i=0; i<M; i++) lsf[i] /= (F)2.*PI;
    dvsub(lsf,MeanLSF,tmp,M);
    SD = dvdot(tmp,tmp,M);
    
    /* compute # zero crossing */
    ZC = (F)0.0f;
    dtemp = sigpp[ZC_START];
    for (i=ZC_START+1 ; i <= ZC_END ; i++) {
        if (dtemp*sigpp[i] < (F)0.0) {
            ZC= ZC +(F)1.0;
        }
        dtemp = sigpp[i];
    }
    ZC = ZC/(F)80.0;
    
    /* Initialize and update Mins */
    if( frm_count < 129 ) {
        if( norm_energy < Min ){
            Min = norm_energy;
            Prev_Min = norm_energy;
        }
        if( (frm_count % 8) == 0){
            Min_buffer[(int)frm_count/8 -1] = Min;
            Min = FLT_MAX_G729;
        }
    }
    if( (frm_count % 8) == 0){
        Prev_Min = Min_buffer[0];
        for ( i =1; i< 15; i++){
            if ( Min_buffer[i] <  Prev_Min )
                Prev_Min = Min_buffer[i];
        }
    }
    
    if( frm_count >= 129 ) {
        if( (frm_count % 8 ) == 1) {
            Min = Prev_Min;
            Next_Min = FLT_MAX_G729;
        }
        if( norm_energy < Min )
            Min = norm_energy;
        if( norm_energy < Next_Min )
            Next_Min = norm_energy;
        if( (frm_count % 8) == 0){
            for ( i =0; i< 15; i++)
                Min_buffer[i] = Min_buffer[i+1];
            Min_buffer[15]  = Next_Min;
            Prev_Min = Min_buffer[0];
            for ( i =1; i< 16; i++){
                if ( Min_buffer[i] <  Prev_Min )
                    Prev_Min = Min_buffer[i];
            }
            
        }
    }
    
    if (frm_count <= INIT_FRAME){
        if( norm_energy < (F)21.0){
            less_count++;
            *marker = NOISE;
        }
        else{
            *marker = VOICE;
            MeanE = (MeanE*( (FLOAT)(frm_count-less_count -1)) +
                norm_energy)/(FLOAT) (frm_count-less_count);
            MeanSZC = (MeanSZC*( (FLOAT)(frm_count-less_count -1)) +
                ZC)/(FLOAT) (frm_count-less_count);
            dvwadd(MeanLSF,(FLOAT) (frm_count-less_count -1),lsf,(F)1.0,MeanLSF,M);
            dvsmul(MeanLSF,(F)1.0/(FLOAT) (frm_count-less_count ),MeanLSF,M);
        }
    }

    if (frm_count >= INIT_FRAME ){
        if (frm_count == INIT_FRAME ){
            MeanSE = MeanE -(F)10.0;
            MeanSLE = MeanE -(F)12.0;
        }

        dSE = MeanSE - norm_energy;
        dSLE = MeanSLE - E_low;
        dSZC = MeanSZC - ZC;

        if( norm_energy < (F)21.0 ){
            *marker = NOISE;
        }
        else{
            *marker =MakeDec(dSLE, dSE, SD, dSZC );
        }
        
        v_flag =0;
        if( (prev_marker == VOICE) && (*marker == NOISE) &&
            (norm_energy > MeanSE + (F)2.0) && ( norm_energy>(F)21.0)){
            *marker = VOICE;
            v_flag=1;
        }
        
        if((flag == 1) ){
            if( (pprev_marker == VOICE) && (prev_marker == VOICE) &&
                (*marker == NOISE) && (fabs(prev_energy - norm_energy)<= (F)3.0)){
                count_ext++;
                *marker = VOICE;
                v_flag=1;
                if(count_ext <=4)
                    flag =1;
                else{
                    flag =0;
                    count_ext=0;
                }
            }
        }
        else
            flag =1;
        
        if(*marker == NOISE)
            count_sil++;
        
        if((*marker == VOICE) && (count_sil > 10) &&
            ((norm_energy - prev_energy) <= (F)3.0)){
            *marker = NOISE;
            count_sil=0;
        }
        
        
        if(*marker == VOICE)
            count_sil=0;
        
        if ((norm_energy < MeanSE+ (F)3.0) && ( frm_count >128) &&( !v_flag) && (rc <(F)0.6) ) *marker = NOISE;

        if ((norm_energy < MeanSE+ (F)3.0) && (rc <(F)0.75) && ( SD<(F)0.002532959)){
            count_update++;
            if (count_update < INIT_COUNT){
                COEF = (F)0.75;
                COEFZC = (F)0.8;
                COEFSD = (F)0.6;
            }
            else
                if (count_update < INIT_COUNT+10){
                    COEF = (F)0.95;
                    COEFZC = (F)0.92;
                    COEFSD = (F)0.65;
                }
                else
                    if (count_update < INIT_COUNT+20){
                        COEF = (F)0.97;
                        COEFZC = (F)0.94;
                        COEFSD = (F)0.70;
                    }
                    else
                        if (count_update < INIT_COUNT+30){
                            COEF = (F)0.99;
                            COEFZC = (F)0.96;
                            COEFSD = (F)0.75;
                        }
                        else
                            if (count_update < INIT_COUNT+40){
                                COEF = (F)0.995;
                                COEFZC = (F)0.99;
                                COEFSD = (F)0.75;
                            }
                            else{
                                COEF = (F)0.995;
                                COEFZC = (F)0.998;
                                COEFSD = (F)0.75;
                            }
            dvwadd(MeanLSF,COEFSD,lsf,(F)1.0 -COEFSD,MeanLSF,M);
            MeanSE = COEF*MeanSE+((F)1.0- COEF)*norm_energy;
            MeanSLE = COEF*MeanSLE+((F)1.0- COEF)*E_low;
            MeanSZC = COEFZC*MeanSZC+((F)1.0- COEFZC)*ZC;
        }
        
        if((frm_count >128) && ( (  MeanSE < Min ) && ( SD<(F)0.002532959)) || ( MeanSE > Min +(F)10.0 )){
            MeanSE = Min;
            count_update = 0;
        }
    }
  
    prev_energy = norm_energy;
    return;
}


static FLOAT a[14] = {
 (F)1.750000e-03, (F)-4.545455e-03, (F)-2.500000e+01,( F)2.000000e+01,
 (F)0.000000e+00, (F)8.800000e+03, (F)0.000000e+00, (F)2.5e+01,
 (F)-2.909091e+01, (F)0.000000e+00, (F)1.400000e+04, (F)0.928571,
 (F)-1.500000e+00, (F)0.714285};

static FLOAT b[14] = {
 (F)0.00085, (F)0.001159091, (F)-5.0, (F)-6.0, (F)-4.7, (F)-12.2, (F)0.0009,
 (F)-7.0, (F)-4.8182, (F)-5.3, (F)-15.5, (F)1.14285, (F)-9.0, (F)-2.1428571};

static int MakeDec(
    FLOAT dSLE,
    FLOAT dSE, 
    FLOAT SD, 
    FLOAT dSZC
)
{
        
    FLOAT pars[4];
    
    pars[0] = dSLE;
    pars[1] = dSE;
    pars[2] = SD;
    pars[3] = dSZC;
    
    /* SD vs dSZC */
    if (pars[2] > a[0]*pars[3]+b[0]) {
        return(VOICE);
    }
    if (pars[2] > a[1]*pars[3]+b[1]) {
        return(VOICE);
    }
        
    /*   dE vs dSZC */
    
    if (pars[1] < a[2]*pars[3]+b[2]) {
        return(VOICE);
    }
    if (pars[1] < a[3]*pars[3]+b[3]) {
        return(VOICE);
    }
    if (pars[1] < b[4]) {
        return(VOICE);
    }
        
    /*   dE vs SD */
    if (pars[1] < a[5]*pars[2]+b[5]) {
        return(VOICE);
    }
    if (pars[2] > b[6]) {
        return(VOICE);
    }
        
    /* dEL vs dSZC */
    if (pars[1] < a[7]*pars[3]+b[7]) {
        return(VOICE);
    }
    if (pars[1] < a[8]*pars[3]+b[8]) {
        return(VOICE);
    }
    if (pars[1] < b[9]) {
        return(VOICE);
    }
    
    /* dEL vs SD */
    if (pars[0] < a[10]*pars[2]+b[10]) {
        return(VOICE);
    }
    
    /* dEL vs dE */
    if (pars[0] > a[11]*pars[1]+b[11]) {
        return(VOICE);
    }
    
    if (pars[0] < a[12]*pars[1]+b[12]) {
        return(VOICE);
    }
    if (pars[0] < a[13]*pars[1]+b[13]) {
        return(VOICE);
    }
    
    return(NOISE);
}

