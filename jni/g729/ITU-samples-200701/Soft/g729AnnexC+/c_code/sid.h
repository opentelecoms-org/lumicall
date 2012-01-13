/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/
/*
 File : SID.H
*/

#define         TRUE 1
#define         FALSE 0
#define         sqr(a)  ((a)*(a))
#define         R_LSFQ 10

void init_lsfq_noise(void);
void lsfq_noise(FLOAT *lsp_new, FLOAT *lspq,
                FLOAT freq_prev[MA_NP][M], int *idx);
void sid_lsfq_decode(int *index, FLOAT *lspq,
                     FLOAT freq_prev[MA_NP][M]);





