/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/

/*
 File : LPFUNCCP.C
*/

#include <math.h>
#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"

/* local function */
static void  get_lsp_pol(FLOAT lsf[],FLOAT f[]);

/*-----------------------------------------------------------------------------
* lsp_az - convert LSPs to predictor coefficients a[]
*-----------------------------------------------------------------------------
*/
void lsp_az(
    FLOAT *lsp,            /* input : lsp[0:M-1] */
    FLOAT *a               /* output: predictor coeffs a[0:M], a[0] = 1. */
)
{
    
    FLOAT f1[NC+1], f2[NC+1];
    int i,j;
    
    get_lsp_pol(&lsp[0],f1);
    get_lsp_pol(&lsp[1],f2);
    
    for (i = NC; i > 0; i--)
    {
        f1[i] += f1[i-1];
        f2[i] -= f2[i-1];
    }
    a[0] = (F)1.0;
    for (i = 1, j = M; i <= NC; i++, j--)
    {
        a[i] = (F)0.5*(f1[i] + f2[i]);
        a[j] = (F)0.5*(f1[i] - f2[i]);
    }
    
    return;
}

/*----------------------------------------------------------------------------
* get_lsp_pol - find the polynomial F1(z) or F2(z) from the LSFs
*----------------------------------------------------------------------------
*/
static void get_lsp_pol(
    FLOAT lsp[],           /* input : line spectral freq. (cosine domain)  */
    FLOAT f[]              /* output: the coefficients of F1 or F2 */
)
{
    FLOAT b;
    int   i,j;
    
    f[0] = (F)1.0;
    b = (F)-2.0*lsp[0];
    f[1] = b;
    for (i = 2; i <= NC; i++)
    {
        b = (F)-2.0*lsp[2*i-2];
        f[i] = b*f[i-1] + (F)2.0*f[i-2];
        for (j = i-1; j > 1; j--)
            f[j] += b*f[j-1] + f[j-2];
        f[1] += b;
    }
    return;
}

/*----------------------------------------------------------------------------
* lsf_lsp - convert from lsf[0..M-1 to lsp[0..M-1]
*----------------------------------------------------------------------------
*/
void lsf_lsp(
    FLOAT lsf[],          /* input :  lsf */
    FLOAT lsp[M],          /* output: lsp */
    int m
)
{
    int     i;
    for ( i = 0; i < m; i++ )
        lsp[i] = (FLOAT)cos((double)lsf[i]);
    return;
}

/*----------------------------------------------------------------------------
* lsp_lsf - convert from lsp[0..M-1 to lsf[0..M-1]
*----------------------------------------------------------------------------
*/
void lsp_lsf(
    FLOAT lsp[],          /* input :  lsp coefficients */
    FLOAT lsf[],          /* output:  lsf (normalized frequencies */
    int m
)
{
    int     i;
    
    for ( i = 0; i < m; i++ )
        lsf[i] = (FLOAT)acos((double)lsp[i]);
    return;
}

/*---------------------------------------------------------------------------
* weigh_az:  Weighting of LPC coefficients  ap[i]  =  a[i] * (gamma ** i)
*---------------------------------------------------------------------------
*/
void weight_az(
    FLOAT *a,              /* input : lpc coefficients a[0:m] */
    FLOAT gamma,           /* input : weighting factor */
    int m,                  /* input : filter order */
    FLOAT *ap             /* output: weighted coefficients ap[0:m] */
)
{
    FLOAT fac;
    int i;
    
    ap[0] = a[0];
    fac = gamma;
    for (i = 1; i <m; i++) {
        ap[i] = fac * a[i];
        fac *= gamma;
    }
    ap[m] = fac * a[m];
    return;
}

/*-----------------------------------------------------------------------------
* int_qlpc -  interpolated M LSP parameters and convert to M+1 LPC coeffs
*-----------------------------------------------------------------------------
*/
void int_qlpc(
    FLOAT lsp_old[],       /* input : LSPs for past frame (0:M-1) */
    FLOAT lsp_new[],       /* input : LSPs for present frame (0:M-1) */
    FLOAT az[]             /* output: filter parameters in 2 subfr (dim 2(m+1)) */
)
{
    int i;
    FLOAT lsp[M];
    
    for (i = 0; i < M; i++)
        lsp[i] = lsp_old[i]*(F)0.5 + lsp_new[i]*(F)0.5;
    
    lsp_az(lsp, az);
    lsp_az(lsp_new, &az[M+1]);
    
    return;
}
/*-----------------------------------------------------------------------------
* int_lpc -  interpolated M LSP parameters and convert to M+1 LPC coeffs
*-----------------------------------------------------------------------------
*/
void int_lpc(
    FLOAT lsp_old[],       /* input : LSPs for past frame (0:M-1) */
    FLOAT lsp_new[],       /* input : LSPs for present frame (0:M-1) */
    FLOAT lsf_int[],        /* output: interpolated lsf coefficients */
    FLOAT lsf_new[],       /* input : LSFs for present frame (0:M-1) */
    FLOAT az[]             /* output: filter parameters in 2 subfr (dim 2(m+1)) */
)
{
    int i;
    FLOAT lsp[M];
    
    for (i = 0; i < M; i++)
        lsp[i] = lsp_old[i]*(F)0.5 + lsp_new[i]*(F)0.5;
    
    lsp_az(lsp, az);
    
    lsp_lsf(lsp, lsf_int, M);
    lsp_lsf(lsp_new, lsf_new, M);
    
    return;
}

