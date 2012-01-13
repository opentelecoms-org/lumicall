/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : UTILCP.C
*/


/*****************************************************************************/
/* auxiliary functions                                                       */
/*****************************************************************************/

#include "typedef.h"
#include "ld8k.h"

/*-------------------------------------------------------------------*
* Function  set zero()                                              *
*           ~~~~~~~~~~                                              *
* Set vector x[] to zero                                            *
*-------------------------------------------------------------------*/
void set_zero(
    FLOAT  x[],       /* (o)    : vector to clear     */
    int L             /* (i)    : length of vector    */
)
{
    int i;
    
    for (i = 0; i < L; i++)
        x[i] = (F)0.0;
    
    return;
}

/*-------------------------------------------------------------------*
* Function  copy:                                                   *
*           ~~~~~                                                   *
* Copy vector x[] to y[]                                            *
*-------------------------------------------------------------------*/
void copy(
    FLOAT  x[],      /* (i)   : input vector   */
    FLOAT  y[],      /* (o)   : output vector  */
    int L            /* (i)   : vector length  */
)
{
    int i;
    
    for (i = 0; i < L; i++)
        y[i] = x[i];
    
    return;
}

/* Random generator  */
INT16 random_g729c(INT16 *seed)
{
        
    *seed = (INT16) ((*seed) * 31821L + 13849L);
    
    return(*seed);
    
}

/*-----------------------------------------------------------*
* fwrite16 - writes a FLOAT array as a Short to a a file    *
*-----------------------------------------------------------*/
void fwrite16(
    FLOAT *data,           /* input: inputdata */
    int length,          /* input: length of data array */
    FILE *fp               /* input: file pointer */
)
{
    int  i;
    INT16 sp16[L_FRAME];
    FLOAT temp;
    
    if (length > L_FRAME) {
        printf("error in fwrite16\n");
        exit(16);
    }
    
    for(i=0; i<length; i++)
    {
        /* round and convert to int  */
        temp = data[i];
        if (temp >= (F)0.0)
            temp += (F)0.5;
        else  temp -= (F)0.5;
        if (temp >  (F)32767.0 ) temp =  (F)32767.0;
        if (temp < (F)-32768.0 ) temp = (F)-32768.0;
        sp16[i] = (INT16) temp;
    }
    fwrite( sp16, sizeof(INT16), length, fp);
    return;
}

/*****************************************************************************/
/* Functions used by VAD.C                                                   */
/*****************************************************************************/
void dvsub(FLOAT *in1, FLOAT *in2, FLOAT *out, INT16 npts)
{
    while (npts--)  *(out++) = *(in1++) - *(in2++);
}

FLOAT dvdot(FLOAT *in1, FLOAT *in2, INT16 npts)
{
    FLOAT accum;
    
    accum = (F)0.0;
    while (npts--)  accum += *(in1++) * *(in2++);
    return(accum);
}

void dvwadd(FLOAT *in1, FLOAT scalar1, FLOAT *in2, FLOAT scalar2,
                        FLOAT *out, INT16 npts)
{
    while (npts--)  *(out++) = *(in1++) * scalar1 + *(in2++) * scalar2;
}

void dvsmul(FLOAT *in, FLOAT scalar, FLOAT *out, INT16 npts)
{
    while (npts--)  *(out++) = *(in++) * scalar;
}

