/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
      ITU-T G.729 Annex H  - Reference C code for fixed point
                         implementation of G.729 Annex H
                         (integration of Annexes D and E)
                         Version 1.1 of October 1999
*/

/*
 File : pitchh.c
 */
/* from pitchd.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from pitch.c G.729 Version 3.3            */

/*---------------------------------------------------------------------------*
 * procedure Pitch_ol                                                        *
 * ~~~~~~~~~~~~~~~~~~                                                        *
 * Compute the open loop pitch lag.                                          *
 *                                                                           *
 *---------------------------------------------------------------------------*/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "ld8k.h"
#include "ld8h.h"
#include "tab_ld8k.h"

/* local function */

static Word16 Lag_max(        /* output: lag found                           */
  Word16 signal[],     /* input : signal used to compute the open loop pitch */
  Word16 L_frame,      /* input : length of frame to compute pitch           */
  Word16 lag_max,      /* input : maximum lag                                */
  Word16 lag_min,      /* input : minimum lag                                */
  Word16 *cor_max);    /* output: normalized correlation of selected lag     */


Word16 Pitch_ol(       /* output: open loop pitch lag                        */
   Word16 signal[],    /* input : signal used to compute the open loop pitch */
                       /*     signal[-pit_max] to signal[-1] should be known */
   Word16   pit_min,   /* input : minimum pitch lag                          */
   Word16   pit_max,   /* input : maximum pitch lag                          */
   Word16   L_frame    /* input : length of frame to compute pitch           */
)
{
    Word16  i, j;
    Word16  max1, max2, max3;
    Word16  p_max1, p_max2, p_max3;
    Word32  t0, L_temp;
    
    /* Scaled signal */
    
    Word16 scaled_signal[L_FRAME+PIT_MAX];
    Word16 *scal_sig;
    
    scal_sig = &scaled_signal[pit_max];
    
    /*--------------------------------------------------------*
    *  Verification for risk of overflow.                    *
    *--------------------------------------------------------*/
    
    Overflow = 0;
    t0 = 0;
    
    for(i= -pit_max; i< L_frame; i++)
        t0 = L_mac(t0, signal[i], signal[i]);
    
        /*--------------------------------------------------------*
        * Scaling of input signal.                               *
        *                                                        *
        *   if Overflow        -> scal_sig[i] = signal[i]>>3     *
        *   else if t0 < 1^20  -> scal_sig[i] = signal[i]<<3     *
        *   else               -> scal_sig[i] = signal[i]        *
    *--------------------------------------------------------*/
    
    if(Overflow == 1)
    {
        for(i=-pit_max; i<L_frame; i++)
            scal_sig[i] = shr(signal[i], 3);
    }
    else {
        L_temp = L_sub(t0, (Word32)1048576L);
        if ( L_temp < (Word32)0 )  /* if (t0 < 2^20) */
        {
            for(i=-pit_max; i<L_frame; i++)
                scal_sig[i] = shl(signal[i], 3);
        }
        else
        {
            for(i=-pit_max; i<L_frame; i++)
                scal_sig[i] = signal[i];
        }
    }
    /*--------------------------------------------------------------------*
    *  The pitch lag search is divided in three sections.                *
    *  Each section cannot have a pitch multiple.                        *
    *  We find a maximum for each section.                               *
    *  We compare the maximum of each section by favoring small lag.     *
    *                                                                    *
    *  First section:  lag delay = pit_max     downto 4*pit_min          *
    *  Second section: lag delay = 4*pit_min-1 downto 2*pit_min          *
    *  Third section:  lag delay = 2*pit_min-1 downto pit_min            *
    *--------------------------------------------------------------------*/
    
    
    j = shl(pit_min, 2);
    p_max1 = Lag_max(scal_sig, L_frame, pit_max, j, &max1);
    
    i = sub(j, 1); j = shl(pit_min, 1);
    p_max2 = Lag_max(scal_sig, L_frame, i, j, &max2);
    
    i = sub(j, 1);
    p_max3 = Lag_max(scal_sig, L_frame, i, pit_min , &max3);
    
    /*--------------------------------------------------------------------*
    * Compare the 3 sections maximum, and favor small lag.               *
    *--------------------------------------------------------------------*/
    
    if( sub(mult(max1, THRESHPIT), max2)  < 0)
    {
        max1 = max2;
        p_max1 = p_max2;
    }
    
    if( sub(mult(max1, THRESHPIT), max3)  < 0)
    {
        p_max1 = p_max3;
    }
    
    
    return (p_max1);
}

/*---------------------------------------------------------------------------*
* procedure Lag_max                                                         *
* ~~~~~~~~~~~~~~~~~                                                         *
* Find the lag that has maximum correlation with scal_sig[]                 *
*                                                                           *
*---------------------------------------------------------------------------*
* arguments:                                                                *
*                                                                           *
*   signal[]   :Signal used to compute the open loop pitch.                 *
*   L_frame    :Length of frame to compute pitch.                           *
*   lag_max    :Maximum lag                                                 *
*   lag_min    :Minimum lag                                                 *
*   *cor_max   ;Maximum of normalized correlation of lag found.             *
*                                                                           *
*   Return lag found.                                                       *
*--------------------------------------------------------------------------*/

static Word16 Lag_max( /* output: lag found                                  */
                      Word16 signal[],     /* input : signal used to compute the open loop pitch */
                      Word16 L_frame,      /* input : length of frame to compute pitch           */
                      Word16 lag_max,      /* input : maximum lag                                */
                      Word16 lag_min,      /* input : minimum lag                                */
                      Word16 *cor_max)     /* output: normalized correlation of selected lag     */
{
    Word16  i, j;
    Word16  *p, *p1;
    Word32  max, t0, L_temp;
    Word16  max_h, max_l, ener_h, ener_l;
    Word16  p_max;
    
    max = MIN_32;
    
    /* initialization used only to suppress Microsoft Visual C++  warnings */
    
    p_max = lag_max;
    
    for (i = lag_max; i >= lag_min; i--)
    {
        p  = signal;
        p1 = &signal[-i];
        t0 = 0;
        
        for (j=0; j<L_frame; j++, p++, p1++)
            t0 = L_mac(t0, *p, *p1);
        
        L_temp = L_sub(t0,max);
        if (L_temp >= 0L)
        {
            max    = t0;
            p_max = i;
        }
    }
    
    /* compute energy */
    
    t0 = 0;
    p = &signal[-p_max];
    for(i=0; i<L_frame; i++, p++)
        t0 = L_mac(t0, *p, *p);
    
    /* 1/sqrt(energy),    result in Q30 */
    
    t0 = Inv_sqrt(t0);
    
    /* max = max/sqrt(energy)                   */
    /* This result will always be on 16 bits !! */
    
    L_Extract(max, &max_h, &max_l);
    L_Extract(t0, &ener_h, &ener_l);
    
    t0 = Mpy_32(max_h, max_l, ener_h, ener_l);
    *cor_max = extract_l(t0);
    
    return(p_max);
}

/*--------------------------------------------------------------------------*
* Function  Pitch_fr3cp()                                                    *
* ~~~~~~~~~~~~~~~~~~~~~                                                    *
* Find the pitch period with 1/3 subsample resolution.                     *
*--------------------------------------------------------------------------*/

/* Local functions */

static void Norm_Corr(Word16 exc[], Word16 xn[], Word16 h[], Word16 L_subfr,
                      Word16 t_min, Word16 t_max, Word16 corr_norm[]);


Word16 Pitch_fr3cp(    /* (o)     : pitch period.                          */
                   Word16 exc[],      /* (i)     : excitation buffer                      */
                   Word16 xn[],       /* (i)     : target vector                          */
                   Word16 h[],        /* (i) Q12 : impulse response of filters.           */
                   Word16 L_subfr,    /* (i)     : Length of subframe                     */
                   Word16 t0_min,     /* (i)     : minimum value in the searched range.   */
                   Word16 t0_max,     /* (i)     : maximum value in the searched range.   */
                   Word16 i_subfr,    /* (i)     : indicator for first subframe.          */
                   Word16 *pit_frac,   /* (o)     : chosen fraction.                       */
                   Word16 rate        /* (i)     : frame rate*/
                   )
{
    Word16 i;
    Word16 t_min, t_max;
    Word16 max, lag, frac;
    Word16 *corr;
    Word16 corr_int;
    Word16 corr_v[40];           /* Total length = t0_max-t0_min+1+2*L_INTER */
    Word16 midLag, tmpLag;
    
    /* Find interval to compute normalized correlation */
    
    t_min = sub(t0_min, L_INTER4);
    t_max = add(t0_max, L_INTER4);
    
    corr = &corr_v[-t_min];
    
    /* Compute normalized correlation between target and filtered excitation */
    
    Norm_Corr(exc, xn, h, L_subfr, t_min, t_max, corr);
    
    /* Find integer pitch */
    
    max = corr[t0_min];
    lag = t0_min;
    
    for(i= t0_min+(Word16)1; i<=t0_max; i++)
    {
        if( sub(corr[i], max) >= 0)
        {
            max = corr[i];
            lag = i;
        }
    }
    
    /* If first subframe and lag > 84 do not search fractional pitch */
    
    if( (i_subfr == 0) && (sub(lag, 84) > 0) )
    {
        *pit_frac = 0;
        return(lag);
    }
    
    /* Test the fractions around T0 and choose the one which maximizes   */
    /* the interpolated normalized correlation.                          */
    if (rate == G729D) {    /* 6.4 kbps */
        /* 6.4kbps (4 bits delta lag) */
        if (i_subfr == 0) {
            max  = Interpol_3(&corr[lag], -2);
            frac = -2;
            
            for (i = -1; i <= 2; i++)
            {
                corr_int = Interpol_3(&corr[lag], i);
                if (sub(corr_int, max) > 0)
                {
                    max = corr_int;
                    frac = i;
                }
            }
        }
        else {
            midLag = sub(t0_max, 4);
            tmpLag = sub(lag, midLag);
            if ((add(tmpLag, 1) == 0) || sub(lag, midLag) == 0) {
                max  = Interpol_3(&corr[lag], -2);
                frac = -2;
                
                for (i = -1; i <= 2; i++) {
                    corr_int = Interpol_3(&corr[lag], i);
                    if(sub(corr_int, max) > 0) {
                        max = corr_int;
                        frac = i;
                    }
                }
            }
            else if (add(tmpLag, 2) == 0) {
                max  = Interpol_3(&corr[lag], 0);
                frac = 0;
                
                for (i = 1; i <= 2; i++) {
                    corr_int = Interpol_3(&corr[lag], i);
                    if(sub(corr_int, max) > 0) {
                        max = corr_int;
                        frac = i;
                    }
                }
            }
            else if (sub(tmpLag, 1) == 0) {
                max  = Interpol_3(&corr[lag], -2);
                frac = -2;
                
                for (i = -1; i <= 0; i++) {
                    corr_int = Interpol_3(&corr[lag], i);
                    if(sub(corr_int, max) > 0) {
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
        max  = Interpol_3(&corr[lag], -2);
        frac = -2;
        
        for (i = -1; i <= 2; i++) {
            corr_int = Interpol_3(&corr[lag], i);
            if (sub(corr_int, max) > 0) {
                max = corr_int;
                frac = i;
            }
        }
    }
    /* limit the fraction value between -1 and 1 */
    if (sub(frac, -2) == 0) {
        frac = 1;
        lag = sub(lag, 1);
    }
    if (sub(frac, 2) == 0) {
        frac = -1;
        lag = add(lag, 1);
    }
    
    *pit_frac = frac;
    
    return(lag);
}

/*---------------------------------------------------------------------------*
* Function Norm_Corr()                                                      *
* ~~~~~~~~~~~~~~~~~~~~                                                      *
* Find the normalized correlation between the target vector and the         *
* filtered past excitation.                                                 *
*---------------------------------------------------------------------------*
* Input arguments:                                                          *
*     exc[]    : excitation buffer                                          *
*     xn[]     : target vector                                              *
*     h[]      : impulse response of synthesis and weighting filters (Q12)  *
*     L_subfr  : Length of subframe                                         *
*     t_min    : minimum value of pitch lag.                                *
*     t_max    : maximum value of pitch lag.                                *
*                                                                           *
* Output arguments:                                                         *
*     corr_norm[]:  normalized correlation (correlation between target and  *
*                   filtered excitation divided by the square root of       *
*                   energy of filtered excitation)                          *
*--------------------------------------------------------------------------*/

static void Norm_Corr(Word16 exc[], Word16 xn[], Word16 h[], Word16 L_subfr,
                      Word16 t_min, Word16 t_max, Word16 corr_norm[])
{
    Word16 i,j,k;
    Word16 corr_h, corr_l, norm_h, norm_l;
    Word32 s, L_temp;
    
    Word16 excf[L_SUBFR];
    Word16 scaling, h_fac, *s_excf, scaled_excf[L_SUBFR];
    
    
    k =  negate(t_min);
    
    /* compute the filtered excitation for the first delay t_min */
    
    Convolve(&exc[k], h, excf, L_subfr);
    
    /* scaled "excf[]" to avoid overflow */
    
    for(j=0; j<L_subfr; j++)
        scaled_excf[j] = shr(excf[j], 2);
    
    /* Compute energy of excf[] for danger of overflow */
    
    s = 0;
    for (j = 0; j < L_subfr; j++)
        s = L_mac(s, excf[j], excf[j]);
    
    L_temp = L_sub(s, 67108864L);
    if (L_temp <= 0L)      /* if (s <= 2^26) */
    {
        s_excf = excf;
        h_fac = 15-12;               /* h in Q12 */
        scaling = 0;
    }
    else {
        s_excf = scaled_excf;        /* "excf[]" is divide by 2 */
        h_fac = 15-12-2;             /* h in Q12, divide by 2 */
        scaling = 2;
    }
    
    /* loop for every possible period */
    
    for (i = t_min; i <= t_max; i++)
    {
        /* Compute 1/sqrt(energy of excf[]) */
        
        s = 0;
        for (j = 0; j < L_subfr; j++)
            s = L_mac(s, s_excf[j], s_excf[j]);
        
        s = Inv_sqrt(s);                     /* Result in Q30 */
        L_Extract(s, &norm_h, &norm_l);
        
        /* Compute correlation between xn[] and excf[] */
        
        s = 0;
        for (j = 0; j < L_subfr; j++)
            s = L_mac(s, xn[j], s_excf[j]);
        
        L_Extract(s, &corr_h, &corr_l);
        
        /* Normalize correlation = correlation * (1/sqrt(energy)) */
        
        s = Mpy_32(corr_h, corr_l, norm_h, norm_l);
        
        corr_norm[i] = extract_h(L_shl(s, 16));   /* Result is on 16 bits */
        
        /* modify the filtered excitation excf[] for the next iteration */
        
        if( sub(i, t_max) != 0)
        {
            k=sub(k,1);
            for (j = L_subfr-(Word16)1; j > 0; j--)
            {
                s = L_mult(exc[k], h[j]);
                s = L_shl(s, h_fac);             /* h is in Q(12-scaling) */
                s_excf[j] = add(extract_h(s), s_excf[j-1]);
            }
            s_excf[0] = shr(exc[k], scaling);
        }
    }
    return;
}

/*---------------------------------------------------------------------*
* Function  G_pitch:                                                  *
*           ~~~~~~~~                                                  *
*---------------------------------------------------------------------*
* Compute correlations <xn,y1> and <y1,y1> to use in gains quantizer. *
* Also compute the gain of pitch. Result in Q14                       *
*  if (gain < 0)  gain =0                                             *
*  if (gain >1.2) gain =1.2                                           *
*---------------------------------------------------------------------*/


Word16 G_pitch(      /* (o) Q14 : Gain of pitch lag saturated to 1.2       */
               Word16 xn[],       /* (i)     : Pitch target.                            */
               Word16 y1[],       /* (i)     : Filtered adaptive codebook.              */
               Word16 g_coeff[],  /* (i)     : Correlations need for gain quantization. */
               Word16 L_subfr     /* (i)     : Length of subframe.                      */
               )
{
    Word16 i;
    Word16 xy, yy, exp_xy, exp_yy, gain;
    Word32 s;
    
    Word16 scaled_y1[L_SUBFR];
    
    /* divide "y1[]" by 4 to avoid overflow */
    
    for(i=0; i<L_subfr; i++)
        scaled_y1[i] = shr(y1[i], 2);
    
    /* Compute scalar product <y1[],y1[]> */
    
    Overflow = 0;
    s = 1;                    /* Avoid case of all zeros */
    for(i=0; i<L_subfr; i++)
        s = L_mac(s, y1[i], y1[i]);
    
    if (Overflow == 0) {
        exp_yy = norm_l(s);
        yy     = round( L_shl(s, exp_yy) );
    }
    else {
        s = 1;                  /* Avoid case of all zeros */
        for(i=0; i<L_subfr; i++)
            s = L_mac(s, scaled_y1[i], scaled_y1[i]);
        exp_yy = norm_l(s);
        yy     = round( L_shl(s, exp_yy) );
        exp_yy = sub(exp_yy, 4);
    }
    
    /* Compute scalar product <xn[],y1[]> */
    
    Overflow = 0;
    s = 0;
    for(i=0; i<L_subfr; i++)
        s = L_mac(s, xn[i], y1[i]);
    
    if (Overflow == 0) {
        exp_xy = norm_l(s);
        xy     = round( L_shl(s, exp_xy) );
    }
    else {
        s = 0;
        for(i=0; i<L_subfr; i++)
            s = L_mac(s, xn[i], scaled_y1[i]);
        exp_xy = norm_l(s);
        xy     = round( L_shl(s, exp_xy) );
        exp_xy = sub(exp_xy, 2);
    }
    
    g_coeff[0] = yy;
    g_coeff[1] = sub(15, exp_yy);
    g_coeff[2] = xy;
    g_coeff[3] = sub(15, exp_xy);
    
    /* If (xy <= 0) gain = 0 */
    
    
    if (xy <= 0)
    {
        g_coeff[3] = -15;   /* Force exp_xy to -15 = (15-30) */
        return( (Word16) 0);
    }
    
    /* compute gain = xy/yy */
    
    xy = shr(xy, 1);             /* Be sure xy < yy */
    gain = div_s( xy, yy);
    
    i = sub(exp_xy, exp_yy);
    gain = shr(gain, i);         /* saturation if > 1.99 in Q14 */
    
    /* if(gain >1.2) gain = 1.2  in Q14 */
    
    if( sub(gain, 19661) > 0)
    {
        gain = 19661;
    }
    
    
    return(gain);
}

/*----------------------------------------------------------------------*
*    Function Enc_lag3                                                 *
*             ~~~~~~~~                                                 *
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


Word16 Enc_lag3cp(     /* output: Return index of encoding */
                  Word16 T0,         /* input : Pitch delay              */
                  Word16 T0_frac,    /* input : Fractional pitch delay   */
                  Word16 *T0_min,    /* in/out: Minimum search delay     */
                  Word16 *T0_max,    /* in/out: Maximum search delay     */
                  Word16 pit_min,    /* input : Minimum pitch delay      */
                  Word16 pit_max,    /* input : Maximum pitch delay      */
                  Word16 pit_flag,    /* input : Flag for 1st subframe    */
                  Word16 rate           /* input : frame rate */
                  )
{
    Word16 index, i;
    
    if (pit_flag == 0)   /* if 1st subframe */
    {
        /* encode pitch delay (with fraction) */
        
        if (sub(T0, 85) <= 0)
        {
            /* index = t0*3 - 58 + t0_frac   */
            i = add(add(T0, T0), T0);
            index = add(sub(i, 58), T0_frac);
        }
        else {
            index = add(T0, 112);
        }
        
        /* find T0_min and T0_max for second (or fourth) subframe */
        
        *T0_min = sub(T0, 5);
        if (sub(*T0_min, pit_min) < 0)
        {
            *T0_min = pit_min;
        }
        
        *T0_max = add(*T0_min, 9);
        if (sub(*T0_max, pit_max) > 0)
        {
            *T0_max = pit_max;
            *T0_min = sub(*T0_max, 9);
        }
    }
    else      /* if second subframe */
    {
        if (rate == G729D) {      /* 4 bits in 2nd subframe (6.4 kbps) */
            i = sub(T0, *T0_min);
            
            if (sub(i, 3) < 0)
                index = i;
            else if (sub(i, 7) < 0) {
                
                i = sub(i, 3);
                i = add(i, add(i, i));
                index = add(i, add(T0_frac, 3));
            }
            else {
                
                index = add(i, 6);
            }
        }
        else {
            /* i = t0 - t0_min;               */
            /* index = i*3 + 2 + t0_frac;     */
            i = sub(T0, *T0_min);
            i = add(add(i, i), i);
            index = add(add(i, 2), T0_frac);
        }
    }
    
    return index;
}


/*---------------------------------------------------------------------------*
* Procedure Interpol_3()                                                    *
* ~~~~~~~~~~~~~~~~~~~~~~                                                    *
* For interpolating the normalized correlation with 1/3 resolution.         *
*--------------------------------------------------------------------------*/
Word16 Interpol_3(      /* (o)  : interpolated value  */
                  Word16 *x,            /* (i)  : input vector        */
                  Word16 frac           /* (i)  : fraction            */
                  )
{
    Word16 i, k;
    Word16 *x1, *x2, *c1, *c2;
    Word32 s;
    
    if(frac < 0)
    {
        frac = add(frac, UP_SAMP);
        x--;
    }
    
    x1 = &x[0];
    x2 = &x[1];
    c1 = &inter_3[frac];
    c2 = &inter_3[sub(UP_SAMP,frac)];
    
    s = 0;
    for(i=0, k=0; i< L_INTER4; i++, k+=UP_SAMP)
    {
        s = L_mac(s, x1[-i], c1[k]);
        s = L_mac(s, x2[i],  c2[k]);
    }
    
    
    return round(s);
}
