/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C+ floating point ANSI C source code
   Copyright (C) 1999, AT&T, France Telecom, NTT, University of
   Sherbrooke, Rockwell International, Ericsson. All rights reserved.
----------------------------------------------------------------------
*/
/*
 File : TABLD8CP.C
*/

extern int bitsno_B[PRM_SIZE_SID];
extern int bitsno_D[PRM_SIZE_D];
extern int bitsno_E_fwd[PRM_SIZE_E_fwd-1];
extern int bitsno_E_bwd[PRM_SIZE_E_bwd-1];

/* 11.8k */
extern FLOAT freq_prev_reset[M];
extern FLOAT lag_bwd[M_BWD];
extern FLOAT hw[NRP+L_FRAME+M_BWD];
extern int ipos[16];

/* 6.4k (for NTT CS-VQ)*/
extern FLOAT gbk1_6k[NCODE1_6K][2];
extern FLOAT gbk2_6k[NCODE2_6K][2];
extern FLOAT coef_6k[2][2];
extern FLOAT thr1_6k[NCODE1_6K-NCAN1_6K];
extern FLOAT thr2_6k[NCODE2_6K-NCAN2_6K];
extern int map1_6k[NCODE1_6K];
extern int imap1_6k[NCODE1_6K];
extern int map2_6k[NCODE2_6K];
extern int imap2_6k[NCODE2_6K];
extern int grayEncode[32];
extern int grayDecode[32];
extern int trackTable0[16];
extern int trackTable1[32];
extern int posSearched[2];
extern FLOAT ph_imp_low[L_SUBFR];
extern FLOAT ph_imp_mid[L_SUBFR];
extern FLOAT ph_imp_high[L_SUBFR];
extern FLOAT freq_prev_reset[M];
extern FLOAT lwindow[M+2];
