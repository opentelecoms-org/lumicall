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
 File : GAINPRED.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/

#include <math.h>
#include "typedef.h"
#include "version.h"
#ifdef VER_G729A
 #include "ld8a.h"
 #include "tab_ld8a.h"
#else
 #include "ld8k.h"
 #include "tab_ld8k.h"
#endif

/*---------------------------------------------------------------------------*
 * Function  Gain_predict                                                    *
 * ~~~~~~~~~~~~~~~~~~~~~~                                                    *
 * MA prediction is performed on the innovation energy (in dB with mean      *
 * removed).                                                                 *
 *---------------------------------------------------------------------------*/
void gain_predict(
   FLOAT past_qua_en[], /* (i)     :Past quantized energies        */
   FLOAT code[],        /* (i)     :Innovative vector.             */
   int l_subfr,         /* (i)     :Subframe length.               */
   FLOAT *gcode0        /* (o)     :Predicted codebook gain        */
)
{
   FLOAT ener_code, pred_code;
   int i;

   pred_code = MEAN_ENER ;

   /* innovation energy */
   ener_code = (F)0.01;
   for(i=0; i<l_subfr; i++)
     ener_code += code[i] * code[i];
   ener_code = (F)10.0 * (FLOAT)log10(ener_code /(FLOAT)l_subfr);

   pred_code -= ener_code;

   /* predicted energy */
   for (i=0; i<4; i++) pred_code += pred[i]*past_qua_en[i];

   /* predicted codebook gain */
   *gcode0 = pred_code;
   *gcode0 = (FLOAT)pow((double)10.0,(double)(*gcode0/20.0));   /* predicted gain */

   return;
}


/*---------------------------------------------------------------------------*
 * Function  gain_update                                                     *
 * ~~~~~~~~~~~~~~~~~~~~~~                                                    *
 * update table of past quantized energies                                   *
 *---------------------------------------------------------------------------*/
void gain_update(
   FLOAT past_qua_en[],   /* input/output :Past quantized energies     */
   FLOAT g_code           /*  input: gbk1[indice1][1]+gbk2[indice2][1] */
)
{
   int i;

   /* update table of past quantized energies */
   for (i = 3; i > 0; i--)
     past_qua_en[i] = past_qua_en[i-1];
   past_qua_en[0] = (F)20.0*(FLOAT)log10((double)g_code);

   return;
}

/*---------------------------------------------------------------------------*
 * Function  gain_update_erasure                                             *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                             *
 * update table of past quantized energies (frame erasure)                   *
 *---------------------------------------------------------------------------*
 *     av_pred_en = 0.0;                                                     *
 *     for (i = 0; i < 4; i++)                                               *
 *        av_pred_en += past_qua_en[i];                                      *
 *     av_pred_en = av_pred_en*0.25 - 4.0;                                   *
 *     if (av_pred_en < -14.0) av_pred_en = -14.0;                           *
 *---------------------------------------------------------------------------*/
void gain_update_erasure(
   FLOAT past_qua_en[]     /* input/output:Past quantized energies        */
)
{
   int i;
   FLOAT  av_pred_en;

    av_pred_en = (F)0.0;
    for (i = 0; i < 4; i++)
        av_pred_en += past_qua_en[i];
    av_pred_en = av_pred_en*(F)0.25 - (F)4.0;
    if (av_pred_en < (F)-14.0) av_pred_en = (F)-14.0;

    for (i = 3; i > 0; i--)
        past_qua_en[i] = past_qua_en[i-1];
    past_qua_en[0] = av_pred_en;

    return;
}

