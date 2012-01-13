/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.2    Last modified: May 1998 */

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
  Word16 *gamma1,   /* Bandwidth expansion parameter */
  Word16 *gamma2,   /* Bandwidth expansion parameter */
  Word16  high_stat /* high_stat indication (see file bwfw.c) */
)
{
    if (high_stat == 0) {
        gamma1[0] = 29491;
        gamma1[1] = 29491;
        gamma2[0] = 13107;
        gamma2[1] = 13107;
    }
    else {
        gamma1[0] = 32112;
        gamma1[1] = 32112;
        gamma2[0] = 13107;
        gamma2[1] = 13107;
    }
    return;
}

