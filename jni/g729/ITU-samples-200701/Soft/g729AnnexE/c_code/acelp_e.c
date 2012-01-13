/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.2    Last modified: May 1998 */

/* ----------------------------------------------------------------- */
/*   New ACELP dictionnaries for higher bit rate extension of G729   */
/*                      Bit rate : 11.8 kb/s                         */
/*                                                                   */
/*   (C) Copyright 1997 : France Telecom / Universite de Sherbrooke  */
/*   All rights reserved.                                            */
/*                                                                   */
/* ----------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8e.h"
#include "tab_ld8e.h"

/* locals functions */
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
