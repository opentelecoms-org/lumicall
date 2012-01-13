/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.

----------------------------------------------------------------------
*/

/*
 File : TAB_LD8K.H
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/
extern FLOAT b100[3];
extern FLOAT a100[3];
extern FLOAT b140[3];
extern FLOAT a140[3];
extern FLOAT hamwindow[L_WINDOW];
extern FLOAT grid[GRID_POINTS+1];
extern FLOAT lspcb1[NC0][M];         /*First Stage Codebook */
extern FLOAT lspcb2[NC1][M];         /*Second Stage Codebook*/
extern FLOAT fg[MODE][MA_NP][M]; /*MA prediction coef.  */
extern FLOAT fg_sum[MODE][M];        /*present MA prediction coef.*/
extern FLOAT fg_sum_inv[MODE][M];    /*inverse coef. */
extern FLOAT inter_3[FIR_SIZE_ANA];
extern FLOAT inter_3l[FIR_SIZE_SYN];
extern FLOAT pred[4];
extern FLOAT coef[2][2];
extern FLOAT thr1[NCODE1-NCAN1];
extern FLOAT thr2[NCODE2-NCAN2];
extern FLOAT gbk1[NCODE1][2];
extern FLOAT gbk2[NCODE2][2];
extern int map1[NCODE1];
extern int map2[NCODE2];
extern int imap1[NCODE1];
extern int imap2[NCODE2];
extern FLOAT tab_hup_l[SIZ_TAB_HUP_L];
extern FLOAT tab_hup_s[SIZ_TAB_HUP_S];
extern int bitsno[PRM_SIZE];
