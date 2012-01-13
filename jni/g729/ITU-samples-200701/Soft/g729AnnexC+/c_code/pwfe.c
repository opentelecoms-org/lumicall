/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : PWFE.C
 */
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"

/************************************************************************/
/*                                                                      */
/*   ADAPTIVE BANDWIDTH EXPANSION FOR THE PERCEPTUAL WEIGHTING FILTER   */
/*                                                                      */
/*                 W(z) = A (z/gamma1) / A(z/gamma2)                    */
/*                                                                      */
/************************************************************************/

void perc_vare (
    FLOAT *gamma1,   /* Bandwidth expansion parameter */
    FLOAT *gamma2,   /* Bandwidth expansion parameter */
    int  high_stat /* high_stat indication (see file bwfw.c) */
)
{
    if (high_stat == 0) {
        gamma1[0] = (F)0.9;
        gamma1[1] = (F)0.9;
        gamma2[0] = (F)0.4;
        gamma2[1] = (F)0.4;
    }
    else {
        gamma1[0] = (F)0.98;
        gamma1[1] = (F)0.98;
        gamma2[0] = (F)0.4;
        gamma2[1] = (F)0.4;
    }
    return;
}

