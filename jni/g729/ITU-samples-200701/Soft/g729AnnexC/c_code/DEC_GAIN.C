/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.

----------------------------------------------------------------------
*/
/*
 File : DEC_GAIN.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/
#include "typedef.h"
#include "version.h"
#ifdef VER_G729A
 #include "ld8a.h"
 #include "tab_ld8a.h"
#else
 #include "ld8k.h"
 #include "tab_ld8k.h"
#endif

/*----------------------------------------------------------------------------
 * dec_gain - decode the adaptive and fixed codebook gains
 *----------------------------------------------------------------------------
 */
void dec_gain(
 int index,             /* input : quantizer index              */
 FLOAT code[],          /* input : fixed code book vector       */
 int l_subfr,           /* input : subframe size                */
 int bfi,               /* input : bad frame indicator good = 0 */
 FLOAT *gain_pit,       /* output: quantized acb gain           */
 FLOAT *gain_code       /* output: quantized fcb gain           */
)
{
   static FLOAT past_qua_en[4]={(F)-14.0,(F)-14.0,(F)-14.0,(F)-14.0};

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

   index1 = imap1[index/NCODE2] ;
   index2 = imap2[index%NCODE2] ;
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
