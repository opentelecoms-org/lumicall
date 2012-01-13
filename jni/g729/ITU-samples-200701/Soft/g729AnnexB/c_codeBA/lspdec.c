/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

#include "typedef.h"
#include "ld8a.h"
#include "basic_op.h"
#include "tab_ld8a.h"

/* static memory */

static Word16 freq_prev[MA_NP][M];   /* Q13 */

/* static memory for frame erase operation */
static Word16 prev_ma;                  /* previous MA prediction coef.*/
static Word16 prev_lsp[M];              /* previous LSP vector         */


/*----------------------------------------------------------------------------
 * Lsp_decw_reset -   set the previous LSP vectors
 *----------------------------------------------------------------------------
 */
void Lsp_decw_reset(
  void
)
{
  Word16 i;

  for(i=0; i<MA_NP; i++)
    Copy( &freq_prev_reset[0], &freq_prev[i][0], M );

  prev_ma = 0;

  Copy( freq_prev_reset, prev_lsp, M);
}



/*----------------------------------------------------------------------------
 * Lsp_iqua_cs -  LSP main quantization routine
 *----------------------------------------------------------------------------
 */
void Lsp_iqua_cs(
 Word16 prm[],          /* (i)     : indexes of the selected LSP */
 Word16 lsp_q[],        /* (o) Q13 : Quantized LSP parameters    */
 Word16 erase           /* (i)     : frame erase information     */
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

    Lsp_get_quant(lspcb1, lspcb2, code0, code1, code2,
      fg[mode_index], freq_prev, lsp_q, fg_sum[mode_index]);

    /* save parameters to use in case of the frame erased situation */

    Copy(lsp_q, prev_lsp, M);
    prev_ma = mode_index;
  }
  else {           /* Frame erased */
    /* use revious LSP */

    Copy(prev_lsp, lsp_q, M);

    /* update freq_prev */

    Lsp_prev_extract(prev_lsp, buf,
      fg[prev_ma], freq_prev, fg_sum_inv[prev_ma]);
    Lsp_prev_update(buf, freq_prev);
  }

  return;
}



/*-------------------------------------------------------------------*
 * Function  D_lsp:                                                  *
 *           ~~~~~~                                                  *
 *-------------------------------------------------------------------*/

void D_lsp(
  Word16 prm[],          /* (i)     : indexes of the selected LSP */
  Word16 lsp_q[],        /* (o) Q15 : Quantized LSP parameters    */
  Word16 erase           /* (i)     : frame erase information     */
)
{
  Word16 lsf_q[M];       /* domain 0.0<= lsf_q <PI in Q13 */


  Lsp_iqua_cs(prm, lsf_q, erase);

  /* Convert LSFs to LSPs */

  Lsf_lsp2(lsf_q, lsp_q, M);

  return;
}

void Get_decfreq_prev(Word16 x[MA_NP][M])
{
  Word16 i;

  for (i=0; i<MA_NP; i++)
    Copy(&freq_prev[i][0], &x[i][0], M);
}
  
void Update_decfreq_prev(Word16 x[MA_NP][M])
{
  Word16 i;

  for (i=0; i<MA_NP; i++)
    Copy(&x[i][0], &freq_prev[i][0], M);
}



