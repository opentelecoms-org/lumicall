/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729 Annex A
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.
----------------------------------------------------------------------
*/

/*
 File : LPCA.C
 Used for the floating point version of G.729A only
 (not for G.729 main body)
*/

/*****************************************************************************/
/* lpc analysis routines                                                     */
/*****************************************************************************/

#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"

/* NOTE: these routines are assuming that the order is defined as M */
/*       and that NC is defined as M/2. M has to be even           */

/*----------------------------------------------------------------------------
 * autocorr - compute the auto-correlations of windowed speech signal
 *----------------------------------------------------------------------------
 */
void autocorr(
     FLOAT *x,              /* input : input signal x[0:L_WINDOW] */
     int m,                 /* input : LPC order                  */
     FLOAT *r               /* output: auto-correlation vector r[0:M]*/
)
{
   static FLOAT y[L_WINDOW];  /* dynamic memory allocation is used in real time*/
   FLOAT sum;
   int i, j;


   for (i = 0; i < L_WINDOW; i++)
        y[i] = x[i]*hamwindow[i];

   for (i = 0; i <= m; i++)
   {
     sum = (F)0.0;
     for (j = 0; j < L_WINDOW-i; j++)
          sum += y[j]*y[j+i];
     r[i] = sum;
   }
   if (r[0]<(F)1.0) r[0]=(F)1.0;

   return;
}

/*-------------------------------------------------------------*
 * procedure lag_window:                                       *
 *           ~~~~~~~~~                                         *
 * lag windowing of the autocorrelations                       *
 *-------------------------------------------------------------*/

void lag_window(
     int m,                 /* input : LPC order                  */
     FLOAT   r[]            /* in/out: correlation */
)
{
   int i;

   for (i=0; i<= m; i++)
     r[i] *= lwindow[i];

   return;
}


/*----------------------------------------------------------------------------
 * levinson - levinson-durbin recursion to compute LPC parameters
 *----------------------------------------------------------------------------
 */
FLOAT levinson(         /* output: prediction error (energy) */
 FLOAT *r,              /* input : auto correlation coefficients r[0:M] */
 FLOAT *a,              /* output: lpc coefficients a[0] = 1 */
 FLOAT *rc              /* output: reflection coefficients rc[0:M-1]    */
)
{
   FLOAT s, at, err;
   int i, j, l;

   rc[0] = (-r[1])/r[0];
   a[0] = (F)1.0;
   a[1] = rc[0];
   err = r[0] + r[1]*rc[0];
   for (i = 2; i <= M; i++)
   {
     s = (F)0.0;
     for (j = 0; j < i; j++)
       s += r[i-j]*a[j];
     rc[i-1]= (-s)/(err);
     for (j = 1; j <= (i/2); j++)
     {
       l = i-j;
       at = a[j] + rc[i-1]*a[l];
       a[l] += rc[i-1]*a[j];
       a[j] = at;
     }
     a[i] = rc[i-1];
     err += rc[i-1]*s;
     if (err <= (F)0.0)
        err = (F)0.001;
   }
   return (err);
}

/*------------------------------------------------------------------*
 *  procedure az_lsp:                                               *
 *            ~~~~~~                                                *
 *   Compute the LSPs from  the LP coefficients a[] using Chebyshev *
 * polynomials. The found LSPs are in the cosine domain with values *
 * in the range from 1 down to -1.                                  *
 * The table grid[] contains the points (in the cosine domain) at   *
 * which the polynomials are evaluated. The table corresponds to    *
 * NO_POINTS frequencies uniformly spaced between 0 and pi.         *
 *------------------------------------------------------------------*/
/* prototypes of local functions */

static FLOAT chebyshev(FLOAT x, FLOAT *f, int n);

void az_lsp(
  FLOAT *a,         /* input : LP filter coefficients                     */
  FLOAT *lsp,       /* output: Line spectral pairs (in the cosine domain) */
  FLOAT *old_lsp    /* input : LSP vector from past frame                 */
)
{
 int i, j, nf, ip;
 FLOAT xlow,ylow,xhigh,yhigh,xmid,ymid,xint;
 FLOAT *pf1;

 FLOAT f1[NC+1], f2[NC+1];

 /*-------------------------------------------------------------*
  * find the sum and diff polynomials F1(z) and F2(z)           *
  *      F1(z) = [A(z) + z^11 A(z^-1)]/(1+z^-1)                 *
  *      F2(z) = [A(z) - z^11 A(z^-1)]/(1-z^-1)                 *
  *-------------------------------------------------------------*/

 f1[0] = (F)1.0;
 f2[0] = (F)1.0;
 for (i=1, j=M; i<=NC; i++, j--){
    f1[i] = a[i]+a[j]-f1[i-1];
    f2[i] = a[i]-a[j]+f2[i-1];
 }

 /*---------------------------------------------------------------------*
  * Find the LSPs (roots of F1(z) and F2(z) ) using the                 *
  * Chebyshev polynomial evaluation.                                    *
  * The roots of F1(z) and F2(z) are alternatively searched.            *
  * We start by finding the first root of F1(z) then we switch          *
  * to F2(z) then back to F1(z) and so on until all roots are found.    *
  *                                                                     *
  *  - Evaluate Chebyshev pol. at grid points and check for sign change.*
  *  - If sign change track the root by subdividing the interval        *
  *    NO_ITER times and ckecking sign change.                          *
  *---------------------------------------------------------------------*/

 nf=0;      /* number of found frequencies */
 ip=0;      /* flag to first polynomial   */

 pf1 = f1;  /* start with F1(z) */

 xlow=grid[0];
 ylow = chebyshev(xlow,pf1,NC);

 j = 0;
 while ( (nf < M) && (j < GRID_POINTS) )
 {
   j++;
   xhigh = xlow;
   yhigh = ylow;
   xlow = grid[j];
   ylow = chebyshev(xlow,pf1,NC);

   if (ylow*yhigh <= 0.0)  /* if sign change new root exists */
   {
     j--;

     /* divide the interval of sign change by 2 */

     for (i = 0; i < 2; i++)
     {
       xmid = (F)0.5*(xlow + xhigh);
       ymid = chebyshev(xmid,pf1,NC);
       if (ylow*ymid <= (F)0.0)
       {
         yhigh = ymid;
         xhigh = xmid;
       }
       else
       {
         ylow = ymid;
         xlow = xmid;
       }
     }

     /* linear interpolation for evaluating the root */

     xint = xlow - ylow*(xhigh-xlow)/(yhigh-ylow);

     lsp[nf] = xint;    /* new root */
     nf++;

     ip = 1 - ip;         /* flag to other polynomial    */
     pf1 = ip ? f2 : f1;  /* pointer to other polynomial */

     xlow = xint;
     ylow = chebyshev(xlow,pf1,NC);
   }
 }

 /* Check if M roots found */
 /* if not use the LSPs from previous frame */

 if ( nf < M)
    for(i=0; i<M; i++)  lsp[i] = old_lsp[i];

 return;
}
/*------------------------------------------------------------------*
 *            End procedure az_lsp()                                *
 *------------------------------------------------------------------*/

/*--------------------------------------------------------------*
 * function  chebyshev:                                         *
 *           ~~~~~~~~~~                                         *
 *    Evaluates the Chebyshev polynomial series                 *
 *--------------------------------------------------------------*
 *  The polynomial order is                                     *
 *     n = m/2   (m is the prediction order)                    *
 *  The polynomial is given by                                  *
 *    C(x) = T_n(x) + f(1)T_n-1(x) + ... +f(n-1)T_1(x) + f(n)/2 *
 *--------------------------------------------------------------*/

FLOAT chebyshev(   /* output: the value of the polynomial C(x)       */
  FLOAT x,         /* input : value of evaluation; x=cos(freq)       */
  FLOAT *f,        /* input : coefficients of sum or diff polynomial */
  int n            /* input : order of polynomial                    */
)
{
  FLOAT b1, b2, b0, x2;
  int i;                            /* for the special case of 10th order */
                                      /*       filter (n=5)                 */
  x2 = (F)2.0*x;                         /* x2 = 2.0*x;                        */
  b2 = (F)1.0;  /* f[0] */               /*                                    */
  b1 = x2 + f[1];                     /* b1 = x2 + f[1];                    */
  for (i=2; i<n; i++) {               /*                                    */
    b0 = x2*b1 - b2 + f[i];           /* b0 = x2 * b1 - 1. + f[2];          */
    b2 = b1;                          /* b2 = x2 * b0 - b1 + f[3];          */
    b1 = b0;                          /* b1 = x2 * b2 - b0 + f[4];          */
  }                                   /*                                    */
  return (x*b1 - b2 + (F)0.5*f[n]);      /* return (x*b1 - b2 + 0.5*f[5]);     */
}

