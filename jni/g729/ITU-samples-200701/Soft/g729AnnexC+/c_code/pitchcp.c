/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : PITCHCP.C
*/

/*****************************************************************************/
/*           Long Term Prediction Routines                                   */
/*****************************************************************************/
#include <math.h>
#include "typedef.h"
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"

/* prototypes for local functions */
static void  norm_corr(FLOAT exc[], FLOAT xn[], FLOAT h[], int l_subfr,
    int t_min, int t_max, FLOAT corr_norm[]);
static FLOAT interpol_3(FLOAT cor[], int frac);
static int   lag_max(FLOAT signal[],int L_frame,int pit_max,
                  int pit_min, FLOAT *cor_max);
static FLOAT inv_sqrt( FLOAT x  );

/*----------------------------------------------------------------------------
 * pitch_ol -  compute the open loop pitch lag
 *----------------------------------------------------------------------------
 */
int pitch_ol(           /* output: open-loop pitch lag */
    FLOAT signal[],        /* input : signal to compute pitch  */
                           /*         s[-PIT_MAX : l_frame-1]  */
    int pit_min,         /* input : minimum pitch lag                          */
    int pit_max,         /* input : maximum pitch lag                          */
    int l_frame          /* input : error minimization window */
)
{
    FLOAT  max1, max2, max3;
    int    p_max1, p_max2, p_max3;
    
    /*--------------------------------------------------------------------*
    *  The pitch lag search is divided in three sections.                *
    *  Each section cannot have a pitch multiple.                        *
    *  We find a maximum for each section.                               *
    *  We compare the maxima of each section by favoring small lag.      *
    *                                                                    *
    *  First section:  lag delay = PIT_MAX to 80                         *
    *  Second section: lag delay = 79 to 40                              *
    *  Third section:  lag delay = 39 to 20                              *
    *--------------------------------------------------------------------*/
    
    p_max1 = lag_max(signal, l_frame, pit_max, 80 , &max1);
    p_max2 = lag_max(signal, l_frame, 79     , 40 , &max2);
    p_max3 = lag_max(signal, l_frame, 39     , pit_min , &max3);
    
    /*--------------------------------------------------------------------*
    * Compare the 3 sections maxima, and favor small lag.                *
    *--------------------------------------------------------------------*/
    
    if ( max1 * THRESHPIT < max2 ) {
        max1 = max2;
        p_max1 = p_max2;
    }
    
    if ( max1 * THRESHPIT < max3 )  p_max1 = p_max3;
    
    return (p_max1);
}
/*----------------------------------------------------------------------------
* lag_max - Find the lag that has maximum correlation
*----------------------------------------------------------------------------
*/
static int lag_max(     /* output: lag found */
    FLOAT signal[],       /* input : Signal to compute the open loop pitch
    signal[-142:-1] should be known.       */
    int l_frame,          /* input : Length of frame to compute pitch       */
    int lagmax,           /* input : maximum lag                            */
    int lagmin,           /* input : minimum lag                            */
    FLOAT *cor_max        /* input : normalized correlation of selected lag */
)
{
    int    i, j;
    FLOAT  *p, *p1;
    FLOAT  max, t0;
    int    p_max;
    
    max = FLT_MIN_G729;
    p_max = lagmax; /* to avoid visual warning */
    
    for (i = lagmax; i >= lagmin; i--) {
        p  = signal;
        p1 = &signal[-i];
        t0 = (F)0.0;
        
        for (j=0; j<l_frame; j++) {
            t0 += *p++ * *p1++;
        }
        
        if (t0 >= max) {
            max    = t0;
            p_max = i;
        }
    }
    
    /* compute energy */
    
    t0 = (F)0.01;                  /* to avoid division by zero */
    p = &signal[-p_max];
    for(i=0; i<l_frame; i++, p++) {
        t0 += *p * *p;
    }
    t0 = inv_sqrt(t0);          /* 1/sqrt(energy)    */
    
    *cor_max = max * t0;        /* max/sqrt(energy)  */
    
    return(p_max);
}


/*----------------------------------------------------------------------------
* pitch_fr3cp - find the pitch period  with 1/3 subsample resolution
*----------------------------------------------------------------------------
*/
int pitch_fr3cp(        /* output: integer part of pitch period        */
    FLOAT exc[],            /* input : excitation buffer                   */
    FLOAT xn[],             /* input : target vector                       */
    FLOAT h[],              /* input : impulse response of filters.        */
    int l_subfr,           /* input : Length of frame to compute pitch    */
    int t0_min,            /* input : minimum value in the searched range */
    int t0_max,            /* input : maximum value in the searched range */
    int i_subfr,           /* input : indicator for first subframe        */
    int *pit_frac,         /* output: chosen fraction                     */
    int rate
)
{
    int    i, frac;
    int    lag, t_min, t_max;
    FLOAT  max;
    FLOAT  corr_int;
    FLOAT  corr_v[10+2*L_INTER4];  /* size: 2*L_INTER4+t0_max-t0_min+1 */
    FLOAT  *corr;
    
    int midLag;
    
    /* Find interval to compute normalized correlation */
    t_min = t0_min - L_INTER4;
    t_max = t0_max + L_INTER4;
    
    corr = &corr_v[-t_min];    /* corr[t_min:t_max] */
    
    /* Compute normalized correlation between target and filtered excitation */
    norm_corr(exc, xn, h, l_subfr, t_min, t_max, corr);
    
    /* find integer pitch */
    max = corr[t0_min];
    lag  = t0_min;
    
    for(i= t0_min+1; i<=t0_max; i++)
    {
        if( corr[i] >= max)
        {
            max = corr[i];
            lag = i;
        }
    }
    
    /* If first subframe and lag > 84 do not search fractionnal pitch */
    
    if( (i_subfr == 0) && (lag > 84) )
    {
        *pit_frac = 0;
        return(lag);
    }
    
    /* test the fractions around lag and choose the one which maximizes
    the interpolated normalized correlation */
    
    if (rate == G729D) {    /* 6.4 kbps */
        if (i_subfr == 0) {
            max  = interpol_3(&corr[lag], -2);
            frac = -2;
            
            for (i = -1; i <= 2; i++) {
                corr_int = interpol_3(&corr[lag], i);
                if(corr_int > max) {
                    max = corr_int;
                    frac = i;
                }
            }
        }
        else {
            midLag = t0_max - 4;
            if ((lag == midLag - 1) || lag == midLag) {
                max  = interpol_3(&corr[lag], -2);
                frac = -2;
                
                for (i = -1; i <= 2; i++) {
                    corr_int = interpol_3(&corr[lag], i);
                    if(corr_int > max) {
                        max = corr_int;
                        frac = i;
                    }
                }
            }
            else if (lag == midLag - 2) {
                max  = interpol_3(&corr[lag], 0);
                frac = 0;
                
                for (i = 1; i <= 2; i++) {
                    corr_int = interpol_3(&corr[lag], i);
                    if(corr_int > max) {
                        max = corr_int;
                        frac = i;
                    }
                }
            }      
            else if (lag == midLag + 1) {
                max  = interpol_3(&corr[lag], -2);
                frac = -2;
                
                for (i = -1; i <= 0; i++) {
                    corr_int = interpol_3(&corr[lag], i);
                    if(corr_int > max) {
                        max = corr_int;
                        frac = i;
                    }
                }
            }
            else
                frac = 0;
        }
        
    }
    else {
        max  = interpol_3(&corr[lag], -2);
        frac = -2;
        
        for (i = -1; i <= 2; i++)
        {
            corr_int = interpol_3(&corr[lag], i);
            if(corr_int > max)
            {
                max = corr_int;
                frac = i;
            }
        }
    }
    
    
    /* limit the fraction value in the interval [-1,0,1] */
    
    if (frac == -2)
    {
        frac = 1;
        lag -= 1;
    }
    if (frac == 2)
    {
        frac = -1;
        lag += 1;
    }
    
    *pit_frac = frac;
    
    return lag;
}

/*----------------------------------------------------------------------------
* norm_corr - Find the normalized correlation between the target vector and
*             the filtered past excitation.
*----------------------------------------------------------------------------
*/
static void norm_corr(
    FLOAT exc[],           /* input : excitation buffer */
    FLOAT xn[],            /* input : target vector */
    FLOAT h[],             /* input : imp response of synth and weighting flt */
    int l_subfr,           /* input : Length of frame to compute pitch */
    int t_min,             /* input : minimum value of searched range */
    int t_max,             /* input : maximum value of search range */
    FLOAT corr_norm[]      /* output: normalized correlation (correlation 
                                       between target and filtered excitation 
                                       divided by the square root of energy of 
                                       filtered excitation) */
)
{
    int    i, j, k;
    FLOAT excf[L_SUBFR];     /* filtered past excitation */
    FLOAT  alp, s, norm;
    
    k = -t_min;
    
    /* compute the filtered excitation for the first delay t_min */
    convolve(&exc[k], h, excf, l_subfr);
    
    /* loop for every possible period */
    for (i = t_min; i <= t_max; i++)
    {
        /* Compute 1/sqrt(energie of excf[]) */
        alp = (F)0.01;
        for (j = 0; j < l_subfr; j++)
            alp += excf[j]*excf[j];
        
        norm = inv_sqrt(alp);
        
        /* Compute correlation between xn[] and excf[] */
        s = (F)0.0;
        for (j = 0; j < l_subfr; j++)  s += xn[j]*excf[j];
        
        /* Normalize correlation = correlation * (1/sqrt(energie)) */
        corr_norm[i] = s*norm;
        
        /* modify the filtered excitation excf[] for the next iteration */
        if (i != t_max)
        {
            k--;
            for (j = l_subfr-1; j > 0; j--)
                excf[j] = excf[j-1] + exc[k]*h[j];
            excf[0] = exc[k];
        }
    }
    
    return;
    
}
/*----------------------------------------------------------------------------
* g_pitch - compute adaptive codebook gain and compute <y1,y1> , -2<xn,y1>
*----------------------------------------------------------------------------
*/
FLOAT g_pitch(          /* output: pitch gain */
    FLOAT xn[],            /* input : target vector */
    FLOAT y1[],            /* input : filtered adaptive codebook vector */
    FLOAT g_coeff[],       /* output: <y1,y1> and -2<xn,y1> */
    int l_subfr            /* input : vector dimension */
)
{
    FLOAT xy, yy, gain;
    int   i;
    
    xy = (F)0.0;
    for (i = 0; i < l_subfr; i++) {
        xy += xn[i] * y1[i];
    }
    yy = (F)0.01;
    for (i = 0; i < l_subfr; i++) {
        yy += y1[i] * y1[i];          /* energy of filtered excitation */
    }
    g_coeff[0] = yy;
    g_coeff[1] = (F)-2.0*xy +(F)0.01;
    
    /* find pitch gain and bound it by [0,1.2] */
    gain = xy/yy;
    
    if (gain<(F)0.0)  gain = (F)0.0;
    if (gain>GAIN_PIT_MAX) gain = GAIN_PIT_MAX;
    
    return gain;
}

/*----------------------------------------------------------------------*
*    Function enc_lag3cp()                                             *
*             ~~~~~~~~~~                                               *
*   Encoding of fractional pitch lag with 1/3 resolution.              *
*----------------------------------------------------------------------*
* The pitch range for the first subframe is divided as follows:        *
*   19 1/3  to   84 2/3   resolution 1/3                               *
*   85      to   143      resolution 1                                 *
*                                                                      *
* The period in the first subframe is encoded with 8 bits.             *
* For the range with fractions:                                        *
*   index = (T-19)*3 + frac - 1;   where T=[19..85] and frac=[-1,0,1]  *
* and for the integer only range                                       *
*   index = (T - 85) + 197;        where T=[86..143]                   *
*----------------------------------------------------------------------*
* For the second subframe a resolution of 1/3 is always used, and the  *
* search range is relative to the lag in the first subframe.           *
* If t0 is the lag in the first subframe then                          *
*  t_min=t0-5   and  t_max=t0+4   and  the range is given by           *
*       t_min - 2/3   to  t_max + 2/3                                  *
*                                                                      *
* The period in the 2nd subframe is encoded with 5 bits:               *
*   index = (T-(t_min-1))*3 + frac - 1;    where T[t_min-1 .. t_max+1] *
*----------------------------------------------------------------------*/
int  enc_lag3cp(    /* output: Return index of encoding */
    int  T0,         /* input : Pitch delay              */
    int  T0_frac,    /* input : Fractional pitch delay   */
    int  *T0_min,    /* in/out: Minimum search delay     */
    int  *T0_max,    /* in/out: Maximum search delay     */
    int  pit_min,    /* input : Minimum pitch delay      */
    int  pit_max,    /* input : Maximum pitch delay      */
    int  pit_flag,   /* input : Flag for 1st subframe    */
    int  rate
)
{
    int index;
    
    if (pit_flag == 0)   /* if 1st subframe */
    {
        /* encode pitch delay (with fraction) */
        
        if (T0 <= 85)
            index = T0*3 - 58 + T0_frac;
        else
            index = T0 + 112;
        
        /* find T0_min and T0_max for second subframe */
        
        *T0_min = T0 - 5;
        if (*T0_min < pit_min) *T0_min = pit_min;
        *T0_max = *T0_min + 9;
        if (*T0_max > pit_max)
        {
            *T0_max = pit_max;
            *T0_min = *T0_max - 9;
        }
    }
    
    else                    /* second subframe */
    {
        if (rate == G729D) {      /* 4 bits in 2nd subframe (6.4 kbps) */
            if (T0 < *T0_min + 3)
                index = T0 - *T0_min;
            else if (T0 < *T0_min + 7)
                index = (T0 - (*T0_min + 3)) * 3 + T0_frac + 3;
            else
                index = (T0 - (*T0_min + 7)) + 13;
        }
        else {
            index = T0 - *T0_min;
            index = index*3 + 2 + T0_frac;
        }
        
    }
    return index;
}

/*----------------------------------------------------------------------------
* interpol_3 - For interpolating the normalized correlation
*----------------------------------------------------------------------------
*/
static FLOAT interpol_3(   /* output: interpolated value */
    FLOAT *x,              /* input : function to be interpolated */
    int frac               /* input : fraction value to evaluate */
)
{
    int i;
    FLOAT s, *x1, *x2, *c1, *c2;
    
    if (frac < 0) {
        frac += UP_SAMP;
        x--;
    }
    x1 = &x[0];
    x2 = &x[1];
    c1 = &inter_3[frac];
    c2 = &inter_3[UP_SAMP-frac];
    
    s = (F)0.0;
    for(i=0; i< L_INTER4; i++, c1+=UP_SAMP, c2+=UP_SAMP)
        s+= (*x1--) * (*c1) + (*x2++) * (*c2);
    
    return s;
}

/*----------------------------------------------------------------------------
* inv_sqrt - compute y = 1 / sqrt(x)
*----------------------------------------------------------------------------
*/
static FLOAT inv_sqrt(         /* output: 1/sqrt(x) */
    FLOAT x                /* input : value of x */
)
{
    return ((F)1.0 / (FLOAT)sqrt((double)x) );
}
