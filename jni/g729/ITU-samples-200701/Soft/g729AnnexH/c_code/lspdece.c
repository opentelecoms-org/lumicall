/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.2    Last modified: May 1998 */
/* from lspdec.c G.729 Version 3.3            */

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8e.h"
#include "tab_ld8k.h"
#include "tab_ld8e.h"

/*----------------------------------------------------------------------------
 * Lsp_decw_resete -   set the previous LSP vectors
 *----------------------------------------------------------------------------
 */
void Lsp_decw_resete(
  Word16 freq_prev[MA_NP][M],    /* (o) Q13 : previous LSP MA vector        */
  Word16 prev_lsp[],            /* (o) Q13 : previous LSP vector        */
  Word16 *prev_ma               /* previous MA prediction coef.*/
)
{
  Word16 i;

  for(i=0; i<MA_NP; i++)
    Copy( &freq_prev_reset[0], &freq_prev[i][0], M );

  *prev_ma = 0;

  Copy( freq_prev_reset, prev_lsp, M);
}



/*----------------------------------------------------------------------------
 * Lsp_iqua_cse -  LSP main quantization routine
 *----------------------------------------------------------------------------
 */
void Lsp_iqua_cse(
 Word16 prm[],          /* (i)     : indexes of the selected LSP */
 Word16 lsp_q[],        /* (o) Q13 : Quantized LSP parameters    */
 Word16 erase,           /* (i)     : frame erase information     */
  Word16 freq_prev[MA_NP][M],    /* (i/o) Q13 : previous LSP MA vector        */
  Word16 prev_lsp[],            /* (i/o) Q13 : previous LSP vector        */
  Word16 *prev_ma               /* (i/o) previous MA prediction coef.*/
)
{
  Word16 mode_index;
  Word16 code0;
  Word16 code1;
  Word16 code2;
  Word16 buf[M];     /* Q13 */

  if( erase==0 ) {  /* Not frame erasure */
    mode_index = shr(prm[0] ,NC0_B) & (Word16)1;
    code0 = prm[0] & (Word16)(NC0 - 1);
    code1 = shr(prm[1] ,NC1_B) & (Word16)(NC1 - 1);
    code2 = prm[1] & (Word16)(NC1 - 1);

    /* compose quantized LSP (lsp_q) from indexes */
    Lsp_get_quante(lspcb1, lspcb2, code0, code1, code2,
      fg[mode_index], freq_prev, lsp_q, fg_sum[mode_index], buf);

    /* save parameters to use in case of the frame erased situation */
    Copy(lsp_q, prev_lsp, M);
    *prev_ma = mode_index;
  }
  else {           /* Frame erased */
    /* use previous LSP */
    Copy(prev_lsp, lsp_q, M);

    /* update freq_prev */
    Lsp_prev_extract(prev_lsp, buf,
      fg[*prev_ma], freq_prev, fg_sum_inv[*prev_ma]);

  }
  Lsp_prev_update(buf, freq_prev);
  return;
}



/*-------------------------------------------------------------------*
 * Function  D_lspe:                                                  *
 *           ~~~~~~                                                  *
 *-------------------------------------------------------------------*/

void D_lspe(
  Word16 prm[],          /* (i)     : indexes of the selected LSP */
  Word16 lsp_q[],        /* (o) Q15 : Quantized LSP parameters    */
  Word16 erase,           /* (i)     : frame erase information     */
  Word16 freq_prev[MA_NP][M],    /* (i/o) Q13 : previous LSP MA vector        */
  Word16 prev_lsp[],            /* (i/o) Q13 : previous LSP vector        */
  Word16 *prev_ma               /* (i/o) previous MA prediction coef.*/

)
{
  Word16 lsf_q[M];       /* domain 0.0<= lsf_q <PI in Q13 */


  Lsp_iqua_cse(prm, lsf_q, erase, freq_prev, prev_lsp, prev_ma);

  /* Convert LSFs to LSPs */
  Lsf_lsp2(lsf_q, lsp_q, M);

  return;
}
