/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX D   Version 1.3    Last modified: Oct 2000 */

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "tab_ld8k.h"
#include "tabld8kd.h"

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
   Word16 *gain_cod,    /* (o) Q1  :Code gain.                     */
   Word16 past_qua_en[]
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
