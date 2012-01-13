/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : Q_AGAINCP.C
*/

/*****************************************************************************/
/* gain quantizer routines                                                   */
/*****************************************************************************/

#include <math.h>
#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

static FLOAT past_qua_en[4]={(F)-14.0,(F)-14.0,(F)-14.0,(F)-14.0};
/* prototypes of local functions */
static void   gbk_presel_6k(
FLOAT best_gain[],     /* input : [0] unquantized pitch gain
                                  [1] unquantized code gain      */
int *cand1,            /* output: index of best 1st stage vector */
int *cand2,            /* output: index of best 2nd stage vector */
FLOAT gcode0           /* input : presearch for gain codebook    */
);
static void   gbk_presel(
    FLOAT best_gain[],     /* input : [0] unquantized pitch gain
                                      [1] unquantized code gain      */
    int *cand1,            /* output: index of best 1st stage vector */
    int *cand2,            /* output: index of best 2nd stage vector */
    FLOAT gcode0           /* input : presearch for gain codebook    */
);


/*----------------------------------------------------------------------------
* qua_gain - Quantization of pitch and codebook gains
*----------------------------------------------------------------------------
*/
int qua_gain_6k(        /* output: quantizer index                   */
    FLOAT code[],         /* input : fixed codebook vector             */
    FLOAT *g_coeff,       /* input : correlation factors               */
    int l_subfr,          /* input : fcb vector length                 */
    FLOAT *gain_pit,      /* output: quantized acb gain                */
    FLOAT *gain_code,     /* output: quantized fcb gain                */
    int tameflag          /* input : flag set to 1 if taming is needed */
)
{
/*
* MA prediction is performed on the innovation energy (in dB with mean      *
* removed).                                                                 *
* An initial predicted gain, g_0, is first determined and the correction    *
* factor     alpha = gain / g_0    is quantized.                            *
* The pitch gain and the correction factor are vector quantized and the     *
* mean-squared weighted error criterion is used in the quantizer search.    *
*   CS Codebook , fast pre-selection version                                *
*/
    int    i,j, index1, index2;
    int    cand1,cand2 ;
    FLOAT  gcode0 ;
    FLOAT  dist, dist_min, g_pitch, g_code;
    FLOAT  best_gain[2],tmp;

    /*---------------------------------------------------*
    *-  energy due to innovation                       -*
    *-  predicted energy                               -*
    *-  predicted codebook gain => gcode0[exp_gcode0]  -*
    *---------------------------------------------------*/
    gain_predict( past_qua_en, code, l_subfr, &gcode0);

    /*-- pre-selection --*/
    tmp = (F)-1./((F)4.*g_coeff[0]*g_coeff[2]-g_coeff[4]*g_coeff[4]) ;
    best_gain[0] = ((F)2.*g_coeff[2]*g_coeff[1]-g_coeff[3]*g_coeff[4])*tmp ;
    best_gain[1] = ((F)2.*g_coeff[0]*g_coeff[3]-g_coeff[1]*g_coeff[4])*tmp ;

    if (tameflag == 1){
        if(best_gain[0]> GPCLIP2) best_gain[0] = GPCLIP2;
    }
    /*----------------------------------------------*
    *   - presearch for gain codebook -            *
    *----------------------------------------------*/
    gbk_presel_6k(best_gain,&cand1,&cand2,gcode0) ;

    /*-- selection --*/
    dist_min = FLT_MAX_G729;
    index1 = index2  = 0;  /* to avoid visual warning */
    if(tameflag == 1) {
        for (i=0;i<NCAN1_6K;i++){
            for(j=0;j<NCAN2_6K;j++){
                g_pitch=gbk1_6k[cand1+i][0]+gbk2_6k[cand2+j][0];
                if(g_pitch < GP0999) {
                    g_code=gcode0*(gbk1_6k[cand1+i][1]+gbk2_6k[cand2+j][1]);
                    dist = g_pitch*g_pitch * g_coeff[0]
                        + g_pitch         * g_coeff[1]
                        + g_code*g_code   * g_coeff[2]
                        + g_code          * g_coeff[3]
                        + g_pitch*g_code  * g_coeff[4] ;
                    if (dist < dist_min){
                        dist_min = dist;
                        index1 = cand1+i ;
                        index2 = cand2+j ;
                    }
                }
            }
        }
    }
    else {
        for (i=0;i<NCAN1_6K;i++){
            for(j=0;j<NCAN2_6K;j++){
                g_pitch=gbk1_6k[cand1+i][0]+gbk2_6k[cand2+j][0];
                g_code=gcode0*(gbk1_6k[cand1+i][1]+gbk2_6k[cand2+j][1]);
                dist = g_pitch*g_pitch * g_coeff[0]
                    + g_pitch         * g_coeff[1]
                    + g_code*g_code   * g_coeff[2]
                    + g_code          * g_coeff[3]
                    + g_pitch*g_code  * g_coeff[4] ;
                if (dist < dist_min){
                    dist_min = dist;
                    index1 = cand1+i ;
                    index2 = cand2+j ;
                }
            }
        }
    }
    *gain_pit  = gbk1_6k[index1][0]+gbk2_6k[index2][0] ;
    g_code = gbk1_6k[index1][1]+gbk2_6k[index2][1];
    *gain_code =  g_code * gcode0;
    /*----------------------------------------------*
    * update table of past quantized energies      *
    *----------------------------------------------*/
    if(g_code < (F)0.2) g_code = (F)0.2;
    gain_update( past_qua_en, g_code);
    
    return (map1_6k[index1]*NCODE2_6K+map2_6k[index2]);
}

/*----------------------------------------------------------------------------
* gbk_presel_6k - presearch for gain codebook
*/
static void   gbk_presel_6k(
    FLOAT best_gain[],     /* input : [0] unquantized pitch gain
                                        [1] unquantized code gain      */
    int *cand1,            /* output: index of best 1st stage vector */
    int *cand2,            /* output: index of best 2nd stage vector */
    FLOAT gcode0           /* input : presearch for gain codebook    */
)
{
    FLOAT    x,y ;
    
    x = (best_gain[1]-(coef_6k[0][0]*best_gain[0]+coef_6k[1][1])*gcode0) * INV_COEF_6K ;
    y = (coef_6k[1][0]*(-coef_6k[0][1]+best_gain[0]*coef_6k[0][0])*gcode0
        -coef_6k[0][0]*best_gain[1]) * INV_COEF_6K ;
    
    if(gcode0>(F)0.0){
        /* pre select codebook #1 */
        *cand1 = 0 ;
        do{
            if(y>thr1_6k[*cand1]*gcode0) (*cand1)++ ;
            else               break ;
        } while((*cand1)<(NCODE1_6K-NCAN1_6K)) ;
        /* pre select codebook #2 */
        *cand2 = 0 ;
        do{
            if(x>thr2_6k[*cand2]*gcode0) (*cand2)++ ;
            else               break ;
        } while((*cand2)<(NCODE2_6K-NCAN2_6K)) ;
    }
    else{
        /* pre select codebook #1 */
        *cand1 = 0 ;
        do{
            if(y<thr1_6k[*cand1]*gcode0) (*cand1)++ ;
            else               break ;
        } while((*cand1)<(NCODE1_6K-NCAN1_6K)) ;
        /* pre select codebook #2 */
        *cand2 = 0 ;
        do{
            if(x<thr2_6k[*cand2]*gcode0) (*cand2)++ ;
            else               break ;
        } while((*cand2)<(NCODE2_6K-NCAN2_6K)) ;
    }
    
    return ;
}
/*----------------------------------------------------------------------------
* qua_gain - Quantization of pitch and codebook gains
*----------------------------------------------------------------------------
*/
int qua_gain(           /* output: quantizer index                   */
    FLOAT code[],         /* input : fixed codebook vector             */
    FLOAT *g_coeff,       /* input : correlation factors               */
    int l_subfr,          /* input : fcb vector length                 */
    FLOAT *gain_pit,      /* output: quantized acb gain                */
    FLOAT *gain_code,     /* output: quantized fcb gain                */
    int tameflag          /* input : flag set to 1 if taming is needed */
)
{
/*
* MA prediction is performed on the innovation energy (in dB with mean      *
* removed).                                                                 *
* An initial predicted gain, g_0, is first determined and the correction    *
* factor     alpha = gain / g_0    is quantized.                            *
* The pitch gain and the correction factor are vector quantized and the     *
* mean-squared weighted error criterion is used in the quantizer search.    *
*   CS Codebook , fast pre-selection version                                *
        */

    int    i,j, index1, index2;
    int    cand1,cand2 ;
    FLOAT  gcode0 ;
    FLOAT  dist, dist_min, g_pitch, g_code;
    FLOAT  best_gain[2],tmp;
    
    /*---------------------------------------------------*
    *-  energy due to innovation                       -*
    *-  predicted energy                               -*
    *-  predicted codebook gain => gcode0[exp_gcode0]  -*
    *---------------------------------------------------*/
    gain_predict( past_qua_en, code, l_subfr, &gcode0);
    
    /*-- pre-selection --*/
    tmp = (F)-1./((F)4.*g_coeff[0]*g_coeff[2]-g_coeff[4]*g_coeff[4]) ;
    best_gain[0] = ((F)2.*g_coeff[2]*g_coeff[1]-g_coeff[3]*g_coeff[4])*tmp ;
    best_gain[1] = ((F)2.*g_coeff[0]*g_coeff[3]-g_coeff[1]*g_coeff[4])*tmp ;
    
    if (tameflag == 1){
        if(best_gain[0]> GPCLIP2) best_gain[0] = GPCLIP2;
    }
    /*----------------------------------------------*
    *   - presearch for gain codebook -            *
    *----------------------------------------------*/
    
    gbk_presel(best_gain,&cand1,&cand2,gcode0) ;
    
    /*-- selection --*/
    dist_min = FLT_MAX_G729;
    index1 = index2  = 0;  /* to avoid visual warning */
    if(tameflag == 1) {
        for (i=0;i<NCAN1;i++){
            for(j=0;j<NCAN2;j++){
                g_pitch=gbk1[cand1+i][0]+gbk2[cand2+j][0];
                if(g_pitch < GP0999) {
                    g_code=gcode0*(gbk1[cand1+i][1]+gbk2[cand2+j][1]);
                    dist = g_pitch*g_pitch * g_coeff[0]
                        + g_pitch         * g_coeff[1]
                        + g_code*g_code   * g_coeff[2]
                        + g_code          * g_coeff[3]
                        + g_pitch*g_code  * g_coeff[4] ;
                    if (dist < dist_min){
                        dist_min = dist;
                        index1 = cand1+i ;
                        index2 = cand2+j ;
                    }
                }
            }
        }
    }
    else {
        for (i=0;i<NCAN1;i++){
            for(j=0;j<NCAN2;j++){
                g_pitch=gbk1[cand1+i][0]+gbk2[cand2+j][0];
                g_code=gcode0*(gbk1[cand1+i][1]+gbk2[cand2+j][1]);
                dist = g_pitch*g_pitch * g_coeff[0]
                    + g_pitch         * g_coeff[1]
                    + g_code*g_code   * g_coeff[2]
                    + g_code          * g_coeff[3]
                    + g_pitch*g_code  * g_coeff[4] ;
                if (dist < dist_min){
                    dist_min = dist;
                    index1 = cand1+i ;
                    index2 = cand2+j ;
                }
            }
        }
    }
    *gain_pit  = gbk1[index1][0]+gbk2[index2][0] ;
    g_code = gbk1[index1][1]+gbk2[index2][1];
    *gain_code =  g_code * gcode0;
    /*----------------------------------------------*
    * update table of past quantized energies      *
    *----------------------------------------------*/
    gain_update( past_qua_en, g_code);
    
    return (map1[index1]*NCODE2+map2[index2]);
}

/*----------------------------------------------------------------------------
* gbk_presel - presearch for gain codebook
*/
static void   gbk_presel(
    FLOAT best_gain[],     /* input : [0] unquantized pitch gain
                                      [1] unquantized code gain      */
    int *cand1,            /* output: index of best 1st stage vector */
    int *cand2,            /* output: index of best 2nd stage vector */
    FLOAT gcode0           /* input : presearch for gain codebook    */
)
{
    FLOAT    x,y ;
    
    x = (best_gain[1]-(coef[0][0]*best_gain[0]+coef[1][1])*gcode0) * INV_COEF ;
    y = (coef[1][0]*(-coef[0][1]+best_gain[0]*coef[0][0])*gcode0
        -coef[0][0]*best_gain[1]) * INV_COEF ;
    
    if(gcode0>(F)0.0){
        /* pre select codebook #1 */
        *cand1 = 0 ;
        do{
            if(y>thr1[*cand1]*gcode0) (*cand1)++ ;
            else               break ;
        } while((*cand1)<(NCODE1-NCAN1)) ;
        /* pre select codebook #2 */
        *cand2 = 0 ;
        do{
            if(x>thr2[*cand2]*gcode0) (*cand2)++ ;
            else               break ;
        } while((*cand2)<(NCODE2-NCAN2)) ;
    }
    else{
        /* pre select codebook #1 */
        *cand1 = 0 ;
        do{
            if(y<thr1[*cand1]*gcode0) (*cand1)++ ;
            else               break ;
        } while((*cand1)<(NCODE1-NCAN1)) ;
        /* pre select codebook #2 */
        *cand2 = 0 ;
        do{
            if(x<thr2[*cand2]*gcode0) (*cand2)++ ;
            else               break ;
        } while((*cand2)<(NCODE2-NCAN2)) ;
    }
    
    return ;
}
