/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
 ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
                         Version 1.1 of October 1999
   
*/

/*
 File : acelp_cp.c
*/

/* from acelp_e.c G.729 Annex E Version 1.2  Last modified: May 1998 */
/* from acelpcod.c G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from acelp_co.c G.729 Version 3.3            */

#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

/* locals functions */
static void Cor_h_cp(
     Word16 *H,         /* (i) Q12 :Impulse response of filters */
     Word16 *rr,         /* (o)     :Correlations of H[]         */
     Word16 rate        /* (i) frame rate */
);
static void Cor_h_X(
     Word16 h[],        /* (i) Q12 :Impulse response of filters      */
     Word16 X[],        /* (i) Q12 :Target vector                    */
     Word16 D[]         /* (o)     :Correlations between h[] and D[] */
                        /*          Normalized to 13 bits            */
);
static Word16 D4i40_17(        /* (o)    : Index of pulses positions.               */
    Word16 Dn[],          /* (i)    : Correlations between h[] and Xn[].       */
    Word16 rr[],          /* (i)    : Correlations of impulse response h[].    */
    Word16 h[],           /* (i) Q12: Impulse response of filters.             */
    Word16 cod[],         /* (o) Q13: Selected algebraic codeword.             */
    Word16 y[],           /* (o) Q12: Filtered algebraic codeword.             */
    Word16 *sign,         /* (o)    : Signs of 4 pulses.                       */
    Word16 i_subfr        /* (i)    : subframe flag                            */
);
static Word16 D2i40_11(        /* (o)    : Index of pulses positions.               */
    Word16 Dn[],          /* (i)    : Correlations between h[] and Xn[].       */
    Word16 rr[],          /* (i)    : Correlations of impulse response h[].    */
    Word16 h[],           /* (i) Q12: Impulse response of filters.             */
    Word16 code[],        /* (o) Q13: Selected algebraic codeword.             */
    Word16 y[],           /* (o) Q12: Filtered algebraic codeword.             */
    Word16 *sign         /* (o)    : Signs of 4 pulses.                       */
);
static void cor_h_x_e(
    Word16 h[],    /* (i) Q12 : impulse response of weighted synthesis filter */
    Word16 x[],    /* (i) Q0  : correlation between target and h[]            */
    Word16 dn[]    /* (o) Q0  : correlation between target and h[]            */
);
static void cor_h_vec(
    Word16 h[],           /* (i) scaled impulse response */
    Word16 vec[],         /* (i) vector to correlate with h[] */
    Word16 track,         /* (i) track to use */
    Word16 sign[],        /* (i) sign vector */
    Word16 rrixix[][NB_POS],  /* (i) correlation of h[x] with h[x] */
    Word16 cor[]          /* (o) result of correlation (NB_POS elements) */
);

static void search_ixiy(
    Word16 track_x,       /* (i) track of pulse 1 */
    Word16 track_y,       /* (i) track of pulse 2 */
    Word16 *ps,           /* (i/o) correlation of all fixed pulses */
    Word16 *alp,          /* (i/o) energy of all fixed pulses */
    Word16 *ix,           /* (o) position of pulse 1 */
    Word16 *iy,           /* (o) position of pulse 2 */
    Word16 dn[],          /* (i) corr. between target and h[] */
    Word16 cor_x[],       /* (i) corr. of pulse 1 with fixed pulses */
    Word16 cor_y[],       /* (i) corr. of pulse 2 with fixed pulses */
    Word16 rrixiy[][MSIZE]  /* (i) corr. of pulse 1 with pulse 2 */
);

static void set_sign(
    Word16 fac_cn,     /* (i) Q15: residual weight for sign determination */
    Word16 cn[],       /* (i) Q0 : residual after long term prediction    */
    Word16 dn[],       /* (i) Q0 : correlation between target and h[]     */
    Word16 sign[],     /* (o) Q15: sign vector (sign of each position)    */
    Word16 inv_sign[], /* (o) Q15: inverse of sign[]                      */
    Word16 pos_max[],  /* (o)    : pos of max of correlation              */
    Word32 corr[]      /* (o)    : correlation of each track              */
);

static void cor_h_e(
    Word16 H[],              /* (i) Q12 :Impulse response of filters */
    Word16 sign[],           /* (i) Q15: sign vector                 */
    Word16 inv_sign[],       /* (i) Q15: inverse of sign[]           */
    Word16 h[],              /* (o)     : scaled h[]                 */
    Word16 h_inv[],          /* (o)     : inverse of scaled h[]      */
    Word16 rrixix[][NB_POS], /* (o) energy of h[].                   */
    Word16 rrixiy[][MSIZE]   /* (o) correlation between 2 pulses.    */
);

static void build_code(
    Word16 codvec[],      /* (i)    : positions of each pulse */
    Word16 sign[],        /* (i) Q15: sign vector             */
    Word16 nb_of_pulse,   /* (i)    : number of pulses        */
    Word16 H[],    /* (i) Q12: impulse response of weighted synthesis filter */
    Word16 code[], /* (o) Q12: algebraic (fixed) codebook excitation         */
    Word16 y[],    /* (o) Q11: filtered fixed codebook excitation            */
    Word16 indx[]  /* (o)    : index of pulses (5 words, 1 per track).       */
);

static Word16 pack3(Word16 index1, Word16 index2, Word16 index3);
Word16  ACELP_Codebook(  /* (o)     :index of pulses positions    */
    Word16 x[],            /* (i)     :Target vector                */
    Word16 h[],            /* (i) Q12 :Impulse response of filters  */
    Word16 T0,             /* (i)     :Pitch lag                    */
    Word16 pitch_sharp,    /* (i) Q14 :Last quantized pitch gain    */
    Word16 i_subfr,        /* (i)     :Indicator of 1st subframe,   */
    Word16 code[],         /* (o) Q13 :Innovative codebook          */
    Word16 y[],            /* (o) Q12 :Filtered innovative codebook */
    Word16 *sign           /* (o)     :Signs of 4 pulses            */
)
{
    Word16 i, index, sharp;
    Word16 Dn[L_SUBFR];
    Word16 rr[DIM_RR];
    
    /*-----------------------------------------------------------------*
    * Include fixed-gain pitch contribution into impulse resp. h[]    *
    * Find correlations of h[] needed for the codebook search.        *
    *-----------------------------------------------------------------*/
    
    sharp = shl(pitch_sharp, 1);          /* From Q14 to Q15 */
    if (sub(T0, L_SUBFR)<0)
        for (i = T0; i < L_SUBFR; i++){    /* h[i] += pitch_sharp*h[i-T0] */
            h[i] = add(h[i], mult(h[i-T0], sharp));
        }
        
    Cor_h_cp(h, rr, G729);
    
    /*-----------------------------------------------------------------*
    * Compute correlation of target vector with impulse response.     *
    *-----------------------------------------------------------------*/
    
    Cor_h_X(h, x, Dn);
    
    /*-----------------------------------------------------------------*
    * Find innovative codebook.                                       *
    *-----------------------------------------------------------------*/
    
    index = D4i40_17(Dn, rr, h, code, y, sign, i_subfr);
    
    /*-----------------------------------------------------------------*
    * Compute innovation vector gain.                                 *
    * Include fixed-gain pitch contribution into code[].              *
    *-----------------------------------------------------------------*/
    
    if(sub(T0 ,L_SUBFR) <0)
        for (i = T0; i < L_SUBFR; i++) {  /* code[i] += pitch_sharp*code[i-T0] */
                code[i] = add(code[i], mult(code[i-T0], sharp));
        }
            
    return index;
}
Word16  ACELP_Codebook64(  /* (o)     :index of pulses positions    */
                         Word16 x[],            /* (i)     :Target vector                */
                         Word16 h[],            /* (i) Q12 :Impulse response of filters  */
                         Word16 T0,             /* (i)     :Pitch lag                    */
                         Word16 pitch_sharp,    /* (i) Q14 :Last quantized pitch gain    */
                         Word16 code[],         /* (o) Q13 :Innovative codebook          */
                         Word16 y[],            /* (o) Q12 :Filtered innovative codebook */
                         Word16 *sign           /* (o)     :Signs of 4 pulses            */
                         )
{
    Word16 i, index, sharp;
    Word16 Dn[L_SUBFR];
    Word16 rr[DIM_RR];
    
    /*-----------------------------------------------------------------*
    * Include fixed-gain pitch contribution into impulse resp. h[]    *
    * Find correlations of h[] needed for the codebook search.        *
    *-----------------------------------------------------------------*/
    
    sharp = shl(pitch_sharp, 1);          /* From Q14 to Q15 */
    if (sub(T0, L_SUBFR)<0)
        for (i = T0; i < L_SUBFR; i++){    /* h[i] += pitch_sharp*h[i-T0] */
            h[i] = add(h[i], mult(h[i-T0], sharp));
        }
        
    Cor_h_cp(h, rr, G729D);
    
    /*-----------------------------------------------------------------*
    * Compute correlation of target vector with impulse response.     *
    *-----------------------------------------------------------------*/
    Cor_h_X(h, x, Dn);
    
    /*-----------------------------------------------------------------*
    * Find innovative codebook.                                       *
    *-----------------------------------------------------------------*/
    index = D2i40_11(Dn, rr, h, code, y, sign);
    
    /*-----------------------------------------------------------------*
    * Compute innovation vector gain.                                 *
    * Include fixed-gain pitch contribution into code[].              *
    *-----------------------------------------------------------------*/
    if(sub(T0 ,L_SUBFR) <0)
        for (i = T0; i < L_SUBFR; i++) {  /* code[i] += pitch_sharp*code[i-T0] */
            code[i] = add(code[i], mult(code[i-T0], sharp));
        }
            
    return index;
}

/*--------------------------------------------------------------------------*
 *  Function  Corr_h_X()                                                    *
 *  ~~~~~~~~~~~~~~~~~~~~                                                    *
 * Compute  correlations of input response h[] with the target vector X[].  *
 *--------------------------------------------------------------------------*/

static void Cor_h_X(
     Word16 h[],        /* (i) Q12 :Impulse response of filters      */
     Word16 X[],        /* (i)     :Target vector                    */
     Word16 D[]         /* (o)     :Correlations between h[] and D[] */
                        /*          Normalized to 13 bits            */
)
{
    Word16 i, j;
    Word32 s, max, L_temp;
    Word32 y32[L_SUBFR];
    
    /* first keep the result on 32 bits and find absolute maximum */
    
    max = 0;
    
    for (i = 0; i < L_SUBFR; i++)
    {
        s = 0;
        for (j = i; j <  L_SUBFR; j++)
            s = L_mac(s, X[j], h[j-i]);
        
        y32[i] = s;
        
        s = L_abs(s);
        L_temp =L_sub(s,max);
        if(L_temp>0L) {
            max = s;
        }
    }
    
    /* Find the number of right shifts to do on y32[]  */
    /* so that maximum is on 13 bits                   */
    
    j = norm_l(max);
    if( sub(j,16) > 0) {
        j = 16;
    }
    
    j = sub(18, j);
    
    for(i=0; i<L_SUBFR; i++) {
        D[i] = extract_l( L_shr(y32[i], j) );
    }
    
    return;
    
}

/*--------------------------------------------------------------------------*
 *  Function  Cor_h_cp()                                                    *
 *  ~~~~~~~~~~~~~~~~~                                                       *
 * Compute  correlations of h[]  needed for the codebook search.            *
 *--------------------------------------------------------------------------*/

static void Cor_h_cp(
     Word16 *H,         /* (i) Q12 :Impulse response of filters */
     Word16 *rr,         /* (o)     :Correlations of H[]         */
     Word16 rate        /* (i) frame rate */
)
{
    Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
    Word16 *rri0i1, *rri0i2, *rri0i3, *rri0i4;
    Word16 *rri1i2, *rri1i3, *rri1i4;
    Word16 *rri2i3, *rri2i4;
    
    Word16 *p0, *p1, *p2, *p3, *p4;
    
    Word16 *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
    Word32 cor;
    Word16 i, k, ldec, l_fin_sup, l_fin_inf;
    Word16 h[L_SUBFR];
    Word32 L_tmp;
    Word16 lsym;
    
    /* Scaling h[] for maximum precision */
    
    cor = 0;
    for(i=0; i<L_SUBFR; i++)
        cor = L_mac(cor, H[i], H[i]);
    
    L_tmp = L_sub(extract_h(cor),32000);
    if(L_tmp>0L )
    {
        for(i=0; i<L_SUBFR; i++) {
            h[i] = shr(H[i], 1);}
    }
    else
    {
        k = norm_l(cor);
        k = shr(k, 1);
        
        for(i=0; i<L_SUBFR; i++) {
            h[i] = shl(H[i], k); }
    }
    
    /*-----------------------------------------------------------------*
    * In case of G729 mode, nine cross correlations has to be         *
    * calculated, namely the following:                               *
    *                                                                 *
    * rri0i1[],                                                       *
    * rri0i2[],   rri1i2[],                                           *
    * rri0i3[],   rri1i3[],  rri2i3[],                                *
    * rri0i4[],   rri1i4[],  rri2i4[],                                *
    *                                                                 *
    * In case of G729 on 6.4 kbps mode, three of the above nine cross *
    * correlations are not needed for the codebook search, namely     *
    * rri0i2[], rri0i4[] and rri2i4[]. Two of these three 64-element  *
    * positions are instead used by two cross correlations needed     *
    * only by the 6.4 kbps mode (see D2i40_11() for details).         *
    *-----------------------------------------------------------------*/
    
    /* Init pointers */
    
    rri0i0 = rr;
    rri1i1 = rri0i0 + NB_POS;
    rri2i2 = rri1i1 + NB_POS;
    rri3i3 = rri2i2 + NB_POS;
    rri4i4 = rri3i3 + NB_POS;
    
    rri0i1 = rri4i4 + NB_POS;
    rri0i2 = rri0i1 + MSIZE;   /* Holds RRi1i1[] in 6.4 kbps mode */
    rri0i3 = rri0i2 + MSIZE;
    rri0i4 = rri0i3 + MSIZE;   /* Holds RRi3i4[] in 6.4 kbps mode */
    rri1i2 = rri0i4 + MSIZE;
    rri1i3 = rri1i2 + MSIZE;
    rri1i4 = rri1i3 + MSIZE;
    rri2i3 = rri1i4 + MSIZE;
    rri2i4 = rri2i3 + MSIZE;   /* Not used in 6.4 kbps mode */
    
                               /*------------------------------------------------------------*
                               * Compute rri0i0[], rri1i1[], rri2i2[], rri3i3 and rri4i4[]  *
    *------------------------------------------------------------*/
    
    p0 = rri0i0 + NB_POS-1;   /* Init pointers to last position of rrixix[] */
    p1 = rri1i1 + NB_POS-1;
    p2 = rri2i2 + NB_POS-1;
    p3 = rri3i3 + NB_POS-1;
    p4 = rri4i4 + NB_POS-1;
    
    ptr_h1 = h;
    cor    = 0;
    for(i=0;  i<NB_POS; i++)
    {
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p4-- = extract_h(cor);
        
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p3-- = extract_h(cor);
        
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p2-- = extract_h(cor);
        
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p1-- = extract_h(cor);
        
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p0-- = extract_h(cor);
    }
    
    /*-----------------------------------------------------------------*
    * Compute elements of: rri2i3[], rri1i2[], rri0i1[] and rri0i4[]  *
    *-----------------------------------------------------------------*/
    
    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Word16)1;
    ldec = NB_POS+1;
    
    ptr_hd = h;
    ptr_hf = ptr_hd + 1;
    
    for(k=0; k<NB_POS; k++) {
        
        p4 = rri0i4 + l_fin_sup;
        p3 = rri2i3 + l_fin_sup;
        p2 = rri1i2 + l_fin_sup;
        p1 = rri0i1 + l_fin_sup;
        p0 = rri0i4 + l_fin_inf;
        cor = 0;
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        
        for(i=k+(Word16)1; i<NB_POS; i++ ) {
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            if (sub(rate, G729D) == 0) *p4 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            if (sub(rate, G729) == 0) *p0 = extract_h(cor);
            
            p4 -= ldec;
            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        if (sub(rate, G729D) == 0) *p4 = extract_h(cor);
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);
        
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p2 = extract_h(cor);
        
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p1 = extract_h(cor);
        
        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }
    
    /*---------------------------------------------------------------------*
    * Compute elements of: rri2i4[], rri1i3[], rri0i2[], rri1i4[], rri0i3 *
    *---------------------------------------------------------------------*/
    
    ptr_hd = h;
    ptr_hf = ptr_hd + 2;
    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Word16)1;
    for(k=0; k<NB_POS; k++) {
        
        p4 = rri2i4 + l_fin_sup;
        p3 = rri1i3 + l_fin_sup;
        p2 = rri0i2 + l_fin_sup;
        p1 = rri1i4 + l_fin_inf;
        p0 = rri0i3 + l_fin_inf;
        
        cor = 0;
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        for(i=k+(Word16)1; i<NB_POS; i++ ) {
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p4 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);
            
            p4 -= ldec;
            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p4 = extract_h(cor);
        
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);
        
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p2 = extract_h(cor);
        
        
        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }
    
    /*----------------------------------------------------------------------*
    * Compute elements of: rri1i4[], rri0i3[], rri2i4[], rri1i3[], rri0i2  *
    *----------------------------------------------------------------------*/
    
    ptr_hd = h;
    ptr_hf = ptr_hd + 3;
    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Word16)1;
    for(k=0; k<NB_POS; k++) {
        
        p4 = rri1i4 + l_fin_sup;
        p3 = rri0i3 + l_fin_sup;
        p2 = rri2i4 + l_fin_inf;
        p1 = rri1i3 + l_fin_inf;
        p0 = rri0i2 + l_fin_inf;
        
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        cor = 0;
        for(i=k+(Word16)1; i<NB_POS; i++ ) {
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p4 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);
            
            p4 -= ldec;
            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p4 = extract_h(cor);
        
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);
        
        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }
    
    /*----------------------------------------------------------------------*
    * Compute elements of: rri0i4[], rri2i3[], rri1i2[], rri0i1[]          *
    *----------------------------------------------------------------------*/
    
    ptr_hd = h;
    ptr_hf = ptr_hd + 4;
    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Word16)1;
    for(k=0; k<NB_POS; k++) {
        
        if (sub(rate, G729) == 0)
            p3 = rri0i4 + l_fin_sup;
        if (sub(rate, G729D) == 0)
            p3 = rri0i4 + l_fin_inf;
        p2 = rri2i3 + l_fin_inf;
        p1 = rri1i2 + l_fin_inf;
        p0 = rri0i1 + l_fin_inf;
        
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        cor = 0;
        for(i=k+(Word16)1; i<NB_POS; i++ ) {
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            if (sub(rate, G729) == 0) *p3 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            if (sub(rate, G729D) == 0) *p3 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);
            
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);
            
            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        if (sub(rate, G729) == 0) *p3 = extract_h(cor);
        
        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }
    
    if (sub(rate, G729D) == 0) {
    /*-----------------------------------------------------------------*
    * Compute elements of RRi1i1[]                                    *
    *-----------------------------------------------------------------*/
        
        p0 = rri0i2;
        for (k=0; k<NB_POS; k++) {
            *p0 = *rri1i1;
            rri1i1++;
            p0 += ldec;
        }
        
        ptr_hd = h;
        ptr_hf = ptr_hd + 5;
        l_fin_sup = MSIZE-1;
        l_fin_inf = l_fin_sup- (Word16)NB_POS;
        lsym = NB_POS - (Word16)1;
        for(k=(Word16)1; k<NB_POS; k++) {
            
            p0 = rri0i2 + l_fin_inf;
            ptr_h1 = ptr_hd;
            ptr_h2 =  ptr_hf;
            cor = 0;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);
            *(p0+lsym) = extract_h(cor);
            p0 -= ldec;
            for(i=k+(Word16)1; i<NB_POS; i++ ) {
                
                cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                *p0 = extract_h(cor);
                *(p0+lsym) = extract_h(cor);
                
                p0 -= ldec;
            }
            
            l_fin_inf -= NB_POS;
            ptr_hf += STEP;
            lsym += NB_POS - (Word16)1;
        }
        
    }
    
    return;
}

/*------------------------------------------------------------------------*
 * Function  D4i40_17                                                     *
 *           ~~~~~~~~~                                                    *
 * Algebraic codebook for ITU 8 kb/s.                                      *
 *  -> 17 bits; 4 pulses in a frame of 40 samples                         *
 *                                                                        *
 *------------------------------------------------------------------------*
 * The code length is 40, containing 4 nonzero pulses i0, i1, i2, i3.     *
 * Each pulses can have 8 possible positions (positive or negative)       *
 * except i3 that have 16 possible positions.                             *
 *                                                                        *
 * i0 (+-1) : 0, 5, 10, 15, 20, 25, 30, 35                                *
 * i1 (+-1) : 1, 6, 11, 16, 21, 26, 31, 36                                *
 * i2 (+-1) : 2, 7, 12, 17, 22, 27, 32, 37                                *
 * i3 (+-1) : 3, 8, 13, 18, 23, 28, 33, 38                                *
 *            4, 9, 14, 19, 24, 29, 34, 39                                *
 *------------------------------------------------------------------------*/

static Word16 extra;

static Word16 D4i40_17( /* (o)    : Index of pulses positions.               */
  Word16 Dn[],          /* (i)    : Correlations between h[] and Xn[].       */
  Word16 rr[],          /* (i)    : Correlations of impulse response h[].    */
  Word16 h[],           /* (i) Q12: Impulse response of filters.             */
  Word16 cod[],         /* (o) Q13: Selected algebraic codeword.             */
  Word16 y[],           /* (o) Q12: Filtered algebraic codeword.             */
  Word16 *sign,         /* (o)    : Signs of 4 pulses.                       */
  Word16 i_subfr        /* (i)    : subframe flag                            */
)
{
    Word16  i0, i1, i2, i3, ip0, ip1, ip2, ip3;
    Word16  i, j, time;
    Word16  ps0, ps1, ps2, ps3, alp, alp0;
    Word32  alp1, alp2, alp3, L32;
    Word16  ps3c, psc, alpha;
    Word16  average, max0, max1, max2, thres;
    Word32  L_temp;
    
    Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
    Word16 *rri0i1, *rri0i2, *rri0i3, *rri0i4;
    Word16 *rri1i2, *rri1i3, *rri1i4;
    Word16 *rri2i3, *rri2i4;
    
    Word16 *ptr_ri0i0, *ptr_ri1i1, *ptr_ri2i2, *ptr_ri3i3, *ptr_ri4i4;
    Word16 *ptr_ri0i1, *ptr_ri0i2, *ptr_ri0i3, *ptr_ri0i4;
    Word16 *ptr_ri1i2, *ptr_ri1i3, *ptr_ri1i4;
    Word16 *ptr_ri2i3, *ptr_ri2i4;
    
    Word16  p_sign[L_SUBFR];
    
    /* Init pointers */
    
    rri0i0 = rr;
    rri1i1 = rri0i0 + NB_POS;
    rri2i2 = rri1i1 + NB_POS;
    rri3i3 = rri2i2 + NB_POS;
    rri4i4 = rri3i3 + NB_POS;
    
    rri0i1 = rri4i4 + NB_POS;
    rri0i2 = rri0i1 + MSIZE;
    rri0i3 = rri0i2 + MSIZE;
    rri0i4 = rri0i3 + MSIZE;
    rri1i2 = rri0i4 + MSIZE;
    rri1i3 = rri1i2 + MSIZE;
    rri1i4 = rri1i3 + MSIZE;
    rri2i3 = rri1i4 + MSIZE;
    rri2i4 = rri2i3 + MSIZE;
    
    /*-----------------------------------------------------------------------*
    * Reset max_time for 1st subframe.                                      *
    *-----------------------------------------------------------------------*/
    
    if (i_subfr == 0){ extra = 30; }
    
    /*-----------------------------------------------------------------------*
    * Chose the sign of the impulse.                                        *
    *-----------------------------------------------------------------------*/
    
    for (i=0; i<L_SUBFR; i++)
    {
        if( Dn[i] >= 0)
        {
            p_sign[i] = 0x7fff;
        }
        else
        {
            p_sign[i] = (Word16)0x8000;
            Dn[i] = negate(Dn[i]);
        }
    }
    
    /*-------------------------------------------------------------------*
    * - Compute the search threshold after three pulses                 *
    *-------------------------------------------------------------------*/
    
    /* Find maximum of Dn[i0]+Dn[i1]+Dn[i2] */
    
    max0 = Dn[0];
    max1 = Dn[1];
    max2 = Dn[2];
    
    for (i = 5; i < L_SUBFR; i+=STEP)
    {
        if (sub(Dn[i]  , max0) > 0){ max0 = Dn[i];   }
        if (sub(Dn[i+1], max1) > 0){ max1 = Dn[i+1]; }
        if (sub(Dn[i+2], max2) > 0){ max2 = Dn[i+2]; }
    }
    max0 = add(max0, max1);
    max0 = add(max0, max2);
    
    /* Find average of Dn[i0]+Dn[i1]+Dn[i2] */
    
    L32 = 0;
    for (i = 0; i < L_SUBFR; i+=STEP)
    {
        L32 = L_mac(L32, Dn[i], 1);
        L32 = L_mac(L32, Dn[i+1], 1);
        L32 = L_mac(L32, Dn[i+2], 1);
    }
    average =extract_l( L_shr(L32, 4));   /* 1/8 of sum */
    
    /* thres = average + (max0-average)*THRESHFCB; */
    
    thres = sub(max0, average);
    thres = mult(thres, THRESHFCB);
    thres = add(thres, average);
    
    /*-------------------------------------------------------------------*
    * Modification of rrixiy[] to take signs into account.              *
    *-------------------------------------------------------------------*/
    
    ptr_ri0i1 = rri0i1;
    ptr_ri0i2 = rri0i2;
    ptr_ri0i3 = rri0i3;
    ptr_ri0i4 = rri0i4;
    
    for(i0=0; i0<L_SUBFR; i0+=STEP)
    {
        for(i1=1; i1<L_SUBFR; i1+=STEP)
        {
            *ptr_ri0i1 = mult(*ptr_ri0i1, mult(p_sign[i0], p_sign[i1]));
            ptr_ri0i1++;
            *ptr_ri0i2 = mult(*ptr_ri0i2, mult(p_sign[i0], p_sign[i1+1]));
            ptr_ri0i2++;
            *ptr_ri0i3 = mult(*ptr_ri0i3, mult(p_sign[i0], p_sign[i1+2]));
            ptr_ri0i3++;
            *ptr_ri0i4 = mult(*ptr_ri0i4, mult(p_sign[i0], p_sign[i1+3]));
            ptr_ri0i4++;
            
        }
    }
    
    ptr_ri1i2 = rri1i2;
    ptr_ri1i3 = rri1i3;
    ptr_ri1i4 = rri1i4;
    
    for(i1=1; i1<L_SUBFR; i1+=STEP)
    {
        for(i2=2; i2<L_SUBFR; i2+=STEP)
        {
            *ptr_ri1i2 = mult(*ptr_ri1i2, mult(p_sign[i1], p_sign[i2]));
            ptr_ri1i2++;
            *ptr_ri1i3 = mult(*ptr_ri1i3, mult(p_sign[i1], p_sign[i2+1]));
            ptr_ri1i3++;
            *ptr_ri1i4 = mult(*ptr_ri1i4, mult(p_sign[i1], p_sign[i2+2]));
            ptr_ri1i4++;
            
        }
    }
    
    ptr_ri2i3 = rri2i3;
    ptr_ri2i4 = rri2i4;
    
    for(i2=2; i2<L_SUBFR; i2+=STEP)
    {
        for(i3=3; i3<L_SUBFR; i3+=STEP)
        {
            *ptr_ri2i3 = mult(*ptr_ri2i3, mult(p_sign[i2], p_sign[i3]));
            ptr_ri2i3++;
            *ptr_ri2i4 = mult(*ptr_ri2i4, mult(p_sign[i2], p_sign[i3+1]));
            ptr_ri2i4++;
            
        }
    }
    
    /*-------------------------------------------------------------------*
    * Search the optimum positions of the four  pulses which maximize   *
    *     square(correlation) / energy                                  *
    * The search is performed in four  nested loops. At each loop, one  *
    * pulse contribution is added to the correlation and energy.        *
    *                                                                   *
    * The fourth loop is entered only if the correlation due to the     *
    *  contribution of the first three pulses exceeds the preset        *
    *  threshold.                                                       *
    *-------------------------------------------------------------------*/
    
    /* Default values */
    
    ip0    = 0;
    ip1    = 1;
    ip2    = 2;
    ip3    = 3;
    psc    = 0;
    alpha  = MAX_16;
    time   = add(MAX_TIME, extra);
    
    
    /* Four loops to search innovation code. */
    
    ptr_ri0i0 = rri0i0;    /* Init. pointers that depend on first loop */
    ptr_ri0i1 = rri0i1;
    ptr_ri0i2 = rri0i2;
    ptr_ri0i3 = rri0i3;
    ptr_ri0i4 = rri0i4;
    
    for (i0 = 0; i0 < L_SUBFR; i0 += STEP)        /* first pulse loop  */
    {
        ps0  = Dn[i0];
        alp0 = *ptr_ri0i0++;
        
        ptr_ri1i1 = rri1i1;    /* Init. pointers that depend on second loop */
        ptr_ri1i2 = rri1i2;
        ptr_ri1i3 = rri1i3;
        ptr_ri1i4 = rri1i4;
        
        for (i1 = 1; i1 < L_SUBFR; i1 += STEP)      /* second pulse loop */
        {
            ps1  = add(ps0, Dn[i1]);
            
            /* alp1 = alp0 + *ptr_ri1i1++ + 2.0 * ( *ptr_ri0i1++); */
            
            alp1 = L_mult(alp0, 1);
            alp1 = L_mac(alp1, *ptr_ri1i1++, 1);
            alp1 = L_mac(alp1, *ptr_ri0i1++, 2);
            
            ptr_ri2i2 = rri2i2;     /* Init. pointers that depend on third loop */
            ptr_ri2i3 = rri2i3;
            ptr_ri2i4 = rri2i4;
            
            for (i2 = 2; i2 < L_SUBFR; i2 += STEP)    /* third pulse loop */
            {
                ps2  = add(ps1, Dn[i2]);
                
                /* alp2 = alp1 + *ptr_ri2i2++ + 2.0 * (*ptr_ri0i2++ + *ptr_ri1i2++); */
                
                alp2 = L_mac(alp1, *ptr_ri2i2++, 1);
                alp2 = L_mac(alp2, *ptr_ri0i2++, 2);
                alp2 = L_mac(alp2, *ptr_ri1i2++, 2);
                
                /* Test threshold */
                
                if ( sub(ps2, thres) > 0)
                {
                    
                    ptr_ri3i3 = rri3i3;    /* Init. pointers that depend on 4th loop */
                    
                    
                    for (i3 = 3; i3 < L_SUBFR; i3 += STEP)      /* 4th pulse loop */
                    {
                        ps3 = add(ps2, Dn[i3]);
                        
                        /* alp3 = alp2 + *ptr_ri3i3++                                */
                        /*       + 2.0*( *ptr_ri0i3++ + *ptr_ri1i3++ + *ptr_ri2i3++); */
                        
                        alp3 = L_mac(alp2, *ptr_ri3i3++, 1);
                        alp3 = L_mac(alp3, *ptr_ri0i3++, 2);
                        alp3 = L_mac(alp3, *ptr_ri1i3++, 2);
                        alp3 = L_mac(alp3, *ptr_ri2i3++, 2);
                        alp  = extract_l(L_shr(alp3, 5));
                        
                        ps3c = mult(ps3, ps3);
                        L_temp = L_mult(ps3c, alpha);
                        L_temp = L_msu(L_temp, psc, alp);
                        if( L_temp > 0L )
                        {
                            psc = ps3c;
                            alpha = alp;
                            ip0 = i0;
                            ip1 = i1;
                            ip2 = i2;
                            ip3 = i3;
                        }
                    }  /*  end of for i3 = */
                    ptr_ri0i3 -= NB_POS;
                    ptr_ri1i3 -= NB_POS;
                    
                    ptr_ri4i4 = rri4i4;    /* Init. pointers that depend on 4th loop */
                    
                    
                    for (i3 = 4; i3 < L_SUBFR; i3 += STEP)      /* 4th pulse loop */
                    {
                        ps3 = add(ps2, Dn[i3]);
                        
                        /* alp3 = alp2 + *ptr_ri4i4++                                */
                        /*       + 2.0*( *ptr_ri0i4++ + *ptr_ri1i4++ + *ptr_ri2i4++); */
                        
                        alp3 = L_mac(alp2, *ptr_ri4i4++, 1);
                        alp3 = L_mac(alp3, *ptr_ri0i4++, 2);
                        alp3 = L_mac(alp3, *ptr_ri1i4++, 2);
                        alp3 = L_mac(alp3, *ptr_ri2i4++, 2);
                        alp  = extract_l(L_shr(alp3, 5));
                        
                        ps3c = mult(ps3, ps3);
                        L_temp = L_mult(ps3c, alpha);
                        L_temp = L_msu(L_temp, psc, alp);
                        if( L_temp > 0L )
                        {
                            psc = ps3c;
                            alpha = alp;
                            ip0 = i0;
                            ip1 = i1;
                            ip2 = i2;
                            ip3 = i3;
                        }
                    }  /*  end of for i3 = */
                    ptr_ri0i4 -= NB_POS;
                    ptr_ri1i4 -= NB_POS;
                    
                    time = sub(time, 1);
                    if(time <= 0 ) goto end_search;     /* Maximum time finish */
                    
                }  /* end of if >thres */
                else
                {
                    ptr_ri2i3 += NB_POS;
                    ptr_ri2i4 += NB_POS;
                }
                
            } /* end of for i2 = */
            
            ptr_ri0i2 -= NB_POS;
            ptr_ri1i3 += NB_POS;
            ptr_ri1i4 += NB_POS;
            
        } /* end of for i1 = */
        
        ptr_ri0i2 += NB_POS;
        ptr_ri0i3 += NB_POS;
        ptr_ri0i4 += NB_POS;
        
    } /* end of for i0 = */
 
end_search:
 
    extra = time;
    
    /* Set the sign of impulses */
    
    i0 = p_sign[ip0];
    i1 = p_sign[ip1];
    i2 = p_sign[ip2];
    i3 = p_sign[ip3];
    
    /* Find the codeword corresponding to the selected positions */
    
    for(i=0; i<L_SUBFR; i++) {cod[i] = 0; }
    
    cod[ip0] = shr(i0, 2);         /* From Q15 to Q13 */
    cod[ip1] = shr(i1, 2);
    cod[ip2] = shr(i2, 2);
    cod[ip3] = shr(i3, 2);
    
    /* find the filtered codeword */
    
    for (i = 0; i < L_SUBFR; i++) {y[i] = 0;  }
    
    if(i0 > 0)
        for(i=ip0, j=0; i<L_SUBFR; i++, j++) {
            y[i] = add(y[i], h[j]); }
    else
        for(i=ip0, j=0; i<L_SUBFR; i++, j++) {
            y[i] = sub(y[i], h[j]); }
        
        if(i1 > 0)
            for(i=ip1, j=0; i<L_SUBFR; i++, j++) {
                    y[i] = add(y[i], h[j]); }
        else
            for(i=ip1, j=0; i<L_SUBFR; i++, j++) {
                y[i] = sub(y[i], h[j]); }
            
            if(i2 > 0)
                for(i=ip2, j=0; i<L_SUBFR; i++, j++) {
                    y[i] = add(y[i], h[j]); }
            else
                for(i=ip2, j=0; i<L_SUBFR; i++, j++) {
                    y[i] = sub(y[i], h[j]); }
                
                if(i3 > 0)
                    for(i=ip3, j=0; i<L_SUBFR; i++, j++) {
                        y[i] = add(y[i], h[j]); }
                else
                    for(i=ip3, j=0; i<L_SUBFR; i++, j++) {
                        y[i] = sub(y[i], h[j]); }
                                    
    /* find codebook index;  17-bit address */
                    
    i = 0;
    if(i0 > 0) i = add(i, 1);
    if(i1 > 0) i = add(i, 2);
    if(i2 > 0) i = add(i, 4);
    if(i3 > 0) i = add(i, 8);
    *sign = i;
    
    ip0 = mult(ip0, 6554);         /* ip0/5 */
    ip1 = mult(ip1, 6554);         /* ip1/5 */
    ip2 = mult(ip2, 6554);         /* ip2/5 */
    i   = mult(ip3, 6554);         /* ip3/5 */
    j   = add(i, shl(i, 2));       /* j = i*5 */
    j   = sub(ip3, add(j, 3));     /* j= ip3%5 -3 */
    ip3 = add(shl(i, 1), j);
    
    i = add(ip0, shl(ip1, 3));
    i = add(i  , shl(ip2, 6));
    i = add(i  , shl(ip3, 9));
    
    return i;
}

/*------------------------------------------------------------------------*
 * Function  D2i40_11                                                     *
 *           ~~~~~~~~~                                                    *
 * Algebraic codebook for ITU 6.4 kb/s.                                   *
 *  -> 11 bits; 2 pulses in a frame of 40 samples                         *
 *                                                                        *
 * The code length is 40, containing 2 nonzero pulses i0, i1.             *
 * i0 can have 16 possible positions (positive or negative) and i1        *
 * can have 32 possible positions. Thus, the two tracks are overlapping.  *
 *                                                                        *
 * i0 (+-1) : 0, 5, 10, 15, 20, 25, 30, 35     (i0,sub0)                  *
 *            1, 6, 11, 16, 21, 26, 31, 36     (i0,sub1)                  *
 *            2, 7, 12, 17, 22, 27, 32, 37     (i0,sub2)                  *
 *            4, 9, 14, 19, 24, 29, 34, 39     (i0,sub3)                  *
 *                                                                        *
 * i1 (+-1) : 1, 6, 11, 16, 21, 26, 31, 36     (i1,sub0)                  *
 *            3, 8, 13, 18, 23, 28, 33, 38     (i1,sub1)                  *
 *                                                                        *
 * The correlations of impulse response stored in the input parameter     *
 * rr[] (see Cor_h_X()) holds nine cross correlations if running in the   *
 * G729 mode. In this case they were stored as:                           *
 *                                                                        *
 * rri0i1[],                                                              *
 * rri0i2[],  rri1i2[],                                                   *
 * rri0i3[],  rri1i3[],  rri2i3[],                                        *
 * rri0i4[],  rri1i4[],  rri2i4[],                                        *
 *                                                                        *
 * In case of G729 at 6.4 kbps mode, three of the above cross             *
 * correlations are unnecessary for the codebook search. Two of           *
 * these three 64-element positions are instead used by two cross         *
 * correlations (RRi1i1[] and RRi3i4[]) needed only by the 6.4            *
 * kbps mode. Thus, eight cross correlations has to be calculated,        *
 * namely the following:                                                  *
 *                                                                        *
 * rri0i1[],                                                              *
 * RRi1i1[],  rri1i2[],                                                   *
 * rri0i3[],  rri1i3[],   rri2i3[],                                       *
 * RRi3i4[],  rri1i4[], (not used),                                       *
 *                                                                        *
 * Note that RRi1i1[] is a 64-element cross correlation, and that         *
 * rri1i1[] is an 8-element.                                              *
 *------------------------------------------------------------------------*/

static Word16 D2i40_11( /* (o)    : Index of pulses positions.            */
  Word16 Dn[],          /* (i)    : Correlations between h[] and Xn[].    */
  Word16 rr[],          /* (i)    : Correlations of impulse response h[]. */
  Word16 h[],           /* (i)    : Impulse response of filters.          */
  Word16 code[],        /* (o)    : Selected algebraic codeword.          */
  Word16 y[],           /* (o)    : Filtered algebraic codeword.          */
  Word16 *sign         /* (o)    : Signs of 4 pulses.                    */
)
{
    Word16  i0, i1, ip0, ip1, p0, p1;
    Word16  i, j, index, tmp, swap;
    Word16  ps0, ps1, alp, alp0;
    Word32  alp1;
    Word16  ps1c, psc, alpha;
    Word32  L_temp;
    Word16 posIndex[2], signIndex[2];
    Word16 m0_bestPos, m1_bestPos;
    
    Word16  p_sign[L_SUBFR];
    
    Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
    Word16 *rri0i1, *RRi1i1, *rri0i3, *RRi3i4;
    Word16 *rri1i2, *rri1i3, *rri1i4;
    Word16 *rri2i3;
    
    Word16 *ptr_ri0i0, *ptr_ri1i1;
    Word16 *ptr_ri0i1, *ptr_Ri0i2, *ptr_ri0i3, *ptr_Ri3i4;
    Word16 *ptr_ri1i2, *ptr_ri1i3, *ptr_ri1i4;
    Word16 *ptr_ri2i3;
    
    Word16 *outPtr_ri1i1; /* Outside loop pointer */
    
    /* Init pointers */
    
    rri0i0 = rr;
    rri1i1 = rri0i0 + NB_POS;
    rri2i2 = rri1i1 + NB_POS;
    rri3i3 = rri2i2 + NB_POS;
    rri4i4 = rri3i3 + NB_POS;
    
    rri0i1 = rri4i4 + NB_POS;
    RRi1i1 = rri0i1 + MSIZE;   /* Special for 6.4 kbps */
    rri0i3 = RRi1i1 + MSIZE;
    RRi3i4 = rri0i3 + MSIZE;   /* Special for 6.4 kbps */
    rri1i2 = RRi3i4 + MSIZE;
    rri1i3 = rri1i2 + MSIZE;
    rri1i4 = rri1i3 + MSIZE;
    rri2i3 = rri1i4 + MSIZE;
    
    /*-----------------------------------------------------------------------*
    * Chose the sign of the impulse.                                        *
    *-----------------------------------------------------------------------*/
    
    for (i=0; i<L_SUBFR; i++)
    {
        if( Dn[i] >= 0)
        {
            p_sign[i] = 0x7fff;
        }
        else
        {
            p_sign[i] = (Word16)0x8000;
            Dn[i] = negate(Dn[i]);
        }
    }
    
    /*-------------------------------------------------------------------*
    * Modification of rrixiy[] to take signs into account.              *
    *-------------------------------------------------------------------*/
    
    ptr_ri0i1 = rri0i1;
    ptr_ri0i3 = rri0i3;
    for(i0=0; i0<L_SUBFR; i0+=STEP) {
        for(i1=1; i1<L_SUBFR; i1+=STEP) {
            *ptr_ri0i1 = mult(*ptr_ri0i1, mult(p_sign[i0], p_sign[i1]));
            ptr_ri0i1++;
            *ptr_ri0i3 = mult(*ptr_ri0i3, mult(p_sign[i0], p_sign[i1+2]));
            ptr_ri0i3++;
        }
    }
    
    ptr_ri1i2 = rri1i2;
    ptr_ri1i3 = rri1i3;
    ptr_ri1i4 = rri1i4;
    for(i0=1; i0<L_SUBFR; i0+=STEP) {
        for(i1=2; i1<L_SUBFR; i1+=STEP) {
            *ptr_ri1i2 = mult(*ptr_ri1i2, mult(p_sign[i0], p_sign[i1]));
            ptr_ri1i2++;
            *ptr_ri1i3 = mult(*ptr_ri1i3, mult(p_sign[i0], p_sign[i1+1]));
            ptr_ri1i3++;
            *ptr_ri1i4 = mult(*ptr_ri1i4, mult(p_sign[i0], p_sign[i1+2]));
            ptr_ri1i4++;
        }
    }
    
    ptr_ri2i3 = rri2i3;
    ptr_Ri3i4 = RRi3i4;
    for(i0=2; i0<L_SUBFR; i0+=STEP) {
        for(i1=3; i1<L_SUBFR; i1+=STEP) {
            *ptr_ri2i3 = mult(*ptr_ri2i3, mult(p_sign[i0], p_sign[i1]));
            ptr_ri2i3++;
            *ptr_Ri3i4 = mult(*ptr_Ri3i4, mult(p_sign[i0+1], p_sign[i1+1]));
            ptr_Ri3i4++;
        }
    }
    
    ptr_Ri0i2 = RRi1i1;
    for(i0=1; i0<L_SUBFR; i0+=STEP) {
        for(i1=1; i1<L_SUBFR; i1+=STEP) {
            *ptr_Ri0i2 = mult(*ptr_Ri0i2, mult(p_sign[i0], p_sign[i1]));
            ptr_Ri0i2++;
        }
    }
    
    /*-------------------------------------------------------------------*
    * The actual search.                                                *
    *-------------------------------------------------------------------*/
    
    ip0 = 1;                     /* Set to any valid pulse position */
    ip1 = 0;                     /* Set to any valid pulse position */
    psc = 0;
    alpha = MAX_16;
    ptr_ri0i1 = rri0i1; 
    
    outPtr_ri1i1 = rri1i1;       /* Initial values for tripple loop below */
    p0=0;                        /* Search i0,sub0 vs. i1,sub0 */ 
    p1=1;
    ptr_ri0i0 = rri0i0;
    
    for (i = 0; i<9; i++) {
        
        if (i == 4) i++;          /* To get right exchange sequence */
        swap = i & (Word16)1;
        
        if (i == 1) p0=1;         /* Search i0,sub1 vs. i1,sub0 */
        
        else if (i == 2) {        /* Search i0,sub0 vs. i1,sub1 */
            outPtr_ri1i1 = rri3i3;
            p0=0;
            p1=3;
            ptr_ri0i0 = rri0i0;
        }
        
        else if (i == 3) {        /* Search i0,sub3 vs. i1,sub1 */
            outPtr_ri1i1 = rri4i4;
            p0=3;
            p1=4;
            ptr_ri0i0 = rri3i3;
        }
        
        else if (i == 5) {        /* Search i0,sub2 vs. i1,sub0 */
            outPtr_ri1i1 = rri2i2;
            p0=1;
            p1=2;
            ptr_ri0i0 = rri1i1;
        }
        
        else if (i == 6) {        /* Search i0,sub1 vs. i1,sub1 */
            outPtr_ri1i1 = rri3i3;
            p1=3;
            ptr_ri0i0 = rri1i1;
        }
        
        else if (i == 7) {        /* Search i0,sub3 vs. i1,sub0 */
            outPtr_ri1i1 = rri4i4;
            p1=4;
            ptr_ri0i0 = rri1i1;
        }
        
        else if (i == 8) {        /* Search i0,sub2 vs. i1,sub1 */
            outPtr_ri1i1 = rri3i3;
            p0=2;
            p1=3;
        }
        
        for (i0 = p0; i0<40; i0+=STEP) {
            ptr_ri1i1 = outPtr_ri1i1;
            ps0 = Dn[i0];
            alp0 = *ptr_ri0i0++;
            for (i1 = p1; i1<40; i1+=STEP) {
                ps1 = add(ps0, Dn[i1]);
                alp1 = L_mult(alp0, 1);
                alp1 = L_mac(alp1, *ptr_ri1i1++, 1);
                alp1 = L_mac(alp1, *ptr_ri0i1++, 2);
                alp = extract_l(L_shr(alp1, 5));
                ps1c = mult(ps1, ps1);
                L_temp = L_mult(ps1c, alpha);
                L_temp = L_msu(L_temp, psc, alp);
                if (L_temp > 0L) {
                    psc = ps1c;
                    alpha = alp;
                    ip0 = i1;
                    ip1 = i0;
                    if ( swap ) {
                        ip0 = i0;
                        ip1 = i1;
                    }
                }
            }
        }
    }
    
    /* convert from position to table entry index */
    for (i0=0; i0<16; i0++)
        if (ip0 == trackTable0[i0]) break;   
    ip0=i0;
        
    for (i1=0; i1<32; i1++) 
        if (ip1 == trackTable1[i1]) break;   
    ip1=i1;
        
    m0_bestPos = trackTable0[ip0];
    m1_bestPos = trackTable1[ip1];
    
    posIndex[0] = grayEncode[ip0];
    posIndex[1] = grayEncode[ip1];
    
    if (p_sign[m0_bestPos] > 0)
        signIndex[0] = 1;
    else
        signIndex[0] = 0;
    
    if (p_sign[m1_bestPos] > 0)
        signIndex[1] = 1;
    else
        signIndex[1] = 0;
    
    /* build innovation vector */
    for (i = 0; i < L_SUBFR; i++) code[i] = 0;
    
    code[m0_bestPos] = shr(p_sign[m0_bestPos], 2);
    code[m1_bestPos] = add(code[m1_bestPos], shr(p_sign[m1_bestPos], 2));
    
    *sign = add(signIndex[1], signIndex[1]);
    *sign = add(*sign, signIndex[0]);
    
    tmp = shl(posIndex[1], 4);
    index = add(posIndex[0], tmp);
    
    /* compute filtered cbInnovation */
    for (i = 0; i < L_SUBFR; i++) y[i] = 0;
    
    if(signIndex[0] == 0) 
        for(i=m0_bestPos, j=0; i<L_SUBFR; i++, j++) y[i] = negate(h[j]);
    else
        for(i=m0_bestPos, j=0; i<L_SUBFR; i++, j++) y[i] = h[j];
            
    if(signIndex[1] == 0)
        for(i=m1_bestPos, j=0; i<L_SUBFR; i++, j++) y[i] = sub(y[i], h[j]);
    else
        for(i=m1_bestPos, j=0; i<L_SUBFR; i++, j++) y[i] = add(y[i], h[j]);
        
    return index;
}

/*-------------------------------------------------------------------*
 * Function  ACELP_12i40_44bits()                                    *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                    *
 * Algebraic codebook; 44 bits: 12 pulses in a frame of 40 samples.  *
 *-------------------------------------------------------------------*
 * The code length is 40, containing 12 nonzero pulses: i0...i11.    *
 * 12 pulses can have two (2) possible amplitudes: +1 or -1.         *
 * 10 pulses can have eight (8) possible positions:                  *
 * i2,i7  :  0, 5, 10, 15, 20, 25, 30, 35.  --> t0                   *
 * i3,i8  :  1, 6, 11, 16, 21, 26, 31, 36.  --> t1                   *
 * i4,i9  :  2, 7, 12, 17, 22, 27, 32, 37.  --> t2                   *
 * i5,i10 :  3, 8, 13, 18, 23, 28, 33, 38.  --> t3                   *
 * i6,i11 :  4, 9, 14, 19, 24, 29, 34, 39.  --> t4                   *
 *                                                                   *
 * The 2 other pulses can be on the following track:                 *
 *   t0-t1,t1-t2,t2-t3,t3-t4,t4-t0.                                  *
 *-------------------------------------------------------------------*/
void ACELP_12i40_44bits(
  Word16 x[],    /* (i) Q0 : target vector                                 */
  Word16 cn[],   /* (i) Q0 : residual after long term prediction           */
  Word16 H[],    /* (i) Q12: impulse response of weighted synthesis filter */
  Word16 code[], /* (o) Q12: algebraic (fixed) codebook excitation         */
  Word16 y[],    /* (o) Q11: filtered fixed codebook excitation            */
  Word16 indx[]  /* (o)    : index 5 words: 13,10,7,7,7 = 44 bits          */
)
{
    Word16 i, j, k, ix, iy, itrk[3], track, pos, index, idx[NB_TRACK];
    Word16 psk, ps, alpk, alp;
    Word32 s, corr[NB_TRACK];
    Word16 *p0, *p1, *h, *h_inv;
    
    Word16 dn[L_SUBFR], sign[L_SUBFR], vec[L_SUBFR];
    Word16 ip[12], codvec[12], pos_max[NB_TRACK];
    Word16 cor_x[NB_POS], cor_y[NB_POS];
    Word16 h_buf[4*L_SUBFR];
    Word16 rrixix[NB_TRACK][NB_POS], rrixiy[NB_TRACK][MSIZE];
    Word32 L_tmp;
    
    h = h_buf;
    h_inv = h_buf + (2*L_SUBFR);
    for (i=0; i<L_SUBFR; i++) {
        *h++ = 0;
        *h_inv++ = 0;
    }
    
    /* Compute correlation between target x[] and H[] */
    cor_h_x_e(H, x, dn);
    
    /* find the sign of each pulse position */
    set_sign(32767, cn, dn, sign, vec, pos_max, corr);
    
    /* Compute correlations of h[] needed for the codebook search. */
    cor_h_e(H, sign, vec, h, h_inv, rrixix, rrixiy);
    
    /*-------------------------------------------------------------------*
    * Search position for pulse i0 and i1.                              *
    *-------------------------------------------------------------------*/
    s = L_add(corr[4], corr[0]);
    for (k=0; k<NB_TRACK-1; k++) corr[k] = L_add(corr[k], corr[k+1]);
    corr[4] = s;
    
    for (k=0; k<3; k++) {
        s = corr[0];
        track = 0;
        for (i=1; i<NB_TRACK; i++) {
            L_tmp = L_sub(corr[i], s);
            if (L_tmp > 0) {
                s = corr[i];
                track = i;
            }
        }
        corr[track] = -1;
        itrk[k] = track;
    }
    
    /*-------------------------------------------------------------------*
    * Deep first search: 3 iterations of 320 tests = 960 tests.         *
    *                                                                   *
    * Stages of deep first search:                                      *
    *   stage 1 : fix i0  and i1  --> 2 positions is fixed previously.  *
    *   stage 2 : fix i2  and i3  --> try 8x8 = 64 positions.           *
    *   stage 3 : fix i4  and i5  --> try 8x8 = 64 positions.           *
    *   stage 4 : fix i6  and i7  --> try 8x8 = 64 positions.           *
    *   stage 5 : fix i8  and i9  --> try 8x8 = 64 positions.           *
    *   stage 6 : fix i10 and i11 --> try 8x8 = 64 positions.           *
    *-------------------------------------------------------------------*/
    
    /* stage 0: fix pulse i0 and i1 according to max of correlation */
    psk = -1;
    alpk = 1;
    for (pos=0; pos<3; pos++)  {
        k = itrk[pos];       /* starting position index */
        
        /* stage 1: fix pulse i0 and i1 according to max of correlation */
        ix = pos_max[ipos[k]];
        iy = pos_max[ipos[k+1]];
        ps = add(dn[ix], dn[iy]);
        i = mult(ix, Q15_1_5);
        j = mult(iy, Q15_1_5);
        alp = add(rrixix[ipos[k]][i], rrixix[ipos[k+1]][j]);
        i = add(shl(i,3), j);
        alp = add(alp, rrixiy[ipos[k]][i]);
        ip[0] = ix;
        ip[1] = iy;
        
        for (i=0; i<L_SUBFR; i++) vec[i] = 0;
        
        /* stage 2..5: fix pulse i2,i3,i4,i5,i6,i7,i8 and i9 */
        for (j=2; j<12; j+=2) {
        /*--------------------------------------------------*
        * Store all impulse response of all fixed pulses   *
        * in vector vec[] for the "cor_h_vec()" function.  *
            *--------------------------------------------------*/
            if (sign[ix] < 0) p0 = h_inv - ix;
            else p0 = h - ix;
            
            if (sign[iy] < 0) p1 = h_inv - iy;
            else p1 = h - iy;
            
            for (i=0; i<L_SUBFR; i++) {
                vec[i] = add(vec[i], add(*p0, *p1));
                p0++; p1++;
            }
            
            /*--------------------------------------------------*
            * Calculate correlation of all possible positions  *
            * of the next 2 pulses with previous fixed pulses. *
            * Each pulse can have 8 possible positions         *
            *--------------------------------------------------*/
            cor_h_vec(h, vec, ipos[k+j], sign, rrixix, cor_x);
            cor_h_vec(h, vec, ipos[k+j+1], sign, rrixix, cor_y);
            
            /*--------------------------------------------------*
            * Fix 2 pulses, try 8x8 = 64 positions.            *
            *--------------------------------------------------*/
            search_ixiy(ipos[k+j], ipos[k+j+1], &ps, &alp, &ix, &iy,
                dn, cor_x, cor_y, rrixiy);
            
            ip[j] = ix;
            ip[j+1] = iy;
            
        }
        
        /* memorise new codevector if it's better than the last one. */
        ps = mult(ps,ps);
        s = L_msu(L_mult(alpk,ps),psk,alp);
        if (s > 0) {
            psk = ps;
            alpk = alp;
            for (i=0; i<12; i++) codvec[i] = ip[i];
        }
    } /* end of for (pos=0; pos<3; pos++) */
    
    /*-------------------------------------------------------------------*
    * index of 12 pulses = 44 bits on 5 words                           *
    * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                           *
    * indx[0] =13 bits --> 3(track) +                                   *
    *                      3(pos#11) + 3(pos#6) + 1(sign#1) + 3(pos#1)  *
    * indx[1] =10 bits --> 3(pos#12) + 3(pos#7) + 1(sign#2) + 3(pos#2)  *
    * indx[2] = 7 bits -->             3(pos#8) + 1(sign#3) + 3(pos#3)  *
    * indx[3] = 7 bits -->             3(pos#9) + 1(sign#4) + 3(pos#4)  *
    * indx[4] = 7 bits -->             3(pos#10)+ 1(sign#5) + 3(pos#5)  *
    *-------------------------------------------------------------------*/
    build_code(codvec+2, sign, 10, H, code, y, idx);
    
    for (k=0; k<2; k++) {
        
        pos = codvec[k];
        index = mult(pos, Q15_1_5);    /* index = pos/5       */
        track = sub(pos, extract_l(L_shr(L_mult(index, 5), 1)));
        if (sign[pos] > 0) {
            code[pos] = add(code[pos], 4096);     /* 1.0 in Q12 */
            for (i=pos, j=0; i<L_SUBFR; i++, j++) y[i] = add(y[i], H[j]);
        }
        else {
            code[pos] = sub(code[pos], 4096);     /* 1.0 in Q12 */
            for (i=pos, j=0; i<L_SUBFR; i++, j++) y[i] = sub(y[i], H[j]);
            index = add(index, 8);
        }
        
        ix = shr(idx[track], (Word16)4) & (Word16)15;
        iy = idx[track] & (Word16)15;
        
        index = pack3(ix, iy, index);
        if (k == 0) index = add(shl(track, 10), index);
        indx[k] = index;
        
    }
    
    for (k=2; k<NB_TRACK; k++) {
        track = add(track, 1);
        if (track >= NB_TRACK) track = 0;
        indx[k] = (idx[track] & (Word16)127);
    }
    
    return;
}

/*-------------------------------------------------------------------*
 * Function  ACELP_10i40_35bits()                                    *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                    *
 * Algebraic codebook; 35 bits: 10 pulses in a frame of 40 samples.  *
 *-------------------------------------------------------------------*
 * The code length is 40, containing 10 nonzero pulses: i0...i9.     *
 * All pulses can have two (2) possible amplitudes: +1 or -1.        *
 * Each pulse can have eight (8) possible positions:                 *
 *                                                                   *
 * i0,i5 :  0, 5, 10, 15, 20, 25, 30, 35.                            *
 * i1,i6 :  1, 6, 11, 16, 21, 26, 31, 36.                            *
 * i2,i7 :  2, 7, 12, 17, 22, 27, 32, 37.                            *
 * i3,i8 :  3, 8, 13, 18, 23, 28, 33, 38.                            *
 * i4,i9 :  4, 9, 14, 19, 24, 29, 34, 39.                            *
 *-------------------------------------------------------------------*/
void ACELP_10i40_35bits(
  Word16 x[],    /* (i) Q0 : target vector                                 */
  Word16 cn[],   /* (i) Q0 : residual after long term prediction           */
  Word16 H[],    /* (i) Q12: impulse response of weighted synthesis filter */
  Word16 code[], /* (o) Q12: algebraic (fixed) codebook excitation         */
  Word16 y[],    /* (o) Q11: filtered fixed codebook excitation            */
  Word16 indx[]  /* (o)    : index 5 words: 7,7,7,7,7 = 35 bits            */
)
{
    Word16 i, j, k, ix, iy, pos, track;
    Word16 psk, ps, alpk, alp, itrk[3];
    Word32 s, corr[NB_TRACK], L_tmp;
    Word16 *p0, *p1, *h, *h_inv;

    /* these vectors are not static */
    Word16 dn[L_SUBFR], sign[L_SUBFR], vec[L_SUBFR];
    Word16 ip[10], codvec[10], pos_max[NB_TRACK];
    Word16 cor_x[NB_POS], cor_y[NB_POS];
    Word16 h_buf[4*L_SUBFR];
    Word16 rrixix[NB_TRACK][NB_POS], rrixiy[NB_TRACK][MSIZE];

    h = h_buf;
    h_inv = h_buf + (2*L_SUBFR);
    for (i=0; i<L_SUBFR; i++) {
        *h++ = 0;
        *h_inv++ = 0;
    }

    /* Compute correlation between target x[] and H[] */
    cor_h_x_e(H, x, dn);

    /* find the sign of each pulse position */
    set_sign(32767, cn, dn, sign, vec, pos_max, corr);

    /* Compute correlations of h[] needed for the codebook search. */
    cor_h_e(H, sign, vec, h, h_inv, rrixix, rrixiy);

    /*-------------------------------------------------------------------*
    * Search starting position for pulse i0 and i1.                     *
    *    In the deep first search, we start 4 times with different      *
    * position for i0 and i1.  At all, we have 5 possible positions to  *
    * start (position 0 to 5).  The following loop remove 1 position    *
    * to keep 4 positions for deep first search step.                   *
    *-------------------------------------------------------------------*/
    s = L_add(corr[4], corr[0]);
    for (k=0; k<NB_TRACK-1; k++) corr[k] = L_add(corr[k], corr[k+1]);
    corr[4] = s;

    for (k=0; k<3; k++) {
        s = corr[0];
        track = 0;
        for (i=1; i<NB_TRACK; i++) {
            L_tmp = L_sub(corr[i], s);
            if (L_tmp > 0) {
                s = corr[i];
                track = i;
            }
        }
        corr[track] = -1;
        itrk[k] = track;
    }

    /*-------------------------------------------------------------------*
    * Deep first search: 4 iterations of 256 tests = 1024 tests.        *
    *                                                                   *
    * Stages of deep first search:                                      *
    *     stage 1 : fix i0 and i1 --> 2 positions is fixed previously.  *
    *     stage 2 : fix i2 and i3 --> try 8x8 = 64 positions.           *
    *     stage 3 : fix i4 and i5 --> try 8x8 = 64 positions.           *
    *     stage 4 : fix i6 and i7 --> try 8x8 = 64 positions.           *
    *     stage 5 : fix i8 and i9 --> try 8x8 = 64 positions.           *
    *-------------------------------------------------------------------*/
    psk = -1;
    alpk = 1;
    for (pos=0; pos<3; pos++) {
        k = itrk[pos];       /* starting position index */

    /* stage 1: fix pulse i0 and i1 according to max of correlation */
        ix = pos_max[ipos[k]];
        iy = pos_max[ipos[k+1]];
        ps = add(dn[ix], dn[iy]);
        i = mult(ix, Q15_1_5);
        j = mult(iy, Q15_1_5);
        alp = add(rrixix[ipos[k]][i], rrixix[ipos[k+1]][j]);
        i = add(shl(i,3), j);
        alp = add(alp, rrixiy[ipos[k]][i]);
        ip[0] = ix;
        ip[1] = iy;

        for (i=0; i<L_SUBFR; i++) vec[i] = 0;

        /* stage 2..5: fix pulse i2,i3,i4,i5,i6,i7,i8 and i9 */
        for (j=2; j<10; j+=2) {

            /*--------------------------------------------------*
            * Store all impulse response of all fixed pulses   *
            * in vector vec[] for the "cor_h_vec()" function.  *
            *--------------------------------------------------*/
            if (sign[ix] < 0) p0 = h_inv - ix;
            else p0 = h - ix;

            if (sign[iy] < 0) p1 = h_inv - iy;
            else p1 = h - iy;

            for (i=0; i<L_SUBFR; i++) {
                vec[i] = add(vec[i], add(*p0, *p1));
                p0++; p1++;
            }

            /*--------------------------------------------------*
            * Calculate correlation of all possible positions  *
            * of the next 2 pulses with previous fixed pulses. *
            * Each pulse can have 8 possible positions         *
            *--------------------------------------------------*/
            cor_h_vec(h, vec, ipos[k+j], sign, rrixix, cor_x);
            cor_h_vec(h, vec, ipos[k+j+1], sign, rrixix, cor_y);

            /*--------------------------------------------------*
            * Fix 2 pulses, try 8x8 = 64 positions.            *
            *--------------------------------------------------*/
            search_ixiy(ipos[k+j], ipos[k+j+1], &ps, &alp, &ix, &iy,
                  dn, cor_x, cor_y, rrixiy);
            ip[j] = ix;
            ip[j+1] = iy;
        }

        /* memorise new codevector if it's better than the last one. */
        ps = mult(ps,ps);
        s = L_msu(L_mult(alpk,ps),psk,alp);

        if (s > 0) {
            psk = ps;
            alpk = alp;
            for (i=0; i<10; i++) codvec[i] = ip[i];
        }

    } /* end of for (pos=0; pos<3; pos++) */

    /*-------------------------------------------------------------------*
    * index of 10 pulses = 35 bits on 5 words                           *
    * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                           *
    * indx[0] = 7 bits --> 3(pos#6) + 1(sign#1) + 3(pos#1)              *
    * indx[1] = 7 bits --> 3(pos#7) + 1(sign#2) + 3(pos#2)              *
    * indx[2] = 7 bits --> 3(pos#8) + 1(sign#3) + 3(pos#3)              *
    * indx[3] = 7 bits --> 3(pos#9) + 1(sign#4) + 3(pos#4)              *
    * indx[4] = 7 bits --> 3(pos#10)+ 1(sign#5) + 3(pos#5)              *
    *-------------------------------------------------------------------*/
    build_code(codvec, sign, 10, H, code, y, indx);

    for (i=0; i<NB_TRACK; i++) indx[i] = indx[i] & (Word16)127;

    return;

}

/*-------------------------------------------------------------------*
 * Function  cor_h_x_e()                                               *
 * ~~~~~~~~~~~~~~~~~~~                                               *
 * Compute correlation between target "x[]" and "h[]".               *
 *-------------------------------------------------------------------*/

static void cor_h_x_e(
  Word16 h[],    /* (i) Q12 : impulse response of weighted synthesis filter */
  Word16 x[],    /* (i) Q0  : correlation between target and h[]            */
  Word16 dn[]    /* (o) Q0  : correlation between target and h[]            */
)
{
    Word16 i, j, k;
    Word32 s, y32[L_SUBFR], max, tot, L_tmp;

    /* first keep the result on 32 bits and find absolute maximum */
    tot = 5;
    for (k=0; k<NB_TRACK; k++) {
        max = 0;

        for (i=k; i<L_SUBFR; i+=STEP) {
            s = 0;
            for (j=i; j<L_SUBFR; j++) s = L_mac(s, x[j], h[j-i]);
            y32[i] = s;
            s = L_abs(s);
            L_tmp = L_sub(s, max);
            if (L_tmp > (Word32)0) max = s;
        }
        tot = L_add(tot, L_shr(max, 1));    /* tot += (2.0 x max) / 4.0 */
    }

    /* Find the number of right shifts to do on y32[] so that */
    /* 2.0 x sumation of all max of dn[] in each track not saturate. */
    j = sub(norm_l(tot), 2);     /* multiply tot by 4 */
    for (i=0; i<L_SUBFR; i++) {
        dn[i] = round(L_shl(y32[i], j));
    }
    return;
}

/*-------------------------------------------------------------------*
 * Function  cor_h_vec()                                             *
 * ~~~~~~~~~~~~~~~~~~~~~                                             *
 * Compute correlations of h[] with vec[] for the specified track.   *
 *-------------------------------------------------------------------*
 *-------------------------------------------------------------------*/
static void cor_h_vec(
  Word16 h[],           /* (i) scaled impulse response */
  Word16 vec[],         /* (i) vector to correlate with h[] */
  Word16 track,         /* (i) track to use */
  Word16 sign[],        /* (i) sign vector */
  Word16 rrixix[][NB_POS],  /* (i) correlation of h[x] with h[x] */
  Word16 cor[]          /* (o) result of correlation (NB_POS elements) */
)
{
    Word16 i, j, pos;
    Word16 *p0, *p1, *p2;
    Word32 s;

    p0 = rrixix[track];
    pos = track;
    for (i=0; i<NB_POS; i++, pos+=STEP) {
        s = 0;
        p1 = h;
        p2 = &vec[pos];
        for (j=pos; j<L_SUBFR; j++) {
            s = L_mac(s, *p1, *p2);
            p1++; p2++;
        }
        cor[i] = add(mult(round(s), sign[pos]), *p0++);
    }

    return;
}

/*-------------------------------------------------------------------*
* Function  search_ixiy()                                           *
* ~~~~~~~~~~~~~~~~~~~~~~~                                           *
* Find the best positions of 2 pulses in a subframe.                *
*-------------------------------------------------------------------*/
static void search_ixiy(
  Word16 track_x,       /* (i) track of pulse 1 */
  Word16 track_y,       /* (i) track of pulse 2 */
  Word16 *ps,           /* (i/o) correlation of all fixed pulses */
  Word16 *alp,          /* (i/o) energy of all fixed pulses */
  Word16 *ix,           /* (o) position of pulse 1 */
  Word16 *iy,           /* (o) position of pulse 2 */
  Word16 dn[],          /* (i) corr. between target and h[] */
  Word16 cor_x[],       /* (i) corr. of pulse 1 with fixed pulses */
  Word16 cor_y[],       /* (i) corr. of pulse 2 with fixed pulses */
  Word16 rrixiy[][MSIZE]  /* (i) corr. of pulse 1 with pulse 2 */
)
{
    Word16 x, y, pos;
    Word16 ps1, ps2, sq, sqk;
    Word16 alp1, alp2, alpk;
    Word16 *p0, *p1, *p2;
    Word32 s;

    p0 = cor_x;
    p1 = cor_y;
    p2 = rrixiy[track_x];
    sqk = -1;
    alpk = 1;
    for (x=track_x; x<L_SUBFR; x+=STEP) {
        ps1 = add(*ps, dn[x]);
        alp1 = add(*alp, *p0++);
        pos = -1;
        for (y=track_y; y<L_SUBFR; y+=STEP) {
            ps2 = add(ps1, dn[y]);
            alp2 = add(alp1, add(*p1++, *p2++));
            sq = mult(ps2, ps2);
            s = L_msu(L_mult(alpk,sq),sqk,alp2);
            if (s > 0) {
                sqk = sq;
                alpk = alp2;
                pos = y;
            }
        }
        p1 -= NB_POS;
        if (pos >= 0) {
            *ix = x;
            *iy = pos;
        }
    }
    *ps = add(*ps, add(dn[*ix], dn[*iy]));
    *alp = alpk;

    return;
}

/*-------------------------------------------------------------------*
* Function  set_sign()                                              *
* ~~~~~~~~~~~~~~~~~~~~                                              *
* Set the sign of each pulse position.                              *
*-------------------------------------------------------------------*/
static void set_sign(
  Word16 fac_cn,     /* (i) Q15: residual weight for sign determination */
  Word16 cn[],       /* (i) Q0 : residual after long term prediction    */
  Word16 dn[],       /* (i) Q0 : correlation between target and h[]     */
  Word16 sign[],     /* (o) Q15: sign vector (sign of each position)    */
  Word16 inv_sign[], /* (o) Q15: inverse of sign[]                      */
  Word16 pos_max[],  /* (o)    : pos of max of correlation              */
  Word32 corr[]      /* (o)    : correlation of each track              */
)
{
    Word16 i, k, pos, k_cn, k_dn, val;
    Word32 s, max;

    /* calculate energy for normalization of cn[] and dn[] */
    s = 0;
    for (i=0; i<L_SUBFR; i++) s = L_mac(s, cn[i], cn[i]);
    if (s < 512) s = 512;
    s = Inv_sqrt(s);
    k_cn = extract_h(L_shl(s, 5));     /* k_cn = 11..23170 */
    k_cn = mult(k_cn, fac_cn);

    s = 0;
    for (i=0; i<L_SUBFR; i++) s = L_mac(s, dn[i], dn[i]);
    if (s < 512) s = 512;
    s = Inv_sqrt(s);
    k_dn = extract_h(L_shl(s, 5));     /* k_dn = 11..23170 */

    /* set sign according to en[] = k_cn*cn[] + k_dn*dn[]    */

    /* find position of maximum of correlation in each track */
    for (k=0; k<NB_TRACK; k++) {
        max = -1;
        for (i=k; i<L_SUBFR; i+=STEP) {
            val = dn[i];
            s = L_mac(L_mult(k_cn, cn[i]), k_dn, val);
            if (s >= 0) {
                sign[i] = 32767L;         /* sign = +1 (Q15) */
                inv_sign[i] = -32768L;
            }
            else {
                sign[i] = -32768L;        /* sign = -1 (Q15) */
                inv_sign[i] = 32767L;
                val = negate(val);
            }
            dn[i] = val;      /* modify dn[] according to the fixed sign */
            s = L_abs(s);
            if (s > max) {
                max = s;
                pos = i;
            }
        }
        pos_max[k] = pos;
        corr[k] = max;
    }

    return;
}

/*-------------------------------------------------------------------*
* Function  cor_h_e()                                                 *
* ~~~~~~~~~~~~~~~~~                                                 *
* Compute correlations of h[] needed for the codebook search.       *
*-------------------------------------------------------------------*/
static void cor_h_e(
  Word16 H[],              /* (i) Q12 :Impulse response of filters */
  Word16 sign[],           /* (i) Q15: sign vector                 */
  Word16 inv_sign[],       /* (i) Q15: inverse of sign[]           */
  Word16 h[],              /* (o)     : scaled h[]                 */
  Word16 h_inv[],          /* (o)     : inverse of scaled h[]      */
  Word16 rrixix[][NB_POS], /* (o) energy of h[].                   */
  Word16 rrixiy[][MSIZE]   /* (o) correlation between 2 pulses.    */
)
{
    Word16 i, j, k, pos;
    Word16 *ptr_h1, *ptr_h2, *ptr_hf, *psign;
    Word16 *p0, *p1, *p2, *p3, *p4;
    Word32 cor;

    /*------------------------------------------------------------*
    * normalize h[] for maximum precision on correlation.        *
    *------------------------------------------------------------*/
    cor = 0;
    for(i=0; i<L_SUBFR; i++) cor = L_mac(cor, H[i], H[i]);

    /* scale h[] with shift operation */
    k = norm_l(cor);
    k = shr(k, 1);
    for(i=0; i<L_SUBFR; i++) h[i] = shl(H[i], k);
    cor = L_shl(cor, add(k, k));

    /*------------------------------------------------------------*
    * Scaling h[] with a factor (0.5 < fac < 0.25)               *
    * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~               *
    * extract_h(cor) = 8192 .. 32768 --> scale to 4096 (1/8 Q15) *
    *                                                            *
    * 4096 (1/8) = fac^2 * extract_h(cor)                        *
    * fac = sqrt(4096/extract_h(cor))                            *
    *                                                            *
    * fac = 1/sqrt(cor/4096) * 256 = 0.125 to 0.5                *
    *------------------------------------------------------------*/
    cor = L_shr(cor, 12);
    k = extract_h(L_shl(Inv_sqrt(cor), 8));
    for(i=0; i<L_SUBFR; i++) {
        h[i] = mult(h[i], k);
        h_inv[i] = negate(h[i]);
    }

    /*------------------------------------------------------------*
    * Compute rrixix[][] needed for the codebook search.         *
    * This algorithm compute impulse response energy of all      *
    * positions (8) in each track (5).         Total = 5x8 = 40. *
    *------------------------------------------------------------*/
    /* storage order --> i4i4, i3i3, i2i2, i1i1, i0i0 */
    /* Init pointers to last position of rrixix[] */
    p0 = &rrixix[0][NB_POS-1];
    p1 = &rrixix[1][NB_POS-1];
    p2 = &rrixix[2][NB_POS-1];
    p3 = &rrixix[3][NB_POS-1];
    p4 = &rrixix[4][NB_POS-1];
    ptr_h1 = h;
    cor    = 0x00010000L;           /* 1.0 (for rounding) */
    for(i=0; i<NB_POS; i++) {
        cor = L_mac(cor, *ptr_h1, *ptr_h1);  ptr_h1++;
        *p4-- = extract_h(cor);
        cor = L_mac(cor, *ptr_h1, *ptr_h1);  ptr_h1++;
        *p3-- = extract_h(cor);
        cor = L_mac(cor, *ptr_h1, *ptr_h1);  ptr_h1++;
        *p2-- = extract_h(cor);
        cor = L_mac(cor, *ptr_h1, *ptr_h1);  ptr_h1++;
        *p1-- = extract_h(cor);
        cor = L_mac(cor, *ptr_h1, *ptr_h1);  ptr_h1++;
        *p0-- = extract_h(cor);
    }

    /* Divide all elements of rrixix[][] by 2. */
    p0 = &rrixix[0][0];
    for(i=0; i<L_SUBFR; i++) 
    {
      *p0 = shr(*p0, 1);
      p0++;
    }

    /*------------------------------------------------------------*
    * Compute rrixiy[][] needed for the codebook search.         *
    * This algorithm compute correlation between 2 pulses        *
    * (2 impulses responses) in 5 possible adjacents tracks.     *
    * (track 0-1, 1-2, 2-3, 3-4 and 4-0).   Total = 5x8x8 = 320. *
    *------------------------------------------------------------*/
    /* storage order --> i3i4, i2i3, i1i2, i0i1, i4i0 */
    pos = MSIZE-1;
    ptr_hf = h + 1;
    for(k=0; k<NB_POS; k++) {
        p4 = &rrixiy[3][pos];
        p3 = &rrixiy[2][pos];
        p2 = &rrixiy[1][pos];
        p1 = &rrixiy[0][pos];
        p0 = &rrixiy[4][pos-NB_POS];
        cor = 0x00008000L;            /* 0.5 (for rounding) */
        ptr_h1 = h;
        ptr_h2 = ptr_hf;
        for(i=k+(Word16)1; i<NB_POS; i++ ) {
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p4 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);
            p4 -= (NB_POS+1);
            p3 -= (NB_POS+1);
            p2 -= (NB_POS+1);
            p1 -= (NB_POS+1);
            p0 -= (NB_POS+1);
        }

        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p4 = extract_h(cor);
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p2 = extract_h(cor);
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p1 = extract_h(cor);
        pos -= NB_POS;
        ptr_hf += STEP;
    }

    /* storage order --> i4i0, i3i4, i2i3, i1i2, i0i1 */
    pos = MSIZE-1;
    ptr_hf = h + 4;
    for(k=0; k<NB_POS; k++) {
        p4 = &rrixiy[4][pos];
        p3 = &rrixiy[3][pos-1];
        p2 = &rrixiy[2][pos-1];
        p1 = &rrixiy[1][pos-1];
        p0 = &rrixiy[0][pos-1];

        cor = 0x00008000L;            /* 0.5 (for rounding) */
        ptr_h1 = h;
        ptr_h2 = ptr_hf;
        for(i=k+(Word16)1; i<NB_POS; i++ ) {
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p4 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);

            p4 -= (NB_POS+1);
            p3 -= (NB_POS+1);
            p2 -= (NB_POS+1);
            p1 -= (NB_POS+1);
            p0 -= (NB_POS+1);
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p4 = extract_h(cor);
        pos--;
        ptr_hf += STEP;
    }

    /*------------------------------------------------------------*
    * Modification of rrixiy[][] to take signs into account.     *
    *------------------------------------------------------------*/
    p0 = &rrixiy[0][0];
    for (k=0; k<NB_TRACK; k++) {
        for(i=k; i<L_SUBFR; i+=STEP) {
            psign = sign;
            if (psign[i] < 0) psign = inv_sign;
            for(j=(Word16)((k+(Word16)1)%NB_TRACK); j<(Word16)L_SUBFR; j+=(Word16)STEP) {
                *p0 = mult(*p0, psign[j]); p0++;
            }
        }
    }

    return;
}
/*-------------------------------------------------------------------*
* Function  build_code()                                            *
* ~~~~~~~~~~~~~~~~~~~~~~                                            *
* Build the codeword, the filtered codeword and index of codevector.*
*-------------------------------------------------------------------*/
static void build_code(
  Word16 codvec[],      /* (i)    : positions of each pulse */
  Word16 sign[],        /* (i) Q15: sign vector             */
  Word16 nb_of_pulse,   /* (i)    : number of pulses        */
  Word16 H[],    /* (i) Q12: impulse response of weighted synthesis filter */
  Word16 code[], /* (o) Q12: algebraic (fixed) codebook excitation         */
  Word16 y[],    /* (o) Q11: filtered fixed codebook excitation            */
  Word16 indx[]  /* (o)    : index of pulses (5 words, 1 per track).       */
)
{
    Word16 i, j, k, index, track;

    for (i=0; i<L_SUBFR; i++) H[i] = shr(H[i], 1);        /* Q12 to Q11 */

    for (i=0; i<L_SUBFR; i++) {
        code[i] = 0;
        y[i] = 0;
    }

    for (i=0; i<NB_TRACK; i++) indx[i] = -1;

    for (k=0; k<nb_of_pulse; k++) {
        i = codvec[k];          /* read pulse position */
        index = mult(i, Q15_1_5);  /* index = pos/5       */
        /* track = pos%5 */
        track = sub(i, extract_l(L_shr(L_mult(index, 5), 1)));
        /* codeword & filtered codeword */
        if (sign[i] > 0) {
            code[i] = add(code[i], 4096);     /* Q12 */
            for (i=codvec[k], j=0; i<L_SUBFR; i++, j++) y[i] = add(y[i], H[j]);
        }
        else {
            code[i] = sub(code[i], 4096);     /* Q12 */
            index = add(index, 8);
            for (i=codvec[k], j=0; i<L_SUBFR; i++, j++) y[i] = sub(y[i], H[j]);
        }

        /* quantize position & sign */
        if (indx[track] < 0) {
            indx[track] = index;
        }
        else {
            if (((index ^ indx[track]) & 8) == 0) {
                /* sign of 1st pulse == sign of 2th pulse */
                if (sub(indx[track],index) <= 0) {
                    indx[track] = add(shl(indx[track], (Word16)4), index) | (Word16)256;
                }
                else {
                    indx[track] = add(shl(index, (Word16)4), indx[track]) | (Word16)256;
                }
            }
            else {
                /* sign of 1st pulse != sign of 2th pulse */
                if (sub((Word16)(indx[track] & (Word16)7),(Word16)(index & (Word16)7)) <= 0) {
                    indx[track] = add(shl(index, (Word16)4), indx[track]) | (Word16)256;
                }
                else {
                    indx[track] = add(shl(indx[track], (Word16)4), index) | (Word16)256;
                }
            }
        }
    }

    return;
}

/*-------------------------------------------------------------------*
* Function  pack3()                                                 *
* ~~~~~~~~~~~~~~~~~                                                 *
* build index of 3 pulses. (pack 3x4 bits into 10 bits).            *
*-------------------------------------------------------------------*/
static Word16 pack3(Word16 index1, Word16 index2, Word16 index3)
{
    Word16 k, index, tmp;

    if ((index1 & 7) > (index2 & 7)) {
        tmp = index1;
        index1 = index2;
        index2 = tmp;
    }
    if ((index1 & 7) > (index3 & 7)) {
        tmp = index1;
        index1 = index3;
        index3 = tmp;
    }
    if ((index2 & 7) > (index3 & 7)) {
        tmp = index2;
        index2 = index3;
        index3 = tmp;
    }

    k = add(add((Word16)(shr(index1, 1) & (Word16)4),(Word16)(shr(index2, 2) & (Word16)2)), (Word16)(shr(index3, 3) & (Word16)1));
    switch (k) {
        case 0:
        case 7:
            index = add(add(shl((Word16)(index1 & (Word16)7), (Word16)7), shl((Word16)(index2 & (Word16)7), (Word16)4)), index3);
            break;
        case 1:
        case 6:
            index = add(add(shl((Word16)(index3 & (Word16)7), (Word16)7), shl((Word16)(index1 & (Word16)7), (Word16)4)), index2);
            break;
        case 2:
        case 5:
            index = add(add(shl((Word16)(index2 & (Word16)7), (Word16)7), shl((Word16)(index1 & (Word16)7), (Word16)4)), index3);
            break;
        case 3:
        case 4:
            index = add(add(shl((Word16)(index2 & (Word16)7), (Word16)7), shl((Word16)(index3 & (Word16)7), (Word16)4)), index1);
            break;
    }

    return (index);
}
