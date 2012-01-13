/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/
                                   
/*
 File : QSIDGAIN.C
*/

/* Quantize SID gain                                      */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "typedef.h"
#include "ld8k.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"
#include "tab_dtx.h"

/* Local function */
static int quant_Energy(
    FLOAT ener,    /* (i)  : Energy                 */
    FLOAT *enerq   /* (o)  : quantized energy in dB */
);

/*-------------------------------------------------------------------*
* Function  Qua_Sidgain                                             *
*           ~~~~~~~~~~~                                             *
*-------------------------------------------------------------------*/
void qua_Sidgain(
    FLOAT *ener,     /* (i)   array of energies                   */
    int nb_ener,     /* (i)   number of energies or               */
    FLOAT *enerq,    /* (o)   decoded energies in dB              */
    int *idx       /* (o)   SID gain quantization index         */
)
{
    int i;
    FLOAT avr_ener;
    
    if(nb_ener == 0) {
        /* Quantize energy saved for frame erasure case                */
        avr_ener = (*ener) * fact[0];
    }
    
    else {
        
    /*
    * Compute weighted average of energies
    * avr_ener = fact[nb_ener] x SUM(i=0->nb_ener-1) ener[i]
    * with fact[nb_ener] =  fact_ener / nb_ener x L_FRAME x nbAcf
        */
        avr_ener = (F)0.;
        for(i=0; i<nb_ener; i++) {
            avr_ener += ener[i];
        }
        avr_ener *= fact[nb_ener];
    }
    
    *idx = quant_Energy(avr_ener, enerq);
    
    return;
}

/* Local function */
static int quant_Energy(
    FLOAT ener,    /* (i)  : Energy                 */
    FLOAT *enerq   /* (o)  : quantized energy in dB */
)
{
    FLOAT ener_dB;
    int index;
    
    if(ener <= MIN_ENER) {  /* MIN_ENER <=> -8dB */
        *enerq = (F)-12.;
        return(0);
    }
    
    ener_dB = (F)10. * (FLOAT)log10(ener);
    
    if(ener_dB <= (F)-8.) {
        *enerq = (F)-12.;
        return(0);
    }
    
    if(ener_dB >= (F)65.) {
        *enerq = (F)66.;
        return(31);
    }
    
    if(ener_dB <= (F)14.) {
        index = (int)((ener_dB + (F)10.) * 0.25);
        if (index < 1) index = 1;
        *enerq = (F)4. * (FLOAT)index - (F)8.;
        return(index);
    }
    
    index = (int)((ener_dB - (F)3.) * 0.5);
    if (index < 6) index = 6;
    *enerq = (F)2. * (FLOAT)index + (F)4.;
    return(index);
}

