/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex H  - Reference C code for fixed point
                         implementation of G.729 Annex H
                         (integration of Annexes D and E)
                         Version 1.1 of October 1999
*/
/*
 File : declagh.c
 */
/* from declag3d.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from dec_lag3.c G.729 Version 3.3            */

/*------------------------------------------------------------------------*
 *    Function Dec_lag3cp                                                   *
 *             ~~~~~~~~                                                   *
 *   Decoding of fractional pitch lag with 1/3 resolution.                *
 * See "Enc_lag3cp.c" for more details about the encoding procedure.        *
 *------------------------------------------------------------------------*/

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8h.h"

void Dec_lag3cp(
  Word16 index,       /* input : received pitch index           */
  Word16 pit_min,     /* input : minimum pitch lag              */
  Word16 pit_max,     /* input : maximum pitch lag              */
  Word16 i_subfr,     /* input : subframe flag                  */
  Word16 *T0,         /* output: integer part of pitch lag      */
  Word16 *T0_frac,     /* output: fractional part of pitch lag   */
  Word16 rate
)
{
    Word16 i;
    Word16 T0_min, T0_max;
    const Word16 modtab[]={1, 2, 0, 1, 2, 0, 1, 2};
    
    if (i_subfr == 0)                  /* if 1st subframe */
    {
        if (sub(index, 197) < 0)
        {
            /* *T0 = (index+2)/3 + 19 */
            
            *T0 = add(mult(add(index, 2), 10923), 19);
            
            /* *T0_frac = index - *T0*3 + 58 */
            
            i = add(add(*T0, *T0), *T0);
            *T0_frac = add(sub(index, i), 58);
        }
        else
        {
            *T0 = sub(index, 112);
            *T0_frac = 0;
        }
        
    }
    
    else  /* second subframe */
    {
        /* find T0_min and T0_max for 2nd subframe */
        
        T0_min = sub(*T0, 5);
        if (sub(T0_min, pit_min) < 0)
        {
            T0_min = pit_min;
        }
        
        T0_max = add(T0_min, 9);
        if (sub(T0_max, pit_max) > 0)
        {
            T0_max = pit_max;
            T0_min = sub(T0_max, 9);
        }
        if (rate == G729D) {  /* 4 bit lag in 2nd subframe (6.4 kbit/s) */
            /* 6.4 kbps */
            
            index = index & (Word16) 15; /* 4 bits delta lag; assure only 4 bits used */
            if (sub(index, 3) <= 0) {
                *T0 = add(T0_min, index);
                *T0_frac = 0;
            }
            else if (sub(index, 12) < 0) {
                /* *T0_frac = index % 3; */
                *T0_frac = modtab[sub(index, 4)];
                /* *T0 = (index - *T0_frac) / 3 + T0_min + 2; */
                i = sub(index, *T0_frac);
                i = mult(i, 10923);
                i = add(i, T0_min);
                *T0 = add(i, 2);
                
                if (sub(*T0_frac, 2) == 0) {
                    *T0_frac = -1;
                    *T0 = add(*T0, 1);
                }
            }
            else {
                /* *T0 = T0_min + 6 + index - 12; */
                *T0 = add(T0_min, sub(index, 6));
                *T0_frac = 0;
            }
            
        }
        else {
            /* i = (index+2)/3 - 1 */
            /* *T0 = i + t0_min;    */
            
            i = sub(mult(add(index, 2), 10923), 1);
            *T0 = add(i, T0_min);
            
            /* t0_frac = index - 2 - i*3; */
            
            i = add(add(i, i), i);
            *T0_frac = sub(sub(index, 2), i);
        }
    }
    return;
}

