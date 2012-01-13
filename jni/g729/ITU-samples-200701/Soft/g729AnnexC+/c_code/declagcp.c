/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/
/*
 File : DECLAGCP.C
*/
#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"

/*------------------------------------------------------------------------*
*    Function dec_lag3cp                                                 *
*             ~~~~~~~~                                                   *
*   Decoding of fractional pitch lag with 1/3 resolution.                *
* See "enc_lag3.c" for more details about the encoding procedure.        *
*------------------------------------------------------------------------*/

void dec_lag3cp(   /* Decode the pitch lag                   */
                int index,       /* input : received pitch index           */
                int pit_min,     /* input : minimum pitch lag              */
                int pit_max,     /* input : maximum pitch lag              */
                int i_subfr,     /* input : subframe flag                  */
                int *T0,         /* output: integer part of pitch lag      */
                int *T0_frac,    /* output: fractional part of pitch lag   */
                int rate
                )
{
    int i;
    int T0_min, T0_max;
    
    if (i_subfr == 0)                  /* if 1st subframe */
    {
        if (index < 197)
        {
            *T0 = (index+2)/3 + 19;
            *T0_frac = index - *T0*3 + 58;
        }
        else
        {
            *T0 = index - 112;
            *T0_frac = 0;
        }
    }
    
    else  /* second subframe */
    {
        /* find T0_min and T0_max for 2nd subframe */
        
        T0_min = *T0 - 5;
        if (T0_min < pit_min)
            T0_min = pit_min;
        
        T0_max = T0_min + 9;
        if(T0_max > pit_max)
        {
            T0_max = pit_max;
            T0_min = T0_max -9;
        }
        
        
        if (rate == G729D) {  /* 4 bit lag in 2nd subframe (6.4 kbit/s) */
            
            index = index & 15; /* 4 bits delta lag; assure only 4 bits used */
            if (index <= 3) {
                *T0 = T0_min + index;
                *T0_frac = 0;
            }
            else if (index < 12) {
                *T0_frac = index % 3;
                *T0 = (index - *T0_frac) / 3 + T0_min + 2;
                if (*T0_frac == 2) {
                    *T0_frac = -1;
                    (*T0)++;
                }
            }
            else {
                *T0 = T0_min + 6 + index - 12;
                *T0_frac = 0;
            }
        }
        else {          /* 5 bit lag in 2nd subframe */
            i = (index+2)/3 - 1;
            *T0 = i + T0_min;
            *T0_frac = index - 2 - i*3;
        }

    }
    
    
    return;
}
