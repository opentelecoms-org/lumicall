/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
    ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
                         Version 1.1 of October 1999
*/

/*
 File : degaincp.c
 */
/* from decgaine.c G.729 Annex E Version 1.2  Last modified: May 1998 */
/* from dec_g6k.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from dec_gain.c G.729 Version 3.3            */

#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

/* Gain predictor, Past quantized energies = -14.0 in Q10 */
static Word16 past_qua_en[4] = { -14336, -14336, -14336, -14336 };

/*---------------------------------------------------------------------------*
* Function  Dec_gain                                                        *
* ~~~~~~~~~~~~~~~~~~                                                        *
* Decode the pitch and codebook gains                                       *
*                                                                           *
*---------------------------------------------------------------------------*
* input arguments:                                                          *
*                                                                           *
*   index      :Quantization index                                          *
*   code[]     :Innovative code vector                                      *
*   L_subfr    :Subframe size                                               *
*   bfi        :Bad frame indicator                                         *
*                                                                           *
* output arguments:                                                         *
*                                                                           *
*   gain_pit   :Quantized pitch gain                                        *
*   gain_cod   :Quantized codebook gain                                     *
*                                                                           *
*---------------------------------------------------------------------------*/
void Dec_gaine(
               Word16 index,        /* (i)     :Index of quantization.         */
               Word16 code[],       /* (i) Q13 :Innovative vector.             */
               Word16 L_subfr,      /* (i)     :Subframe length.               */
               Word16 bfi,          /* (i)     :Bad frame indicator            */
               Word16 *gain_pit,    /* (o) Q14 :Pitch gain.                    */
               Word16 *gain_cod,     /* (o) Q1  :Code gain.                     */
               Word16 rate,        /* (i)   : number of bits per frame             */
               Word16 gain_pit_mem,
               Word16 gain_cod_mem,
               Word16  *c_muting,
               Word16  count_bfi,
               Word16  stationnary
               )
{
    Word16  index1, index2, tmp;
    Word16  gcode0, exp_gcode0;
    Word32  L_gbk12, L_acc, L_accb;
    
    /*-------------- Case of erasure. ---------------*/
    if(bfi != 0) {
        if(rate == G729E) {
            if (count_bfi < 2) {
                if (stationnary) *gain_pit = 16384;
                else *gain_pit = 15564;
                *gain_cod = gain_cod_mem;
            }
            else {
                *gain_pit = mult(gain_pit_mem, *c_muting);
                *gain_cod = mult(gain_cod_mem, *c_muting);
                if (stationnary) {
                    if (count_bfi > 10) *c_muting = mult( *c_muting, 32604);
                }
                else *c_muting = mult( *c_muting, 32112);
                
            }
        }
        else {
            *gain_pit = mult( gain_pit_mem, 29491 );      /* *0.9 in Q15 */
            if (sub( *gain_pit, 29491) > 0) *gain_pit = 29491;
            *gain_cod = mult( gain_cod_mem, 32111 );      /* *0.98 in Q15 */
        }
        /*----------------------------------------------*
        * update table of past quantized energies      *
        *                              (frame erasure) *
        *----------------------------------------------*/
        Gain_update_erasure(past_qua_en);
        
        return;
    }
    
    
    /*-------------- Decode pitch gain ---------------*/
    index1 = imap1[ shr(index,NCODE2_B) ] ;
    index2 = imap2[ index & (NCODE2-1) ] ;
    *gain_pit = add( gbk1[index1][0], gbk2[index2][0] );
    
    /*-------------- Decode codebook gain ---------------*/
    
    /*---------------------------------------------------*
    *-  energy due to innovation                       -*
    *-  predicted energy                               -*
    *-  predicted codebook gain => gcode0[exp_gcode0]  -*
    *---------------------------------------------------*/
    Gain_predict( past_qua_en, code, L_subfr, &gcode0, &exp_gcode0 );
    
    /*-----------------------------------------------------------------*
    * *gain_code = (gbk1[indice1][1]+gbk2[indice2][1]) * gcode0;      *
    *-----------------------------------------------------------------*/
    L_acc = L_deposit_l( gbk1[index1][1] );
    L_accb = L_deposit_l( gbk2[index2][1] );
    L_gbk12 = L_add( L_acc, L_accb );                       /* Q13 */
    tmp = extract_l( L_shr( L_gbk12,1 ) );                  /* Q12 */
    L_acc = L_mult(tmp, gcode0);             /* Q[exp_gcode0+12+1] */
    
    L_acc = L_shl(L_acc, add( negate(exp_gcode0),(-12-1+1+16) ));
    *gain_cod = extract_h( L_acc );                          /* Q1 */
    
    /*----------------------------------------------*
    * update table of past quantized energies      *
    *----------------------------------------------*/
    Gain_update( past_qua_en, L_gbk12 );
    
    return;
    
}
/*---------------------------------------------------------------------------*
* Function  Dec_gain                                                        *
* ~~~~~~~~~~~~~~~~~~                                                        *
* Decode the pitch and codebook gains                                       *
*                                                                           *
*---------------------------------------------------------------------------*
* input arguments:                                                          *
*                                                                           *
*   index      :Quantization index                                          *
*   code[]     :Innovative code vector                                      *
*   L_subfr    :Subframe size                                               *
*   bfi        :Bad frame indicator                                         *
*                                                                           *
* output arguments:                                                         *
*                                                                           *
*   gain_pit   :Quantized pitch gain                                        *
*   gain_cod   :Quantized codebook gain                                     *
*                                                                           *
*---------------------------------------------------------------------------*/
void Dec_gain_6k(
                 Word16 index,        /* (i)     :Index of quantization.         */
                 Word16 code[],       /* (i) Q13 :Innovative vector.             */
                 Word16 L_subfr,      /* (i)     :Subframe length.               */
                 Word16 bfi,          /* (i)     :Bad frame indicator            */
                 Word16 *gain_pit,    /* (o) Q14 :Pitch gain.                    */
                 Word16 *gain_cod     /* (o) Q1  :Code gain.                     */
                 )
{
    Word16  index1, index2, tmp;
    Word16  gcode0, exp_gcode0;
    Word32  L_gbk12, L_acc, L_accb;
    
    /*-------------- Case of erasure. ---------------*/
    if(bfi != 0){
        *gain_pit = mult( *gain_pit, 29491 );      /* *0.9 in Q15 */
        if (sub( *gain_pit, 29491) > 0) *gain_pit = 29491;
        *gain_cod = mult( *gain_cod, 32111 );      /* *0.98 in Q15 */
        
        /*----------------------------------------------*
        * update table of past quantized energies      *
        *                              (frame erasure) *
        *----------------------------------------------*/
        Gain_update_erasure(past_qua_en);
        
        return;
    }
    
    /*-------------- Decode pitch gain ---------------*/
    index1 = imap1_6k[ shr(index,NCODE2_B_6K) ] ;
    index2 = imap2_6k[ index & (NCODE2_6K-1) ] ;
    *gain_pit = add( gbk1_6k[index1][0], gbk2_6k[index2][0] );
    
    /*-------------- Decode codebook gain ---------------*/
    /*---------------------------------------------------*
    *-  energy due to innovation                       -*
    *-  predicted energy                               -*
    *-  predicted codebook gain => gcode0[exp_gcode0]  -*
    *---------------------------------------------------*/
    Gain_predict(past_qua_en, code, L_subfr, &gcode0, &exp_gcode0);
    
    /*-----------------------------------------------------------------*
    * *gain_code = (gbk1[indice1][1]+gbk2[indice2][1]) * gcode0;      *
    *-----------------------------------------------------------------*/
    L_acc = L_deposit_l( gbk1_6k[index1][1] );
    L_accb = L_deposit_l( gbk2_6k[index2][1] );
    L_gbk12 = L_add( L_acc, L_accb );                          /* Q14 */
    L_gbk12 = L_shr(L_gbk12, 1);                               /* Q13 */
    tmp = extract_l( L_gbk12 );                                /* Q13 */
    L_acc = L_mult(tmp, gcode0);                /* Q[exp_gcode0+13+1] */
    L_acc = L_shl(L_acc, add( negate(exp_gcode0),(-13-1+1+16) ));
    *gain_cod = extract_h( L_acc );                             /* Q1 */
    
    /*----------------------------------------------*
    * update table of past quantized energies      *
    *----------------------------------------------*/
    Gain_update( past_qua_en, L_gbk12 );
       
    return;
}

