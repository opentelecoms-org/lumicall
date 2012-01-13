/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : FILTERE.C
*/

/***************************************/
/* General filter routines             */
/***************************************/
#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"

/*-----------------------------------------------------------*
* convolve - convolve vectors x and h and put result in y   *
*-----------------------------------------------------------*/

void convolve(
    FLOAT x[],             /* input : input vector x[0:l]                     */
    FLOAT h[],             /* input : impulse response or second input h[0:l] */
    FLOAT y[],             /* output: x convolved with h , y[0:l]             */
    int  l                 /* input : dimension of all vectors                */
)
{
    FLOAT temp;
    int    i, n;
    
    for (n = 0; n < l; n++)
    {
        temp = (F)0.0;
        for (i = 0; i <= n; i++)
            temp += x[i]*h[n-i];
        y[n] = temp;
    }
    
    return;
}

/*-----------------------------------------------------------*
* syn_filt - filter with synthesis filter 1/A(z)            *
*-----------------------------------------------------------*/

void syn_filte(
    int m,         /* (i)    : LPC order                         */
    FLOAT a[],     /* input : predictor coefficients a[0:m]    */
    FLOAT x[],     /* input : excitation signal                */
    FLOAT y[],     /* output: filtered output signal           */
    int  l,        /* input : vector dimension                 */
    FLOAT mem[],   /* in/out: filter memory                    */
    int  update    /* input : 0 = no memory update, 1 = update */
)
{
    int  i,j;
    
    /* This is usually done by memory allocation (l+m) */
    FLOAT yy_b[L_SUBFR+M_BWD];
    FLOAT s, *yy, *py, *pa;
    
    /* Copy mem[] to yy[] */
    yy = yy_b;
    for (i = 0; i <m; i++)  *yy++ =  *mem++;
    
    /* Filtering */
    for (i = 0; i < l; i++)
    {
        py=yy;
        pa=a;
        s = *x++;
        for (j = 0; j <m; j++)  s -= (*++pa) * (*--py);
        *yy++ = s;
        *y++ = s;
    }
    
    /* Update memory if required */
    if(update !=0 ) for (i = 0; i <m; i++)  *--mem =*--yy;
    
    return;
}

/*-----------------------------------------------------------*
* residu - filter input vector with all-zero filter A(Z)    *
*-----------------------------------------------------------*/
void residue(    
    int m,         /* (i)    : LPC order                         */
    FLOAT *a,      /* input : prediction coefficients a[0:m+1], a[0]=1. */
    FLOAT *x,      /* input : input signal x[0:l-1], x[-1:m] are needed */
    FLOAT *y,      /* output: output signal y[0:l-1] NOTE: x[] and y[]
                        cannot point to same array               */
    int  l        /* input : dimension of x and y                      */
)
{
    FLOAT s;
    int  i, j;
    
    for (i = 0; i < l; i++)
    {
        s = x[i];
        for (j = 1; j <= m; j++) s += a[j]*x[i-j];
        *y++ = s;
    }
    return;
}
