/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
File : DACELPCP.C
*/
#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

static void dec_pulses(
    int index,        /* (i)       : pulses index (4 or 7 bits)    */
    int track,        /* (i)       : track of the pulses           */
    int nb_of_pulses, /* (i)       : 1 or 2 pulses                 */
    FLOAT cod[]       /* (i/o)     : algebraic codebook excitation */
);

/*-----------------------------------------------------------*
*  Function  decod_ACELP()                                  *
*  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
*   Algebraic codebook decoder.                             *
*----------------------------------------------------------*/

void decod_ACELP(
    int sign,              /* input : signs of 4 pulses     */
    int index,             /* input : positions of 4 pulses */
    FLOAT cod[]            /* output: innovative codevector */
)
{
    int pos[4];
    int i, j;
    
    /* decode the positions of 4 pulses */
    i = index & 7;
    pos[0] = i*5;
    
    index >>= 3;
    i = index & 7;
    pos[1] = i*5 + 1;
    
    index >>= 3;
    i = index & 7;
    pos[2] = i*5 + 2;
    
    index >>= 3;
    j = index & 1;
    index >>= 1;
    i = index & 7;
    pos[3] = i*5 + 3 + j;
    
    /* find the algebraic codeword */
    
    for (i = 0; i < L_SUBFR; i++) cod[i] = 0;
    
    /* decode the signs of 4 pulses */
    
    for (j=0; j<4; j++)
    {
        
        i = sign & 1;
        sign >>= 1;
        
        if (i != 0) {
            cod[pos[j]] = (F)1.0;
        }
        else {
            cod[pos[j]] = (F)-1.0;
        }
    }
    
    return;
}

void dec_ACELP_10i40_35bits(
    int *index,     /* (i): 5 words index (positions & sign)      */
    FLOAT cod[]    /* (o) : algebraic (fixed) codebook excitation */
)
{
    int i, j;

    /* decode the positions and signs of pulses and build the codeword */
    for (i=0; i<L_SUBFR; i++) {
        cod[i] = (FLOAT)0.;
    }

    for (j=0; j<5; j++){
        dec_pulses(index[j], j, 2, cod);
    }
    return;
}

void dec_ACELP_12i40_44bits(
    int *index,    /* (i) : 5 words index (positions & sign)      */
    FLOAT cod[]    /* (o) : algebraic (fixed) codebook excitation */
)
{
    int i, j, track;

    /* decode the positions and signs of pulses and build the codeword */
    for (i=0; i<L_SUBFR; i++) {
        cod[i] = (FLOAT)0.;
    }

    track = (index[0]>> 10) & (int)7;
    if (track > 4) track = 4;

    for (j=0; j<2; j++) {
        dec_pulses(index[j], track, 3, cod);
        track++;
        if (track > 4) track = 0;
    }

    for (j=2; j<5; j++) {
        dec_pulses(index[j], track, 2, cod);
        track++;
        if (track > 4) track = 0;
    }
    
    return;
}

static void dec_pulses(
    int index,        /* (i)       : pulses index (4 or 7 bits)    */
    int track,        /* (i)       : track of the pulses           */
    int nb_of_pulses, /* (i)       : 1, 2 or 3 pulses              */
    FLOAT cod[]       /* (i/o)   : algebraic codebook excitation */
)
{
    int i, pos1, pos2, pos3;
    FLOAT sign;

    /* compute index i */
    i = (int) ( (index & (int)7) * 5 );
    pos1 = i+ track;         /* position of pulse 1 */
    i = (int)(index>> 3) & (int)1;
    
    if (i == 0) sign = (FLOAT)1.;
    else sign = (FLOAT)-1.;

    cod[pos1] = sign;
    
    pos2 = 0; /* to avoid visual warning */
    if (nb_of_pulses >= 2) {
        /* compute index i */
        i = ( (index>>4) & (int)7) * 5;
        pos2 = i+ track;       /* position of pulse 2 */
        if (pos2>pos1) {
            sign = -sign;
        }
        cod[pos2] += sign;
    }
    
    if (nb_of_pulses == 3) {
        /* compute index i */
        i = ((index>> 7) & (int)7)* 5;
        pos3 = i+ track;       /* position of pulse 2 */
        if (pos3>pos2) {
            sign = -sign;
        }
        cod[pos3] += sign;
    }
    return;
}
/*-----------------------------------------------------------*
*  Function  decod_ACELP64()                                *
*  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
*   Algebraic codebook decoder.                             *
*----------------------------------------------------------*/

void decod_ACELP64(
    int sign,              /* input : signs of 2 pulses     */
    int index,             /* input : positions of 2 pulses */
    FLOAT cod[]            /* output: innovative codevector */
)
{
    int pos[2];
    int i, j;

    /* decode the positions of 4 pulses */
    i = index & 15;
    pos[0] = trackTable0[grayDecode[i]];

    index >>= 4;
    i = index & 31;
    pos[1] = trackTable1[grayDecode[i]];

    /* find the algebraic codeword */
    
    for (i = 0; i < L_SUBFR; i++) cod[i] = 0;
    
    /* decode the signs of 2 pulses */
    for (j=0; j<2; j++)
    {
        i = sign & 1;
        sign >>= 1;
        
        if (i != 0) {
            cod[pos[j]] += (F)1.0;
        }
        else {
            cod[pos[j]] -= (F)1.0;
        }
    }
    
    return;
}
