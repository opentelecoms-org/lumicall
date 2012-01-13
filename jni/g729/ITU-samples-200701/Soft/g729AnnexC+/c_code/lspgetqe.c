/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : LSPGETQE.C
*/

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"

/*----------------------------------------------------------------------------
 * lsp_get_quant - reconstruct quantized LSP parameter and check the stabilty
 *----------------------------------------------------------------------------
 */
void lsp_get_quante(
    FLOAT  lspcb1[][M],    /*input : first stage LSP codebook     */
    FLOAT  lspcb2[][M],    /*input : Second stage LSP codebook    */
    int    code0,          /*input : selected code of first stage */
    int    code1,          /*input : selected code of second stage*/
    int    code2,          /*input : selected code of second stage*/
    FLOAT  fg[][M],        /*input : MA prediction coef.          */
    FLOAT  freq_prev[][M], /*input : previous LSP vector          */
    FLOAT  lspq[],         /*output: quantized LSP parameters     */
    FLOAT  fg_sum[],        /*input : present MA prediction coef.  */
    FLOAT freq_cur[]        /* (o)  : current LSP MA vector        */
)
{
    int  j;
    FLOAT  buf[M];
    
    for(j=0; j<NC; j++)
        buf[j] = lspcb1[code0][j] + lspcb2[code1][j];
    for(j=NC; j<M; j++)
        buf[j] = lspcb1[code0][j] + lspcb2[code2][j];
    
    /* check */
    lsp_expand_1_2(buf, GAP1);
    lsp_expand_1_2(buf, GAP2);
    
    /* reconstruct quantized LSP parameters */
    lsp_prev_compose(buf, lspq, fg, freq_prev, fg_sum);
    
    /*lsp_prev_update(buf, freq_prev);*/
    copy(buf, freq_cur, M);
    
    lsp_stability( lspq );  /* check the stabilty */
    
    return;
}

/*----------------------------------------------------------------------------
* lsp_expand_1  - check for lower (0-4)
*----------------------------------------------------------------------------
*/
void lsp_expand_1(
    FLOAT  buf[],          /* in/out: lsp vectors  */
    FLOAT gap
)
{
    int   j;
    FLOAT diff, tmp;
    
    for(j=1; j<NC; j++) {
        diff = buf[j-1] - buf[j];
        tmp  = (diff + gap) * (F)0.5;
        if(tmp >  0) {
            buf[j-1] -= tmp;
            buf[j]   += tmp;
        }
    }
    return;
}

/*----------------------------------------------------------------------------
* lsp_expand_2 - check for higher (5-9)
*----------------------------------------------------------------------------
*/
void lsp_expand_2(
    FLOAT  buf[],          /*in/out: lsp vectors  */
    FLOAT gap
)
{
    int   j;
    FLOAT diff, tmp;
    
    for(j=NC; j<M; j++) {
        diff = buf[j-1] - buf[j];
        tmp  = (diff + gap) * (F)0.5;
        if(tmp >  0) {
            buf[j-1] -= tmp;
            buf[j]   += tmp;
        }
    }
    return;
}

/*----------------------------------------------------------------------------
* lsp_expand_1_2 - ..
*----------------------------------------------------------------------------
*/
void lsp_expand_1_2(
    FLOAT  buf[],          /*in/out: LSP parameters  */
    FLOAT  gap             /*input      */
)
{
    int   j;
    FLOAT diff, tmp;
    
    for(j=1; j<M; j++) {
        diff = buf[j-1] - buf[j];
        tmp  = (diff + gap) * (F)0.5;
        if(tmp >  0) {
            buf[j-1] -= tmp;
            buf[j]   += tmp;
        }
    }
    return;
}

/*
Functions which use previous LSP parameter (freq_prev).
*/

/*
compose LSP parameter from elementary LSP with previous LSP.
*/
void lsp_prev_compose(
    FLOAT lsp_ele[],             /* (i)  : LSP vectors                 */
    FLOAT lsp[],                 /* (o)  : quantized LSP parameters    */
    FLOAT fg[][M],               /* (i)  : MA prediction coef.         */
    FLOAT freq_prev[][M],        /* (i)  : previous LSP vector         */
    FLOAT fg_sum[]               /* (i)  : present MA prediction coef. */
)
{
    int j, k;
    
    for(j=0; j<M; j++) {
        lsp[j] = lsp_ele[j] * fg_sum[j];
        for(k=0; k<MA_NP; k++) lsp[j] += freq_prev[k][j]*fg[k][j];
    }
    return;
}

/*
extract elementary LSP from composed LSP with previous LSP
*/
void lsp_prev_extract(
    FLOAT lsp[M],                /* (i)  : unquantized LSP parameters  */
    FLOAT lsp_ele[M],            /* (o)  : target vector               */
    FLOAT fg[MA_NP][M],          /* (i)  : MA prediction coef.         */
    FLOAT freq_prev[MA_NP][M],   /* (i)  : previous LSP vector         */
    FLOAT fg_sum_inv[M]          /* (i)  : inverse previous LSP vector */
)
{
    int j, k;
    
    /*----- compute target vectors for each MA coef.-----*/
    for( j = 0 ; j < M ; j++ ) {
        lsp_ele[j]=lsp[j];
        for ( k = 0 ; k < MA_NP ; k++ )
            lsp_ele[j] -= freq_prev[k][j] * fg[k][j];
        lsp_ele[j] *= fg_sum_inv[j];
    }
    
    return;
}
/*
update previous LSP parameter
*/
void lsp_prev_update(
    FLOAT lsp_ele[M],             /* input : LSP vectors           */
    FLOAT freq_prev[MA_NP][M]     /* input/output: previous LSP vectors  */
)
{
    int k;
    
    for ( k = MA_NP-1 ; k > 0 ; k-- )
        copy(freq_prev[k-1], freq_prev[k], M);
    
    copy(lsp_ele, freq_prev[0], M);
    return;
}
/*----------------------------------------------------------------------------
* lsp_stability - check stability of lsp coefficients
*----------------------------------------------------------------------------
*/
void lsp_stability(
    FLOAT  buf[]           /*in/out: LSP parameters  */
)
{
    int   j;
    FLOAT diff, tmp;
    
    for(j=0; j<M-1; j++) {
        diff = buf[j+1] - buf[j];
        if( diff < (F)0. ) {
            tmp      = buf[j+1];
            buf[j+1] = buf[j];
            buf[j]   = tmp;
        }
    }
    
    if( buf[0] < L_LIMIT ) {
        buf[0] = L_LIMIT;
        printf("warning LSP Low \n");
    }
    for(j=0; j<M-1; j++) {
        diff = buf[j+1] - buf[j];
        if( diff < GAP3 ) {
            buf[j+1] = buf[j]+ GAP3;
        }
    }
    if( buf[M-1] > M_LIMIT ) {
        buf[M-1] = M_LIMIT;
        printf("warning LSP High \n");
    }
    return;
}
