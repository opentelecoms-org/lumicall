/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
    ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
                         Version 1.1 of October 1999
*/
/*
 File : utilcp.c
 */

/* from util.c G729 Annex B Version 1.3  Last modified: August 1997 */
/* from util.c G729 Version 3.3 */

/*-------------------------------------------------------------------*
* Function  Set zero()                                              *
*           ~~~~~~~~~~                                              *
* Set vector x[] to zero                                            *
*-------------------------------------------------------------------*/

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"

void Set_zero(
              Word16 x[],       /* (o)    : vector to clear     */
              Word16 L          /* (i)    : length of vector    */
              )
{
    Word16 i;
    
    for (i = 0; i < L; i++)
        x[i] = 0;
    
    return;
}
/*-------------------------------------------------------------------*
* Function  Copy:                                                   *
*           ~~~~~                                                   *
* Copy vector x[] to y[]                                            *
*-------------------------------------------------------------------*/

void Copy(
          Word16 x[],      /* (i)   : input vector   */
          Word16 y[],      /* (o)   : output vector  */
          Word16 L         /* (i)   : vector length  */
          )
{
    Word16 i;
    
    for (i = 0; i < L; i++)
        y[i] = x[i];
    
    return;
}

/* Random generator  */

Word16 Random_g729cp(Word16 *seed)
{
    
    /* seed = seed*31821 + 13849; */
    *seed = extract_l(L_add(L_shr(L_mult(*seed, 31821), 1), 13849L));
    
    return(*seed);
}
