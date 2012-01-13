/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/

/*
 File : ACELP_CP.C
*/

/*****************************************************************************/
/* fixed codebook encoding routines  for 11.8, 8 and 6.4 kbit/s              */
/*****************************************************************************/
#include <math.h>

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

/* prototypes of local functions */
static void cor_h_cp(
    FLOAT *H,         /* (i)     :Impulse response of filters */
    FLOAT *rr,        /* (o)     :Correlations of H[]         */
    int   rate
);
static int d4i40_17(   /* (o)    : Index of pulses positions.               */
    FLOAT Dn[],          /* (i)    : Correlations between h[] and Xn[].       */
    FLOAT rr[],          /* (i)    : Correlations of impulse response h[].    */
    FLOAT h[],           /* (i)    : Impulse response of filters.             */
    FLOAT cod[],         /* (o)    : Selected algebraic codeword.             */
    FLOAT y[],           /* (o)    : Filtered algebraic codeword.             */
    int   *sign,         /* (o)    : Signs of 4 pulses.                       */
    int   i_subfr        /* (i)    : subframe flag                            */
);
static void cor_h_vec(
    FLOAT h[],           /* (i) scaled impulse response */
    FLOAT vec[],         /* (i) vector to correlate with h[] */
    int track,         /* (i) track to use */
    FLOAT sign[],        /* (i) sign vector */
    FLOAT rrixix[][NB_POS],  /* (i) correlation of h[x] with h[x] */
    FLOAT cor[]          /* (o) result of correlation (NB_POS elements) */
);

static void search_ixiy(
    int track_x,       /* (i) track of pulse 1 */
    int track_y,       /* (i) track of pulse 2 */
    FLOAT *ps,           /* (i/o) correlation of all fixed pulses */
    FLOAT *alp,          /* (i/o) energy of all fixed pulses */
    int *ix,           /* (o) position of pulse 1 */
    int *iy,           /* (o) position of pulse 2 */
    FLOAT dn[],          /* (i) corr. between target and h[] */
    FLOAT cor_x[],       /* (i) corr. of pulse 1 with fixed pulses */
    FLOAT cor_y[],       /* (i) corr. of pulse 2 with fixed pulses */
    FLOAT rrixiy[][MSIZE]  /* (i) corr. of pulse 1 with pulse 2 */
);

static void set_sign(
    FLOAT cn[],       /* (i) : residual after long term prediction    */
    FLOAT dn[],       /* (i) : correlation between target and h[]     */
    FLOAT sign[],     /* (o) : sign vector (sign of each position)    */
    FLOAT inv_sign[], /* (o) : inverse of sign[]                      */
    int pos_max[],    /* (o) : pos of max of correlation              */
    FLOAT corr[]      /* (o) : correlation of each track              */
);

static void cor_h_e(
    FLOAT sign[],           /* (i) : sign vector                 */
    FLOAT inv_sign[],       /* (i) : inverse of sign[]           */
    FLOAT h[],              /* (o) : scaled h[]                 */
    FLOAT h_inv[],          /* (o) : inverse of scaled h[]      */
    FLOAT rrixix[][NB_POS], /* (o) energy of h[].                   */
    FLOAT rrixiy[][MSIZE]   /* (o) correlation between 2 pulses.    */
);

static void build_code(
    int codvec[],      /* (i)  : positions of each pulse */
    FLOAT sign[],      /* (i)  : sign vector             */
    int nb_of_pulse,   /* (i)  : number of pulses        */
    FLOAT h[],         /* (i)  : impulse response of weighted synthesis filter */
    FLOAT code[],      /* (o)  : algebraic (fixed) codebook excitation         */
    FLOAT y[],         /* (o)  : filtered fixed codebook excitation            */
    int indx[]         /* (o)  : index of pulses (5 words, 1 per track).       */
);

static int pack3(int index1, int index2, int index3);


int ACELP_codebook(     /* (o)     :index of pulses positions    */
    FLOAT x[],            /* (i)     :Target vector                */
    FLOAT h[],            /* (i)     :Impulse response of filters  */
    int   t0,             /* (i)     :Pitch lag                    */
    FLOAT pitch_sharp,    /* (i)     :Last quantized pitch gain    */
    int i_subfr,          /* (i)     :Indicator of 1st subframe,   */
    FLOAT code[],         /* (o)     :Innovative codebook          */
    FLOAT y[],            /* (o)     :Filtered innovative codebook */
    int *sign             /* (o)     :Signs of 4 pulses            */
)
{
    int i, index;
    FLOAT dn[L_SUBFR];
    FLOAT rr[DIM_RR];
    
    /*----------------------------------------------------------------*
    * Include fixed-gain pitch contribution into impulse resp. h[]    *
    * Find correlations of h[] needed for the codebook search.        *
    *-----------------------------------------------------------------*/
    
    if(t0 < L_SUBFR) {
        for (i = t0; i < L_SUBFR; i++)
            h[i] += pitch_sharp * h[i-t0];
    }       
    cor_h_cp(h, rr, G729);
    
    /*----------------------------------------------------------------*
    * Compute correlation of target vector with impulse response.     *
    *-----------------------------------------------------------------*/
    
    cor_h_x(h, x, dn);      /* backward filtered target vector dn */
    
                            /*----------------------------------------------------------------*
                            * Find innovative codebook.                                       *
    *-----------------------------------------------------------------*/
    
    index = d4i40_17(dn, rr, h, code, y, sign, i_subfr);
    
    /*------------------------------------------------------*
    * - Add the fixed-gain pitch contribution to code[].    *
    *-------------------------------------------------------*/
    
    if(t0 < L_SUBFR) {
        for (i = t0; i < L_SUBFR; i++)
            code[i] += pitch_sharp * code[i-t0];
    }
    return index;
}


int ACELP_codebook64(   /* (o)     :Index of pulses positions    */
                     FLOAT x[],            /* (i)     :Target vector                */
                     FLOAT h[],            /* (i)     :Impulse response of filters  */
                     int   t0,             /* (i)     :Pitch lag                    */
                     FLOAT pitch_sharp,    /* (i)     :Last quantized pitch gain    */
                     FLOAT code[],         /* (o)     :Innovative codebook          */
                     FLOAT y[],            /* (o)     :Filtered innovative codebook */
                     int *sign             /* (o)     :Signs of 4 pulses            */
                     )
{
    int i, j;

    int posIndex[NB_PULSES_6K];            /* position index of last constructed vector */
    int signIndex[NB_PULSES_6K];           /* sign index of last constructed vector */
    /* (1=positive, 0=negative) */
    
    FLOAT dn[L_SUBFR];
    
    FLOAT Csq_best;
    FLOAT E_best;
    FLOAT C;
    FLOAT Csq;
    FLOAT E;
    int m0_bestIndex;
    int m1_bestIndex;
    int m0_bestPos;
    int m1_bestPos;
    int index;
    int m0, m1, pulse0, pulse1;
    
    FLOAT *ptr1, *ptr2, *ptr3;
    FLOAT C0;
    
    FLOAT rr[DIM_RR];
    int p_sign[L_SUBFR];
    
    int   i0, i1, i2, i3;
    FLOAT *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
    FLOAT *rri0i1, *rri0i2, *rri0i3, *rri0i4;
    FLOAT *rri1i2, *rri1i3, *rri1i4;
    FLOAT *rri2i3, *rri2i4;
    
    FLOAT *ptr_ri0i1, *ptr_ri0i2, *ptr_ri0i3, *ptr_ri0i4;
    FLOAT *ptr_ri1i2, *ptr_ri1i3, *ptr_ri1i4;
    FLOAT *ptr_ri2i3, *ptr_ri2i4;
    
    /*----------------------------------------------------------------*
    * Include fixed-gain pitch contribution into impulse resp. h[]    *
    * Find correlations of h[] needed for the codebook search.        *
    *-----------------------------------------------------------------*/
    if(t0 < L_SUBFR) {
        for (i = t0; i < L_SUBFR; i++)
            h[i] += pitch_sharp * h[i-t0];
    }
    cor_h_x(h, x, dn); /* backward filtered target vector dn */

    cor_h_cp(h, rr, G729D);

    /* approximate sign by using ltpResidual and target */
    for ( i = 0; i < L_SUBFR; i++) {
        if (dn[i] >= 0) {
            p_sign[i] = 1;
        }
        else {
            p_sign[i] = -1;
            dn[i] = -dn[i];   /* absolute value vector */
        }
    }

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

    /*-------------------------------------------------------------------*
    * Modification of rrixiy to take into account signs.                *
    *-------------------------------------------------------------------*/
    ptr_ri0i1 = rri0i1;
    ptr_ri0i2 = rri0i2;
    ptr_ri0i3 = rri0i3;
    ptr_ri0i4 = rri0i4;
    
    for(i0=0; i0<L_SUBFR; i0+=STEP)
    {
        for(i1=1; i1<L_SUBFR; i1+=STEP) {
            *ptr_ri0i1 *= (p_sign[i0] * p_sign[i1]);
            ptr_ri0i1++;
            *ptr_ri0i3 *= (p_sign[i0] * p_sign[i1+2]);
            ptr_ri0i3++;
        }
    }

    ptr_ri1i2 = rri1i2;
    ptr_ri1i3 = rri1i3;
    ptr_ri1i4 = rri1i4;

    for(i1=1; i1<L_SUBFR; i1+=STEP)
    {
        for(i2=2; i2<L_SUBFR; i2+=STEP)
        {
            *ptr_ri1i2 *= (p_sign[i1] * p_sign[i2]);
            ptr_ri1i2++;
            *ptr_ri1i3 *= (p_sign[i1] * p_sign[i2+1]);
            ptr_ri1i3++;
            *ptr_ri1i4 *= (p_sign[i1] * p_sign[i2+2]);
            ptr_ri1i4++;
        }
    }
    
    ptr_ri2i3 = rri2i3;
    ptr_ri2i4 = rri2i4;
    ptr_ri0i4=rri0i4;
    
    for(i2=2; i2<L_SUBFR; i2+=STEP)
    {
        for(i3=3; i3<L_SUBFR; i3+=STEP)
        {
            *ptr_ri2i3 *= (p_sign[i2] * p_sign[i3]);
            ptr_ri2i3++;
            *ptr_ri2i4 *= (p_sign[i2] * p_sign[i3+1]);
            ptr_ri2i4++;
            *ptr_ri0i4 *= (p_sign[i2+1] * p_sign[i3+1]); /* signs for i3 vs i4 */
            ptr_ri0i4++;
        }
    }
    
    /* signs for new i1 corr i1 */
    ptr_ri0i2=rri0i2;
    for(i2=1; i2<L_SUBFR; i2+=STEP)
    {
        for(i3=1; i3<L_SUBFR; i3+=STEP)
        {
            *ptr_ri0i2 *= (p_sign[i2] * p_sign[i3]);
            ptr_ri0i2++;
        }
    }
    
    /* start of actual search */
    
    Csq_best=(F)0.0;
    E_best=(F)1.0e38;
    
    m0_bestIndex=trackTable0[0];
    m1_bestIndex=trackTable1[0];
    
    /* m0 -> i1 m1 -> i0 (sub 0 vs sub 0) */
    m1 = 0;
    ptr1 = rri0i0;
    ptr3 = rri0i1;
    for (pulse1=0; pulse1<24; pulse1+=3, m1+=STEP) {
        m0 = 1;
        ptr2 = rri1i1;
        C0 = dn[m1];
        for (pulse0=0; pulse0<16; pulse0+=2, m0+=STEP) {
            C = C0;
            E = *ptr1;
            C += dn[m0];
            Csq = C*C;
            E += *ptr2++;
            E += (F)2.0 * *ptr3++;
            if ( (Csq*E_best) > (E*Csq_best) ) {
                E_best = E;
                Csq_best = Csq;
                m0_bestIndex=pulse0;
                m1_bestIndex=pulse1;
            }
        }
        ptr1++;
    }
    
    /* m0 -> i1 m1 -> i2 (sub 0 vs sub 1) */
    m0 = 1;
    ptr1 = rri1i1;
    ptr3 = rri1i2;
    for (pulse0=0; pulse0<posSearched[0]; pulse0+=2, m0+=STEP) {
        m1 = 2;
        ptr2 = rri2i2;
        C0 = dn[m0];
        for (pulse1=1; pulse1<24; pulse1+=3, m1+=STEP) {
            C = C0;
            E = *ptr1;
            C += dn[m1];
            Csq = C*C;
            E += *ptr2++;
            E += (F)2.0 * *ptr3++;
            if ( (Csq*E_best) > (E*Csq_best) ) {
                E_best = E;
                Csq_best = Csq;
                m0_bestIndex=pulse0;
                m1_bestIndex=pulse1;
            }
        }
        ptr1++;
    }
    
    /* m0 -> i1 m1 -> i4 (sub 0 vs sub 2) */
    m0 = 1;
    ptr1 = rri1i1;
    ptr3 = rri1i4;
    for (pulse0=0; pulse0<posSearched[0]; pulse0+=2, m0+=STEP) {
        m1 = 4;
        ptr2 = rri4i4;
        C0 = dn[m0];
        for (pulse1=2; pulse1<24; pulse1+=3, m1+=STEP) {
            C = C0;
            E = *ptr1;
            C += dn[m1];
            Csq = C*C;
            E += *ptr2++;
            E += (F)2.0 * *ptr3++;
            if ( (Csq*E_best) > (E*Csq_best) ) {
                E_best = E;
                Csq_best = Csq;
                m0_bestIndex=pulse0;
                m1_bestIndex=pulse1;
            }
        }
        ptr1++;
    }
    
    /* m0 -> i1 m1 -> i1 (sub 0 vs sub 3) */
    m0 = 1;
    ptr1 = rri1i1;
    ptr3 = rri0i2;
    for (pulse0=0; pulse0<posSearched[0]; pulse0+=2, m0+=STEP) {
        m1 = 1;
        ptr2 = rri1i1;
        C0=dn[m0];
        for (pulse1=24; pulse1<posSearched[1]; pulse1++, m1+=STEP) {
            C = C0;
            E = *ptr1;
            C += dn[m1];
            Csq = C*C;
            E += *ptr2++;
            E += (F)2.0 * *ptr3++;
            if ( (Csq*E_best) > (E*Csq_best) ) {
                E_best = E;
                Csq_best = Csq;
                m0_bestIndex=pulse0;
                m1_bestIndex=pulse1;
            }
        }
        ptr1++;
    }
    
    /* m0 -> i3 m1 -> i0 (sub 1 vs sub 0) */
    m1 = 0;
    ptr1 = rri0i0;
    ptr3 = rri0i3;
    for (pulse1=0; pulse1<24; pulse1+=3, m1+=STEP) {
        m0 = 3;
        ptr2 = rri3i3;
        C0 = dn[m1];
        for (pulse0=1; pulse0<16; pulse0+=2, m0+=STEP) {
            C = C0;
            E = *ptr1;
            C += dn[m0];
            Csq = C*C;
            E += *ptr2++;
            E += (F)2.0 * *ptr3++;
            if ( (Csq*E_best) > (E*Csq_best) ) {
                E_best = E;
                Csq_best = Csq;
                m0_bestIndex=pulse0;
                m1_bestIndex=pulse1;
            }
        }
        ptr1++;
    }

    /* m0 -> i3 m1 -> i2 (sub 1 vs sub 1) */
    m1 = 2;
    ptr1 = rri2i2;
    ptr3 = rri2i3;
    for (pulse1=1; pulse1<24; pulse1+=3, m1+=STEP) {
        m0 = 3;
        ptr2 = rri3i3;
        C0 = dn[m1];
        for (pulse0=1; pulse0<16; pulse0+=2, m0+=STEP) {
            C = C0;
            E = *ptr1;
            C += dn[m0];
            Csq = C*C;
            E += *ptr2++;
            E += (F)2.0 * *ptr3++;
            if ( (Csq*E_best) > (E*Csq_best) ) {
                E_best = E;
                Csq_best = Csq;
                m0_bestIndex=pulse0;
                m1_bestIndex=pulse1;
            }
        }
        ptr1++;
    }
    
    /* m0 -> i3 m1 -> i4 (sub 1 vs sub 2) rri0i4 contains rri3i4 */
    m0 = 3;
    ptr1 = rri3i3;
    ptr3 = rri0i4;
    for (pulse0=1; pulse0<posSearched[0]; pulse0+=2, m0+=STEP) {
        m1 = 4;
        ptr2 = rri4i4;
        C0 = dn[m0];
        for (pulse1=2; pulse1<24; pulse1+=3, m1+=STEP) {
            C = C0;
            E = *ptr1;
            C += dn[m1];
            Csq = C*C;
            E += *ptr2++;
            E += (F)2.0 * *ptr3++;
            if ( (Csq*E_best) > (E*Csq_best) ) {
                E_best = E;
                Csq_best = Csq;
                m0_bestIndex=pulse0;
                m1_bestIndex=pulse1;
            }
        }
        ptr1++;
    }
    
    /* m0 -> i3 m1 -> i1 (sub 1 vs sub 3) */
    m1 = 1;
    ptr1 = rri1i1;
    ptr3 = rri1i3;
    for (pulse1=24; pulse1<32; pulse1++, m1+=STEP) {
        m0 = 3;
        ptr2 = rri3i3;
        C0 = dn[m1];
        for (pulse0=1; pulse0<16; pulse0+=2, m0+=STEP) {
            C = C0;
            E = *ptr1;
            C += dn[m0];
            Csq = C*C;
            E += *ptr2++;
            E += (F)2.0 * *ptr3++;
            if ( (Csq*E_best) > (E*Csq_best) ) {
                E_best = E;
                Csq_best = Csq;
                m0_bestIndex=pulse0;
                m1_bestIndex=pulse1;
            }
        }
        ptr1++;
    }
    
    m0_bestPos = trackTable0[m0_bestIndex];
    m1_bestPos = trackTable1[m1_bestIndex];
    
    posIndex[0] = grayEncode[m0_bestIndex];
    posIndex[1] = grayEncode[m1_bestIndex];

    signIndex[0] = ( p_sign[m0_bestPos] > 0);
    signIndex[1] = ( p_sign[m1_bestPos] > 0);

    /* build innovation vector */
    for (i = 0; i < L_SUBFR; i++) code[i] = (F)0.0;
    code[m0_bestPos] = (FLOAT)p_sign[m0_bestPos];
    code[m1_bestPos] += (FLOAT)p_sign[m1_bestPos];
    
    *sign = signIndex[0] + 2*signIndex[1];
    index = posIndex[0] + 16*posIndex[1];
    
    /* compute filtered cbInnovation */
    for (i = 0; i < L_SUBFR; i++) y[i] = (F)0.0;
    
    if(signIndex[0] == 1) {
        for(i=m0_bestPos, j=0; i<L_SUBFR; i++, j++) y[i] = h[j];
    }
    else {
        for(i=m0_bestPos, j=0; i<L_SUBFR; i++, j++) y[i] = -h[j];
    }           
    if(signIndex[1] == 1) {
        for(i=m1_bestPos, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] + h[j];
    }
    else {
        for(i=m1_bestPos, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] - h[j];
    }
                
    if(t0 < L_SUBFR) {
        for (i = t0; i < L_SUBFR; i++)
            code[i] += pitch_sharp * code[i-t0];
    }
    return index;
}

/*--------------------------------------------------------------------------*
*  Function  cor_h()                                                       *
*  ~~~~~~~~~~~~~~~~~                                                       *
* Compute  correlations of h[]  needed for the codebook search.            *
*--------------------------------------------------------------------------*/

static void cor_h_cp(
                     FLOAT *h,      /* (i) :Impulse response of filters */
                     FLOAT *rr,     /* (o) :Correlations of H[]         */
                     int   rate
                     )
{
    FLOAT *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
    FLOAT *rri0i1, *rri0i2, *rri0i3, *rri0i4;
    FLOAT *rri1i2, *rri1i3, *rri1i4;
    FLOAT *rri2i3, *rri2i4;
    
    FLOAT *p0, *p1, *p2, *p3, *p4;
    
    FLOAT *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
    FLOAT cor;
    int i, k, ldec, l_fin_sup, l_fin_inf;
    int lsym;
    
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
    
    /*------------------------------------------------------------*
    * Compute rri0i0[], rri1i1[], rri2i2[], rri3i3 and rri4i4[]  *
    *------------------------------------------------------------*/
    
    p0 = rri0i0 + NB_POS-1;   /* Init pointers to last position of rrixix[] */
    p1 = rri1i1 + NB_POS-1;
    p2 = rri2i2 + NB_POS-1;
    p3 = rri3i3 + NB_POS-1;
    p4 = rri4i4 + NB_POS-1;
    
    ptr_h1 = h;
    cor    = (F)0.0;
    for(i=0;  i<NB_POS; i++)
    {
        cor += *ptr_h1 * *ptr_h1; ptr_h1++;
        *p4-- = cor;
        
        cor += *ptr_h1 * *ptr_h1; ptr_h1++;
        *p3-- = cor;
        
        cor += *ptr_h1 * *ptr_h1; ptr_h1++;
        *p2-- = cor;
        
        cor += *ptr_h1 * *ptr_h1; ptr_h1++;
        *p1-- = cor;
        
        cor += *ptr_h1 * *ptr_h1; ptr_h1++;
        *p0-- = cor;
    }
    
    /*-----------------------------------------------------------------*
    * Compute elements of: rri2i3[], rri1i2[], rri0i1[] and rri0i4[]  *
    *-----------------------------------------------------------------*/
    
    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-1;
    ldec = NB_POS+1;
    
    ptr_hd = h;
    ptr_hf = ptr_hd + 1;
    
    for(k=0; k<NB_POS; k++) {
        
        p4=rri0i4+l_fin_sup;
        p3 = rri2i3 + l_fin_sup;
        p2 = rri1i2 + l_fin_sup;
        p1 = rri0i1 + l_fin_sup;
        p0 = rri0i4 + l_fin_inf;
        cor = (F)0.0;
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        
        for(i=k+1; i<NB_POS; i++ ) {
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            if (rate == G729D) *p4=cor;           
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p3 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p2 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p1 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            if (rate == G729) *p0 = cor;
            
            p4-=ldec;                                  
            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        if (rate == G729D) *p4=cor;
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        *p3 = cor;
        
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        *p2 = cor;
        
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        *p1 = cor;
        
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
    l_fin_inf = l_fin_sup-1;
    for(k=0; k<NB_POS; k++) {
        
        p4 = rri2i4 + l_fin_sup;
        p3 = rri1i3 + l_fin_sup;
        p2 = rri0i2 + l_fin_sup;
        p1 = rri1i4 + l_fin_inf;
        p0 = rri0i3 + l_fin_inf;
        
        cor = (F)0.0;
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        for(i=k+1; i<NB_POS; i++ ) {
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p4 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p3 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p2 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p1 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p0 = cor;
            
            p4 -= ldec;
            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        *p4 = cor;
        
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        *p3 = cor;
        
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        *p2 = cor;
        
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
    l_fin_inf = l_fin_sup-1;
    for(k=0; k<NB_POS; k++) {
        
        p4 = rri1i4 + l_fin_sup;
        p3 = rri0i3 + l_fin_sup;
        p2 = rri2i4 + l_fin_inf;
        p1 = rri1i3 + l_fin_inf;
        p0 = rri0i2 + l_fin_inf;
        
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        cor = (F)0.0;
        for(i=k+1; i<NB_POS; i++ ) {
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p4 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p3 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p2 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p1 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p0 = cor;
            
            p4 -= ldec;
            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        *p4 = cor;
        
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        *p3 = cor;
        
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
    l_fin_inf = l_fin_sup-1;
    for(k=0; k<NB_POS; k++) {
        
        if (rate == G729D)
            p3 = rri0i4 + l_fin_inf;
        else if (rate == G729)
            p3 = rri0i4 + l_fin_sup;
        p2 = rri2i3 + l_fin_inf;
        p1 = rri1i2 + l_fin_inf;
        p0 = rri0i1 + l_fin_inf;
        
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        cor = (F)0.;
        for(i=k+1; i<NB_POS; i++ ) {
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            if (rate == G729) *p3 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            if (rate == G729D) *p3 = cor;
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p2 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p1 = cor;
            
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p0 = cor;
            
            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
        if (rate == G729) *p3 = cor;
        
        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }
    
    /*----------------------------------------------------------------------*/  
    /* rri0i2 for i1 corr i1 */
    
    if (rate == G729D) {
        /* acf elements */
        p0 = rri0i2;                     
        for(k=0; k<NB_POS; k++) {
            *p0 = *rri1i1++;
            p0 += ldec;
        }
        ptr_hd = h;
        ptr_hf = ptr_hd + 5;
        l_fin_sup = MSIZE-1;
        l_fin_inf = l_fin_sup - NB_POS;
        lsym = 7;
        for (k=1; k < NB_POS; k++) {
            p0 = rri0i2+l_fin_inf;
            ptr_h1 = ptr_hd;
            ptr_h2 = ptr_hf;
            cor = (F)0.;
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
            *p0 = *(p0+lsym) = cor;
            p0 -= ldec;
            for(i=k+1; i<NB_POS; i++ ) {
                cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                cor += *ptr_h1 * *ptr_h2; ptr_h1++; ptr_h2++;
                *p0 = *(p0+lsym) = cor;
                
                p0 -= ldec;
            }
            l_fin_inf -= NB_POS;
            ptr_hf += 5;
            lsym += 7;
        }     
    }
    return;
}

static int extra;

/*----------------------------------------------------------------------------
* d4i40_17 - algebraic codebook search 17 bits; 4 pulses 40 sampleframe
*----------------------------------------------------------------------------
*/
static int d4i40_17(    /* output: pulse positions                          */
                    FLOAT dn[],           /* input : backward filtered target vector          */
                    FLOAT rr[],           /* input : autocorrelations of impulse response h[] */
                    FLOAT h[],            /* input : impulse response of filters              */
                    FLOAT cod[],          /* output: selected algebraic codeword              */
                    FLOAT y[],            /* output: filtered algebraic codeword              */
                    int   *signs,         /* output: signs of 4 pulses                        */
                    int   i_subfr         /* input : subframe flag                            */
                    )
{
/*
* The code length is 40, containing 4 nonzero pulses i0, i1, i2, i3.
* with pulse spacings of step = 5
* Each pulses can have 8 possible positions (positive or negative):
*
* i0 (+-1) : 0, 5, 10, 15, 20, 25, 30, 35
* i1 (+-1) : 1, 6, 11, 16, 21, 26, 31, 36
* i2 (+-1) : 2, 7, 12, 17, 22, 27, 32, 37
* i3 (+-1) : 3, 8, 13, 18, 23, 28, 33, 38
*            4, 9, 14, 19, 24, 29, 34, 39
*---------------------------------------------------------------------------
    */
    int   i0, i1, i2, i3, ip0, ip1, ip2, ip3;
    int   i, j, time;
    FLOAT ps0, ps1, ps2, ps3, alp0, alp1, alp2, alp3;
    FLOAT ps3c, psc, alpha;
    FLOAT average, max0, max1, max2, thres;
    FLOAT p_sign[L_SUBFR];
    
    FLOAT *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
    FLOAT *rri0i1, *rri0i2, *rri0i3, *rri0i4;
    FLOAT *rri1i2, *rri1i3, *rri1i4;
    FLOAT *rri2i3, *rri2i4;
    
    FLOAT *ptr_ri0i0, *ptr_ri1i1, *ptr_ri2i2, *ptr_ri3i3, *ptr_ri4i4;
    FLOAT *ptr_ri0i1, *ptr_ri0i2, *ptr_ri0i3, *ptr_ri0i4;
    FLOAT *ptr_ri1i2, *ptr_ri1i3, *ptr_ri1i4;
    FLOAT *ptr_ri2i3, *ptr_ri2i4;
    
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
    *-----------------------------------------------------------------------*
    */
    if (i_subfr == 0) extra = 30;
    
    /*----------------------------------------------------------------------*
    * Chose the signs of the impulses.                                      *
    *-----------------------------------------------------------------------*/
    
    for (i=0; i<L_SUBFR; i++)
    {
        if( dn[i] >= (F)0.0)
        {
            p_sign[i] = (F)1.0;
        }
        else {
            p_sign[i] = (F)-1.0;
            dn[i] = -dn[i];
        }
    }
    
    /*-------------------------------------------------------------------*
    * - Compute the search threshold after three pulses                 *
    *-------------------------------------------------------------------*/
    
    
    average  = dn[0] + dn[1] + dn[2];
    max0 = dn[0];
    max1 = dn[1];
    max2 = dn[2];
    for (i = 5; i < L_SUBFR; i+=STEP)
    {
        average += dn[i] + dn[i+1]+ dn[i+2];
        if (dn[i]   > max0) max0 = dn[i];
        if (dn[i+1] > max1) max1 = dn[i+1];
        if (dn[i+2] > max2) max2 = dn[i+2];
    }
    max0 += max1+max2;
    average  *= (F)0.125;         /* 1/8 */
    thres = average + (max0-average)*THRESHFCB;
    
    /*-------------------------------------------------------------------*
    * Modification of rrixiy to take into account signs.                *
    *-------------------------------------------------------------------*/
    ptr_ri0i1 = rri0i1;
    ptr_ri0i2 = rri0i2;
    ptr_ri0i3 = rri0i3;
    ptr_ri0i4 = rri0i4;
    
    for(i0=0; i0<L_SUBFR; i0+=STEP)
    {
        for(i1=1; i1<L_SUBFR; i1+=STEP) {
            *ptr_ri0i1 *= (p_sign[i0] * p_sign[i1]);
            ptr_ri0i1++;
            *ptr_ri0i2 *= (p_sign[i0] * p_sign[i1+1]);
            ptr_ri0i2++;
            *ptr_ri0i3 *= (p_sign[i0] * p_sign[i1+2]);
            ptr_ri0i3++;
            *ptr_ri0i4 *= (p_sign[i0] * p_sign[i1+3]);
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
            *ptr_ri1i2 *= (p_sign[i1] * p_sign[i2]);
            ptr_ri1i2++;
            *ptr_ri1i3 *= (p_sign[i1] * p_sign[i2+1]);
            ptr_ri1i3++;
            *ptr_ri1i4 *= (p_sign[i1] * p_sign[i2+2]);
            ptr_ri1i4++;
        }
    }
    
    ptr_ri2i3 = rri2i3;
    ptr_ri2i4 = rri2i4;
    
    for(i2=2; i2<L_SUBFR; i2+=STEP)
    {
        for(i3=3; i3<L_SUBFR; i3+=STEP)
        {
            *ptr_ri2i3 *= (p_sign[i2] * p_sign[i3]);
            ptr_ri2i3++;
            *ptr_ri2i4 *= (p_sign[i2] * p_sign[i3+1]);
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
    psc    = (F)0.0;
    alpha  = (F)1000000.0;
    time   = MAX_TIME + extra;
    
    /* Four loops to search innovation code. */
    ptr_ri0i0 = rri0i0;    /* Init. pointers that depend on first loop */
    ptr_ri0i1 = rri0i1;
    ptr_ri0i2 = rri0i2;
    ptr_ri0i3 = rri0i3;
    ptr_ri0i4 = rri0i4;
    
    for (i0 = 0; i0 < L_SUBFR; i0 += STEP)        /* first pulse loop  */
    {
        ps0  = dn[i0];
        alp0 = *ptr_ri0i0++;
        
        ptr_ri1i1 = rri1i1;    /* Init. pointers that depend on second loop */
        ptr_ri1i2 = rri1i2;
        ptr_ri1i3 = rri1i3;
        ptr_ri1i4 = rri1i4;
        
        for (i1 = 1; i1 < L_SUBFR; i1 += STEP)      /* second pulse loop */
        {
            ps1  = ps0  + dn[i1];
            alp1 = alp0 + *ptr_ri1i1++ + (F)2.0 **ptr_ri0i1++;
            
            ptr_ri2i2 = rri2i2;     /* Init. pointers that depend on third loop */
            ptr_ri2i3 = rri2i3;
            ptr_ri2i4 = rri2i4;
            
            for (i2 = 2; i2 < L_SUBFR; i2 += STEP)
            {
                ps2  = ps1  + dn[i2];
                alp2 = alp1 + *ptr_ri2i2++ + (F)2.0*(*ptr_ri0i2++ + *ptr_ri1i2++);
                
                if ( ps2 > thres)
                {
                    ptr_ri3i3 = rri3i3;    /* Init. pointers that depend on 4th loop */
                    
                    for (i3 = 3; i3 < L_SUBFR; i3 += STEP)
                    {
                        ps3 = ps2 + dn[i3];
                        alp3 = alp2 + *ptr_ri3i3++ + (F)2.0*(*ptr_ri1i3++ + *ptr_ri0i3++ + *ptr_ri2i3++);
                        
                        ps3c = ps3*ps3;
                        if( (ps3c*alpha) > (psc * alp3) )
                        {
                            psc = ps3c;
                            alpha = alp3;
                            ip0 = i0;
                            ip1 = i1;
                            ip2 = i2;
                            ip3 = i3;
                        }
                    }  /*  end of for i3 = */
                    
                    ptr_ri0i3 -= NB_POS;
                    ptr_ri1i3 -= NB_POS;
                    
                    ptr_ri4i4 = rri4i4;    /* Init. pointers that depend on 4th loop */
                    
                    for (i3 = 4; i3 < L_SUBFR; i3 += STEP)
                    {
                        ps3 = ps2 + dn[i3];
                        alp3 = alp2 + *ptr_ri4i4++ + (F)2.0*(*ptr_ri1i4++ + *ptr_ri0i4++ + *ptr_ri2i4++);
                        
                        ps3c = ps3*ps3;
                        if( (ps3c*alpha) > (psc * alp3) )
                        {
                            psc = ps3c;
                            alpha = alp3;
                            ip0 = i0;
                            ip1 = i1;
                            ip2 = i2;
                            ip3 = i3;
                        }
                    }       /*  end of for i3 = */
                    ptr_ri0i4 -= NB_POS;
                    ptr_ri1i4 -= NB_POS;
                    
                    time --;
                    if(time <= 0 ) goto end_search;     /* Maximum time finish */
                    
                }  /* end of if >thres */
                else {
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
    
    /* Find the codeword corresponding to the selected positions */
    
    for(i=0; i<L_SUBFR; i++) cod[i] = (F)0.0;
    cod[ip0] = p_sign[ip0];
    cod[ip1] = p_sign[ip1];
    cod[ip2] = p_sign[ip2];
    cod[ip3] = p_sign[ip3];
    
    /* find the filtered codeword */
    
    for (i = 0; i < L_SUBFR; i++) y[i] = (F)0.0;
    
    if(p_sign[ip0] > (F)0.0) {
        for(i=ip0, j=0; i<L_SUBFR; i++, j++) y[i] = h[j];
    }
    else {
        for(i=ip0, j=0; i<L_SUBFR; i++, j++) y[i] = -h[j];
    }   
    if(p_sign[ip1] > (F)0.0) {
        for(i=ip1, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] + h[j];
    }
    else {
        for(i=ip1, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] - h[j];
    }
    if(p_sign[ip2] > (F)0.0) {
        for(i=ip2, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] + h[j];
    }
    else {
        for(i=ip2, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] - h[j];
    }                       
    if(p_sign[ip3] > (F)0.0) {
        for(i=ip3, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] + h[j];
    }
    else {
        for(i=ip3, j=0; i<L_SUBFR; i++, j++) y[i] = y[i] - h[j];
    }                           
    /* find codebook index;  4 bit signs + 13 bit positions */
    
    i = 0;
    if(p_sign[ip0] > (F)0.0) i+=1;
    if(p_sign[ip1] > (F)0.0) i+=2;
    if(p_sign[ip2] > (F)0.0) i+=4;
    if(p_sign[ip3] > (F)0.0) i+=8;
    *signs = i;
    
    ip0 = ip0 / 5;
    ip1 = ip1 / 5;
    ip2 = ip2 / 5;
    j   = (ip3 % 5) - 3;
    ip3 = ( (ip3 / 5) << 1 ) + j;
    
    i = (ip0) + (ip1<<3) + (ip2<<6) + (ip3<<9);
    
    return i;
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
    FLOAT x[],    /* (i) : target vector                                 */
    FLOAT cn[],   /* (i) : residual after long term prediction           */
    FLOAT h1[],   /* (i) : impulse response of weighted synthesis filter */
    FLOAT code[], /* (o) : algebraic (fixed) codebook excitation         */
    FLOAT y[],    /* (o) : filtered fixed codebook excitation            */
    int indx[]    /* (o) : index 5 words: 13,10,7,7,7 = 44 bits          */
)
{

    int i, j, k, ix, iy, itrk[3], track, pos, index;
    int idx[NB_TRACK];
    FLOAT psk, ps, alpk, alp;
    FLOAT s, corr[NB_TRACK];
    FLOAT *p0, *p1, *h_inv, *h;
    
    FLOAT dn[L_SUBFR], sign[L_SUBFR], vec[L_SUBFR];
    int ip[12], codvec[12], pos_max[NB_TRACK];
    FLOAT cor_x[NB_POS], cor_y[NB_POS];
    FLOAT h_buf[4*L_SUBFR];
    FLOAT rrixix[NB_TRACK][NB_POS], rrixiy[NB_TRACK][MSIZE];
    FLOAT L_tmp;
    
    h = h_buf;
    h_inv = h_buf + (2*L_SUBFR);
    for (i=0; i<L_SUBFR; i++) {
        *h++ = (FLOAT)0.;
        *h_inv++ = (FLOAT)0.;
    }
    for (i=0; i<L_SUBFR; i++) {
        h[i] = h1[i];
    }
    
    /* Compute correlation between target x[] and H[] */
    cor_h_x(h, x, dn);
    
    /* find the sign of each pulse position */
    set_sign(cn, dn, sign, vec, pos_max, corr);
    
    /* Compute correlations of h[] needed for the codebook search. */
    cor_h_e(sign, vec, h, h_inv, rrixix, rrixiy);
    
    /*-------------------------------------------------------------------*
    * Search position for pulse i0 and i1.                              *
    *-------------------------------------------------------------------*/
    s = corr[4] + corr[0];
    for (k=0; k<NB_TRACK-1; k++) corr[k] += corr[k+1];
    corr[4] = s;
    
    for (k=0; k<3; k++) {
        s = corr[0];
        track = 0;
        for (i=1; i<NB_TRACK; i++) {
            L_tmp = corr[i]-s;
            if (L_tmp > (FLOAT)0.) {
                s = corr[i];
                track = i;
            }
        }
        corr[track] = (FLOAT)-1.;
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
    psk = (FLOAT)-1.;
    alpk = (FLOAT)1.;
    for (pos=0; pos<3; pos++)  {
        k = itrk[pos];       /* starting position index */
        
        /* stage 1: fix pulse i0 and i1 according to max of correlation */
        ix = pos_max[ipos[k]];
        iy = pos_max[ipos[k+1]];
        ps = dn[ix] + dn[iy];
        i = ix/5;
        j = iy/5;
        alp = rrixix[ipos[k]][i] + rrixix[ipos[k+1]][j];
        i = (i<<3) + j;
        alp += rrixiy[ipos[k]][i];
        ip[0] = ix;
        ip[1] = iy;
        
        for (i=0; i<L_SUBFR; i++) vec[i] = (FLOAT)0.;
        
        /* stage 2..5: fix pulse i2,i3,i4,i5,i6,i7,i8 and i9 */
        for (j=2; j<12; j+=2) {
        /*--------------------------------------------------*
        * Store all impulse response of all fixed pulses   *
        * in vector vec[] for the "cor_h_vec()" function.  *
            *--------------------------------------------------*/
            if (sign[ix] < (FLOAT)0.) p0 = h_inv - ix;
            else p0 = h - ix;
            
            if (sign[iy] < (FLOAT)0.) p1 = h_inv - iy;
            else p1 = h - iy;
            
            for (i=0; i<L_SUBFR; i++) {
                vec[i] += *p0 + *p1;
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
            search_ixiy((int)ipos[k+j], (int)ipos[k+j+1], &ps, &alp, &ix, &iy,
                dn, cor_x, cor_y, rrixiy);
            
            ip[j] = ix;
            ip[j+1] = iy;
            
        }
        
        /* memorise new codevector if it's better than the last one. */
        ps *= ps;
        s = alpk*ps-psk*alp;
        if (s > (FLOAT)0.) {
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
    build_code(codvec+2, sign, 10, h, code, y, idx);
    track = 0; /* to avoid visual warning */
    for (k=0; k<2; k++) {
        
        pos = codvec[k];
        index = pos/5;    /* index = pos/5       */
        track = pos%5;
        if (sign[pos] > (FLOAT)0.) {
            code[pos] += (FLOAT)1.;
            for (i=pos, j=0; i<L_SUBFR; i++, j++) y[i] += h[j];
        }
        else {
            code[pos] -= (FLOAT)1.;
            for (i=pos, j=0; i<L_SUBFR; i++, j++) y[i] -= h[j];
            index +=  8;
        }
        
        ix = (idx[track]>>4) & (int)15;
        iy = idx[track] & (int)15;
        
        index = pack3(ix, iy, index);
        if (k == 0) index += track<<10;
        indx[k] = index;
        
    }
    
    for (k=2; k<NB_TRACK; k++) {
        track++;
        if (track >= NB_TRACK) track = 0;
        indx[k] = (idx[track] & (int)127);
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
    FLOAT x[],    /* (i) : target vector                                 */
    FLOAT cn[],   /* (i) : residual after long term prediction           */
    FLOAT h1[],   /* (i) : impulse response of weighted synthesis filter */
    FLOAT code[], /* (o) : algebraic (fixed) codebook excitation         */
    FLOAT y[],    /* (o) : filtered fixed codebook excitation            */
    int indx[]    /* (o) : index 5 words: 7,7,7,7,7 = 35 bits            */
)
{
    int i, j, k, ix, iy, pos, track;
    FLOAT psk, ps, alpk, alp;
    int itrk[3];
    FLOAT s, corr[NB_TRACK], L_tmp;
    FLOAT *p0, *p1, *h_inv, *h;
    
    /* these vectors are not static */
    FLOAT dn[L_SUBFR], sign[L_SUBFR], vec[L_SUBFR];
    int ip[10], codvec[10], pos_max[NB_TRACK];
    FLOAT cor_x[NB_POS], cor_y[NB_POS];
    FLOAT h_buf[4*L_SUBFR];
    FLOAT rrixix[NB_TRACK][NB_POS], rrixiy[NB_TRACK][MSIZE];
    
    h = h_buf;
    h_inv = h_buf + (2*L_SUBFR);
    for (i=0; i<L_SUBFR; i++) {
        *h++ = (FLOAT)0.;
        *h_inv++ = (FLOAT)0.;
    }
    for (i=0; i<L_SUBFR; i++) {
        h[i] = h1[i];
    }
    
    /* Compute correlation between target x[] and h[] */
    cor_h_x(h, x, dn);
    
    /* find the sign of each pulse position */
    set_sign(cn, dn, sign, vec, pos_max, corr);
    
    /* Compute correlations of h[] needed for the codebook search. */
    cor_h_e(sign, vec, h, h_inv, rrixix, rrixiy);
    
    /*-------------------------------------------------------------------*
    * Search starting position for pulse i0 and i1.                     *
    *    In the deep first search, we start 4 times with different      *
    * position for i0 and i1.  At all, we have 5 possible positions to  *
    * start (position 0 to 5).  The following loop remove 1 position    *
    * to keep 4 positions for deep first search step.                   *
    *-------------------------------------------------------------------*/
    s = corr[4] + corr[0];
    for (k=0; k<NB_TRACK-1; k++) corr[k] += corr[k+1];
    corr[4] = s;
    
    for (k=0; k<3; k++) {
        s = corr[0];
        track = 0;
        for (i=1; i<NB_TRACK; i++) {
            L_tmp = corr[i]- s;
            if (L_tmp > (FLOAT)0.) {
                s = corr[i];
                track = i;
            }
        }
        corr[track] = (FLOAT)-1.;
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
    psk = (FLOAT)-1.;
    alpk = (FLOAT)1.;
    for (pos=0; pos<3; pos++) {
        k = itrk[pos];       /* starting position index */
        
        /* stage 1: fix pulse i0 and i1 according to max of correlation */
        ix = pos_max[ipos[k]];
        iy = pos_max[ipos[k+1]];
        ps = dn[ix] + dn[iy];
        i = ix/5;
        j = iy/5;
        alp = rrixix[ipos[k]][i] + rrixix[ipos[k+1]][j];
        i = (i<<3) +  j;
        alp += rrixiy[ipos[k]][i];
        ip[0] = ix;
        ip[1] = iy;
        
        for (i=0; i<L_SUBFR; i++) vec[i] = (FLOAT)0.;
        
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
                vec[i] += *p0 + *p1;
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
        ps *= ps;
        s = (alpk*ps)-(psk*alp);
        
        if (s > (FLOAT)0.) {
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
    build_code(codvec, sign, 10, h, code, y, indx);
    
    for (i=0; i<NB_TRACK; i++) indx[i] = indx[i] & (int)127;
    
    return;
    
}

/*-------------------------------------------------------------------*
* Function  cor_h_vec()                                             *
* ~~~~~~~~~~~~~~~~~~~~~                                             *
* Compute correlations of h[] with vec[] for the specified track.   *
*-------------------------------------------------------------------*
*-------------------------------------------------------------------*/
static void cor_h_vec(
                      FLOAT h[],           /* (i) scaled impulse response */
                      FLOAT vec[],         /* (i) vector to correlate with h[] */
                      int track,         /* (i) track to use */
                      FLOAT sign[],        /* (i) sign vector */
                      FLOAT rrixix[][NB_POS],  /* (i) correlation of h[x] with h[x] */
                      FLOAT cor[]          /* (o) result of correlation (NB_POS elements) */
                      )
{
    int i, j, pos;
    FLOAT *p0, *p1, *p2;
    FLOAT s;
    
    p0 = rrixix[track];
    pos = track;
    for (i=0; i<NB_POS; i++, pos+=STEP) {
        s = (FLOAT)0.;
        p1 = h;
        p2 = &vec[pos];
        for (j=pos; j<L_SUBFR; j++) {
            s += *p1 *  *p2;
            p1++; p2++;
        }
        cor[i] = (s* sign[pos]) + *p0++;
    }
    
    return;
}

/*-------------------------------------------------------------------*
* Function  search_ixiy()                                           *
* ~~~~~~~~~~~~~~~~~~~~~~~                                           *
* Find the best positions of 2 pulses in a subframe.                *
*-------------------------------------------------------------------*/
static void search_ixiy(
                        int track_x,       /* (i) track of pulse 1 */
                        int track_y,       /* (i) track of pulse 2 */
                        FLOAT *ps,           /* (i/o) correlation of all fixed pulses */
                        FLOAT *alp,          /* (i/o) energy of all fixed pulses */
                        int *ix,           /* (o) position of pulse 1 */
                        int *iy,           /* (o) position of pulse 2 */
                        FLOAT dn[],          /* (i) corr. between target and h[] */
                        FLOAT cor_x[],       /* (i) corr. of pulse 1 with fixed pulses */
                        FLOAT cor_y[],       /* (i) corr. of pulse 2 with fixed pulses */
                        FLOAT rrixiy[][MSIZE]  /* (i) corr. of pulse 1 with pulse 2 */
                        )
{
    int i, j, pos;
    FLOAT ps1, ps2, sq, sqk;
    FLOAT alp1, alp2, alpk;
    FLOAT *p0, *p1, *p2;
    FLOAT s;
    
    p0 = cor_x;
    p1 = cor_y;
    p2 = rrixiy[track_x];
    sqk = (FLOAT)-1.;
    alpk = (FLOAT)1.;
    for (i=track_x; i<L_SUBFR; i+=STEP) {
        ps1 = *ps + dn[i];
        alp1 = *alp + *p0++;
        pos = -1;
        for (j=track_y; j<L_SUBFR; j+=STEP) {
            ps2 = ps1 + dn[j];
            alp2 = alp1 +  *p1++ + *p2++;
            sq = (ps2 * ps2);
            s = (alpk*sq)-(sqk*alp2);
            if (s > (FLOAT)0.) {
                sqk = sq;
                alpk = alp2;
                pos = j;
            }
        }
        p1 -= NB_POS;
        if (pos >= 0) {
            *ix = i;
            *iy = pos;
        }
    }
    *ps += dn[*ix] + dn[*iy];
    *alp = alpk;
    
    return;
}

/*-------------------------------------------------------------------*
* Function  set_sign()                                              *
* ~~~~~~~~~~~~~~~~~~~~                                              *
* Set the sign of each pulse position.                              *
*-------------------------------------------------------------------*/
static void set_sign(
    FLOAT cn[],       /* (i) : residual after long term prediction    */
    FLOAT dn[],       /* (i) : correlation between target and h[]     */
    FLOAT sign[],     /* (o) : sign vector (sign of each position)    */
    FLOAT inv_sign[], /* (o) : inverse of sign[]                      */
    int pos_max[],    /* (o) : pos of max of correlation              */
    FLOAT corr[]      /* (o) : correlation of each track              */
)
{
    int i, k, pos;
    FLOAT k_cn, k_dn, val;
    FLOAT s, max;
    
    /* calculate energy for normalization of cn[] and dn[] */
    s = (FLOAT)0.;
    for (i=0; i<L_SUBFR; i++) s += cn[i] * cn[i];
    if (s < (F)0.01) s = (F)0.01;
    k_cn = (F)1./(FLOAT)sqrt((double)s);

    s = (FLOAT)0.;
    for (i=0; i<L_SUBFR; i++) s += dn[i] * dn[i];
    if (s < (F)0.01) s = (F)0.01;
    k_dn = (F)1./(FLOAT)sqrt((double)s);
    
    /* set sign according to en[] = k_cn*cn[] + k_dn*dn[]    */

    /* find position of maximum of correlation in each track */
    pos = 0; /* to avoid visual warning */
    for (k=0; k<NB_TRACK; k++) {
        max = (FLOAT)-1.;
        for (i=k; i<L_SUBFR; i+=STEP) {
            val = dn[i];
            s = (k_cn* cn[i])+ (k_dn* val);
            if (s >= (FLOAT)0.) {
                sign[i] = (FLOAT)1.;
                inv_sign[i] = (FLOAT)-1.;
            }
            else {
                sign[i] = (FLOAT)-1.;
                inv_sign[i] = (FLOAT)1.;
                val = -val;
            }
            dn[i] = val;      /* modify dn[] according to the fixed sign */
            s = (FLOAT)fabs(s);
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
    FLOAT sign[],           /* (i) : sign vector                 */
    FLOAT inv_sign[],       /* (i) : inverse of sign[]           */
    FLOAT h[],              /* (o) : scaled h[]                 */
    FLOAT h_inv[],          /* (o) : inverse of scaled h[]      */
    FLOAT rrixix[][NB_POS], /* (o) energy of h[].                   */
    FLOAT rrixiy[][MSIZE]   /* (o) correlation between 2 pulses.    */
)
{
    int i, j, k, pos;
    FLOAT *ptr_h1, *ptr_h2, *ptr_hf, *psign;
    FLOAT *p0, *p1, *p2, *p3, *p4;
    FLOAT cor;
    
    for(i=0; i<L_SUBFR; i++) {
        h_inv[i] = -h[i];
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
    cor    = (FLOAT)0.;
    for(i=0; i<NB_POS; i++) {
        cor += (*ptr_h1) * (*ptr_h1); ptr_h1++;
        *p4-- = cor;
        cor += (*ptr_h1) * (*ptr_h1); ptr_h1++;
        *p3-- = cor;
        cor += (*ptr_h1) * (*ptr_h1); ptr_h1++;
        *p2-- = cor;
        cor += (*ptr_h1) * (*ptr_h1); ptr_h1++;
        *p1-- = cor;
        cor += (*ptr_h1) * (*ptr_h1); ptr_h1++;
        *p0-- = cor;
    }
    
    /* Divide all elements of rrixix[][] by 2. */
    p0 = &rrixix[0][0];
    for(i=0; i<L_SUBFR; i++)
    {
        *p0 *= (FLOAT)0.5;
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
        cor = (FLOAT)0.;
        ptr_h1 = h;
        ptr_h2 = ptr_hf;
        for(i=k+1; i<NB_POS; i++ ) {
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p4 = cor;
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = cor;
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = cor;
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = cor;
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = cor;
            p4 -= (NB_POS+1);
            p3 -= (NB_POS+1);
            p2 -= (NB_POS+1);
            p1 -= (NB_POS+1);
            p0 -= (NB_POS+1);
        }
        
        cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
        *p4 = cor;
        cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = cor;
        cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
        *p2 = cor;
        cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
        *p1 = cor;
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
        
        cor = (FLOAT)0.;
        ptr_h1 = h;
        ptr_h2 = ptr_hf;
        for(i=k+1; i<NB_POS; i++ ) {
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p4 = cor;
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = cor;
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = cor;
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = cor;
            cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = cor;
            
            p4 -= (NB_POS+1);
            p3 -= (NB_POS+1);
            p2 -= (NB_POS+1);
            p1 -= (NB_POS+1);
            p0 -= (NB_POS+1);
        }
        cor += (*ptr_h1) * (*ptr_h2); ptr_h1++; ptr_h2++;
        *p4 = cor;
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
            for(j=ipos[k+1]; j<L_SUBFR; j+= STEP) {
                *p0 *= psign[j]; p0++;
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
    int codvec[],      /* (i) : positions of each pulse */
    FLOAT sign[],      /* (i) : sign vector             */
    int nb_of_pulse,   /* (i) : number of pulses        */
    FLOAT h[],         /* (i) : impulse response of weighted synthesis filter */
    FLOAT code[],      /* (o) : algebraic (fixed) codebook excitation         */
    FLOAT y[],         /* (o) : filtered fixed codebook excitation            */
    int indx[]         /* (o) : index of pulses (5 words, 1 per track).       */
)
{
    int i, j, k, index, track;

    for (i=0; i<L_SUBFR; i++) {
        code[i] = (FLOAT)0.;
        y[i] = (FLOAT)0.;
    }

    for (i=0; i<NB_TRACK; i++) indx[i] = -1;

    for (k=0; k<nb_of_pulse; k++) {
        i = codvec[k];          /* read pulse position */
        index = i/5;
        /* track = pos%5 */
        track = i%5;
        /* codeword & filtered codeword */
        if (sign[i] > 0) {
            code[i] += (FLOAT)1.;
            for (i=codvec[k], j=0; i<L_SUBFR; i++, j++) y[i] += h[j];
        }
        else {
            code[i] -= (FLOAT)1.;
            index += 8;
            for (i=codvec[k], j=0; i<L_SUBFR; i++, j++) y[i] -= h[j];
        }

        /* quantize position & sign */
        if (indx[track] < 0) {
            indx[track] = index;
        }
        else {
            if (((index ^ indx[track]) & 8) == 0) {
                /* sign of 1st pulse == sign of 2th pulse */
                if ( indx[track]<=index ) {
                    indx[track] = (indx[track]<< (int)4)+ index;
                    indx[track] |= (int)256;
                }
                else {
                    indx[track] = (index<< (int)4)+ indx[track];
                    indx[track] |= (int)256;
                }
            }
            else {
                /* sign of 1st pulse != sign of 2th pulse */
                if ( (int)(indx[track] & (int)7) <=
                    (int)(index & (int)7) )  {
                    indx[track] = (index<< (int)4)+ indx[track];
                    indx[track] |= (int)256;
                }
                else {
                    indx[track] = (indx[track]<< (int)4)+ index;
                    indx[track] |= (int)256;
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
static int pack3(int index1, int index2, int index3)
{
    int k, index, tmp;
    
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
    
    k = (int)(index1>> 1) & (int)4;
    k += (int)(index2>> 2) & (int)2;
    k +=  (int)(index3>>3) & (int)1;
    index = 0; /* to avoid visual warning */
    switch (k) {
    case 0:
    case 7:
        index = (int)(index1 & (int)7) << (int)7;
        index += (int)(index2 & (int)7) << (int)4;
        index += index3;
        break;
    case 1:
    case 6:
        index = (int)(index3 & (int)7) << (int)7;
        index +=  (int)(index1 & (int)7) << (int)4;
        index +=  index2;
        break;
    case 2:
    case 5:
        index = (int)(index2 & (int)7) << (int)7;
        index += (int)(index1 & (int)7) << (int)4;
        index +=  index3;
        break;
    case 3:
    case 4:
        index = (int)(index2 & (int)7) << (int)7;
        index += (int)(index3 & (int)7)<< (int)4;
        index += index1;
        break;
    }
    
    return (index);
}
