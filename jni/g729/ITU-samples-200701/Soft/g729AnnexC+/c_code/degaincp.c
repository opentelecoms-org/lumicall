/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/


/*
 File : DEGAINCP.C
 */
#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

static FLOAT past_qua_en[4]={(F)-14.0,(F)-14.0,(F)-14.0,(F)-14.0};
/*----------------------------------------------------------------------------
* dec_gain - decode the adaptive and fixed codebook gains
*----------------------------------------------------------------------------
*/
void dec_gaine(
    int index,             /* input : quantizer index              */
    FLOAT code[],          /* input : fixed code book vector       */
    int l_subfr,           /* input : subframe size                */
    int bfi,               /* input : bad frame indicator good = 0 */
    FLOAT *gain_pit,       /* output: quantized acb gain           */
    FLOAT *gain_code,      /* output: quantized fcb gain           */
    int rate,              /* (i)   : number of bits per frame             */
    FLOAT gain_pit_mem,
    FLOAT gain_cod_mem,
    FLOAT  *c_muting,
    int count_bfi,
    int stationnary
)
{

    
    int    index1,index2;
    FLOAT  gcode0, g_code;
    
    /*----------------- Test erasure ---------------*/
    if (bfi != 0)  {
        if(rate == G729E) {
            if (count_bfi < 2) {
                if (stationnary) *gain_pit = (F)1.;
                else *gain_pit = (F)0.95;
                *gain_code = gain_cod_mem;
            }
            else {
                *gain_pit  = gain_pit_mem * (*c_muting);
                *gain_code = gain_cod_mem * (*c_muting);
                if (stationnary) {
                    if (count_bfi > 10) *c_muting *= (F)0.995;
                }
                else *c_muting *= (F)0.98;
                
            }
        }
        else {
            *gain_pit *= (F)0.9;
            if(*gain_pit > (F)0.9) *gain_pit=(F)0.9;
            *gain_code *= (F)0.98;
        }
        
        /*----------------------------------------------*
        * update table of past quantized energies      *
        *                              (frame erasure) *
        *----------------------------------------------*/
        gain_update_erasure(past_qua_en);
        
        return;
    }
    
    /*-------------- Decode pitch gain ---------------*/
    index1 = imap1[index>>NCODE2_B] ;
    index2 = imap2[index & (NCODE2-1)] ;
    *gain_pit = gbk1[index1][0]+gbk2[index2][0] ;
    
    /*-------------- Decode codebook gain ---------------*/
    /*---------------------------------------------------*
    *-  energy due to innovation                       -*
    *-  predicted energy                               -*
    *-  predicted codebook gain => gcode0[exp_gcode0]  -*
    *---------------------------------------------------*/
    gain_predict( past_qua_en, code, l_subfr, &gcode0);

    /*-----------------------------------------------------------------*
    * *gain_code = (gbk1[indice1][1]+gbk2[indice2][1]) * gcode0;      *
    *-----------------------------------------------------------------*/
    g_code = gbk1[index1][1]+gbk2[index2][1];
    *gain_code =  g_code * gcode0;
    
    /*----------------------------------------------*
    * update table of past quantized energies      *
    *----------------------------------------------*/
    gain_update( past_qua_en, g_code);
    
    return;
}
void dec_gain_6k(
    int index,             /* input : quantizer index              */
    FLOAT code[],          /* input : fixed code book vector       */
    int l_subfr,           /* input : subframe size                */
    int bfi,               /* input : bad frame indicator good = 0 */
    FLOAT *gain_pit,       /* output: quantized acb gain           */
    FLOAT *gain_code       /* output: quantized fcb gain           */
)
{
    int    index1,index2;
    FLOAT  gcode0, g_code;
    
    /*----------------- Test erasure ---------------*/
    if (bfi != 0)
    {
        *gain_pit *= (F)0.9;
        if(*gain_pit > (F)0.9) *gain_pit=(F)0.9;
        *gain_code *= (F)0.98;
        
        /*----------------------------------------------*
        * update table of past quantized energies      *
        *                              (frame erasure) *
        *----------------------------------------------*/
        gain_update_erasure(past_qua_en);
        return;
    }
    
    /*-------------- Decode pitch gain ---------------*/
    index1 = imap1_6k[index>>NCODE2_B_6K] ;
    index2 = imap2_6k[index & (NCODE2_6K-1)] ;
    *gain_pit = gbk1_6k[index1][0]+gbk2_6k[index2][0] ;
    
    /*-------------- Decode codebook gain ---------------*/
    /*---------------------------------------------------*
    *-  energy due to innovation                       -*
    *-  predicted energy                               -*
    *-  predicted codebook gain => gcode0[exp_gcode0]  -*
    *---------------------------------------------------*/
    gain_predict( past_qua_en, code, l_subfr, &gcode0);
    
    /*-----------------------------------------------------------------*
    * *gain_code = (gbk1_6k[indice1][1]+gbk2_6k[indice2][1]) * gcode0;      *
    *-----------------------------------------------------------------*/
    g_code = gbk1_6k[index1][1]+gbk2_6k[index2][1];
    *gain_code =  g_code * gcode0;
    
    /*----------------------------------------------*
    * update table of past quantized energies      *
    *----------------------------------------------*/
    if(g_code < (F)0.2) g_code = (F)0.2;
    gain_update( past_qua_en, g_code);
    
    return;
}
