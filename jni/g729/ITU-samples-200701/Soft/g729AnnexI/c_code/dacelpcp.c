/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
    ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
                         Version 1.1 of October 1999
*/

/*
 File : dacelpcp.c
 */
/* from deacelpe.c G.729 Annex E Version 1.2  Last modified: May 1998 */
/* from deacelpd.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from de_acelp.c G.729 Version 3.3            */

/*-----------------------------------------------------------*
*   Algebraic codebook decoder.                             *
*----------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

/* local function */
static void dec_pulses(
                       Word16 index,        /* (i)       : pulses index (4 or 7 bits)    */
                       Word16 track,        /* (i)       : track of the pulses           */
                       Word16 nb_of_pulses, /* (i)       : 1 or 2 pulses                 */
                       Word16 cod[]         /* (i/o) Q13 : algebraic codebook excitation */
                       );
                       /*-----------------------------------------------------------*
                       *  Function  Decod_ACELP()                                  *
                       *  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
                       *   Algebraic codebook decoder.                             *
*----------------------------------------------------------*/
void Decod_ACELP(
                 Word16 sign,      /* (i)     : signs of 4 pulses.                       */
                 Word16 index,     /* (i)     : Positions of the 4 pulses.               */
                 Word16 cod[]      /* (o) Q13 : algebraic (fixed) codebook excitation    */
                 )
{
    Word16 i, j;
    Word16 pos[4];
    
    /* Decode the positions */
    i      = index & (Word16)7;
    pos[0] = add(i, shl(i, 2));           /* pos0 =i*5 */
    
    index  = shr(index, 3);
    i      = index & (Word16)7;
    i      = add(i, shl(i, 2));           /* pos1 =i*5+1 */
    pos[1] = add(i, 1);
    
    index  = shr(index, 3);
    i      = index & (Word16)7;
    i      = add(i, shl(i, 2));           /* pos2 =i*5+1 */
    pos[2] = add(i, 2);
    
    index  = shr(index, 3);
    j      = index & (Word16)1;
    index  = shr(index, 1);
    i      = index & (Word16)7;
    i      = add(i, shl(i, 2));           /* pos3 =i*5+3+j */
    i      = add(i, 3);
    pos[3] = add(i, j);
    
    /* decode the signs  and build the codeword */
    for (i=0; i<L_SUBFR; i++) {
        cod[i] = 0;
    }
    for (j=0; j<4; j++) {
        i = sign & (Word16)1;
        sign = shr(sign, 1);
        if (i != 0) {
            cod[pos[j]] = 8191;      /* Q13 +1.0 */
        }
        else {
            cod[pos[j]] = -8192;     /* Q13 -1.0 */
        }
    }
    return;
}


void Dec_ACELP_10i40_35bits(
                            Word16 *index,  /* (i)     : 5 words index (positions & sign)      */
                            Word16 cod[]    /* (o) Q13 : algebraic (fixed) codebook excitation */
                            )
{
    Word16 i, j;
    
    /* decode the positions and signs of pulses and build the codeword */
    for (i=0; i<L_SUBFR; i++) {
        cod[i] = 0;
    }
    
    for (j=0; j<5; j++) {
        dec_pulses(index[j], j, 2, cod);
    }
    
    return;
}
void Dec_ACELP_12i40_44bits(
                            Word16 *index,  /* (i)     : 5 words index (positions & sign)      */
                            Word16 cod[]    /* (o) Q13 : algebraic (fixed) codebook excitation */
                            )
{
    Word16 i, j, track;
    
    /* decode the positions and signs of pulses and build the codeword */
    for (i=0; i<L_SUBFR; i++) {
        cod[i] = 0;
    }
    
    track = shr(index[0], 10) & (Word16)7;
    if (track > 4) track = 4;
    
    for (j=0; j<2; j++) {
        dec_pulses(index[j], track, 3, cod);
        track = add(track, 1);
        if (track > 4) track = 0;
    }
    
    for (j=2; j<5; j++) {
        dec_pulses(index[j], track, 2, cod);
        track = add(track, 1);
        if (track > 4) track = 0;
    }
    
    return;
}
/*-----------------------------------------------------------*
*  Function  decod_ACELP64()                                *
*  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
*   Algebraic codebook decoder.                             *
*----------------------------------------------------------*/
void Decod_ACELP64(
                   Word16 sign,              /* input : signs of 2 pulses     */
                   Word16 index,             /* input : positions of 2 pulses */
                   Word16 cod[]          /* Q13 output: innovative codevector */
                   )
{
    Word16 pos[4];
    Word16 i, j;
    
    /* decode the positions of 4 pulses */
    i = index & (Word16) 15;
    pos[0] = trackTable0[ grayDecode[i] ];
    
    index = shr(index, 4);
    i = index & (Word16) 31;
    pos[1] = trackTable1[ grayDecode[i] ];
    
    /* find the algebraic codeword */
    for (i = 0; i < L_SUBFR; i++) cod[i] = 0;
    
    /* decode the signs of 2 pulses */
    for (j=0; j<2; j++) {
        i = sign & (Word16) 1;
        sign = shr(sign, 1);
        if (i != 0) {
            cod[pos[j]] = add(cod[pos[j]], 8191); /* 1 in Q13 */
        }
        else {
            cod[pos[j]] = sub(cod[pos[j]], 8192); /* -1 in Q13 */
        }
    }
    return;
}

static void dec_pulses(
                       Word16 index,        /* (i)       : pulses index (4 or 7 bits)    */
                       Word16 track,        /* (i)       : track of the pulses           */
                       Word16 nb_of_pulses, /* (i)       : 1, 2 or 3 pulses              */
                       Word16 cod[]         /* (i/o) Q13 : algebraic codebook excitation */
                       )
{
    
    Word16 i, pos1, pos2, pos3, sign;
    
    /* compute index i */
    i = extract_l(L_shr(L_mult((Word16)(index & (Word16)7), 5), 1));
    pos1 = add(i, track);         /* position of pulse 1 */
    i = shr(index, 3) & (Word16)1;
    if (i == 0) sign = 4096;          /* Q12 +1.0 */
    else sign = -4096;                /* Q12 -1.0 */
    cod[pos1] = sign;             

    if (nb_of_pulses >= 2)
    {
        /* compute index i */
        i = extract_l(L_shr(L_mult((Word16)(shr(index, 4) & (Word16)7), 5), 1));
        pos2 = add(i, track);       /* position of pulse 2 */
        if (sub(pos2,pos1) > 0)
        {
            sign = negate(sign);
        }
        cod[pos2] = add(cod[pos2], sign);
    }  

    if (nb_of_pulses == 3)
    {
        /* compute index i */
        i = extract_l(L_shr(L_mult((Word16)(shr(index, 7) & (Word16)7), 5), 1));
        pos3 = add(i, track);       /* position of pulse 2 */
        if (sub(pos3,pos2) > 0)
        {
            sign = negate(sign);
        }
        cod[pos3] = add(cod[pos3], sign);
    }  
    
    return;
}


                       
