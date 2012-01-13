/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/


/*
 File : LSPDECE.C
 */

#include <math.h>
#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "tab_ld8k.h"
#include "tabld8cp.h"

/* Prototype definitions of static functions */
static void lsp_iqua_cse(int prm[], FLOAT lsp_q[], int erase,
    FLOAT freq_prev[MA_NP][M], FLOAT prev_lsp[], int *prev_ma);


/*----------------------------------------------------------------------------
 * Lsp_decw_reset -   set the previous LSP vectors
 *----------------------------------------------------------------------------
 */
void lsp_decw_resete(
    FLOAT freq_prev[MA_NP][M],   /* (o) : previous LSP MA vector        */
    FLOAT prev_lsp[],            /* (o) : previous LSP vector        */
    int *prev_ma               /* previous MA prediction coef.*/
)
{
    int  i;
    
    for(i=0; i<MA_NP; i++)
        copy (freq_prev_reset, &freq_prev[i][0], M );
    
    *prev_ma = 0;
    
    copy (freq_prev_reset, prev_lsp, M );
    
    return;
}


/*----------------------------------------------------------------------------
* lsp_iqua_cs -  LSP main quantization routine
*----------------------------------------------------------------------------
*/
static void lsp_iqua_cse(
    int    prm[],          /* input : codes of the selected LSP */
    FLOAT  lsp_q[],        /* output: Quantized LSP parameters  */
    int    erase,          /* input : frame erase information   */
    FLOAT freq_prev[MA_NP][M],   /* (i/o) : previous LSP MA vector        */
    FLOAT prev_lsp[],            /* (i/o) : previous LSP vector        */
    int *prev_ma               /* (i/o) previous MA prediction coef.*/
)
{
    int  mode_index;
    int  code0;
    int  code1;
    int  code2;
    FLOAT buf[M];
    
    
    if(erase==0)                 /* Not frame erasure */
    {
        mode_index = (prm[0] >> NC0_B) & 1;
        code0 = prm[0] & (INT16)(NC0 - 1);
        code1 = (prm[1] >> NC1_B) & (INT16)(NC1 - 1);
        code2 = prm[1] & (INT16)(NC1 - 1);
        
        lsp_get_quante(lspcb1, lspcb2, code0, code1, code2, fg[mode_index],
            freq_prev, lsp_q, fg_sum[mode_index], buf);
        
        copy(lsp_q, prev_lsp, M );
        *prev_ma = mode_index;
    }
    else                         /* Frame erased */
    {
        copy(prev_lsp, lsp_q, M );
        
        /* update freq_prev */
        lsp_prev_extract(prev_lsp, buf,
            fg[*prev_ma], freq_prev, fg_sum_inv[*prev_ma]);
    }
    lsp_prev_update(buf, freq_prev);
    return;
}
/*----------------------------------------------------------------------------
* d_lsp - decode lsp parameters
*----------------------------------------------------------------------------
*/
void d_lspe(
    int prm[],                 /* (i)  : indexes of the selected LSP */
    FLOAT lsp_q[],             /* (o)  : Quantized LSP parameters    */
    int erase,                 /* (i)  : frame erase information     */
    FLOAT freq_prev[MA_NP][M], /* (i/o): previous LSP MA vector      */
    FLOAT prev_lsp[],          /* (i/o): previous LSP vector         */
    int *prev_ma             /* (i/o): previous MA prediction coef.*/
)
{
    int i;
    lsp_iqua_cse(prm, lsp_q, erase, freq_prev, prev_lsp, prev_ma);
    
    /* Convert LSFs to LSPs */
    
    for (i=0; i<M; i++ )
        lsp_q[i] = (FLOAT)cos(lsp_q[i]);
    
    return;
}


