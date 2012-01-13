/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
    ITU-T G.729 Annex I  - Reference C code for fixed point
                         implementation of G.729 Annex I
                         Version 1.1 of October 1999
*/
/*
 File : tabld8cp.h
 */

/* from tab_ld8e.h G.729 Annex E Version 1.2  Last modified: May 1998 */
/* from tabld8kd.h G.729 Annex D Version 1.2  Last modified: May 1998 */
/* from tab_ld8k.h G.729 Annex B Version 1.3  Last modified: August 1997 */

extern Word16 freq_prev_reset[M];
extern Word16 lag_h_B[M+2];
extern Word16 lag_l_B[M+2];
extern Word16 lag_h_bwd[M_BWD];
extern Word16 lag_l_bwd[M_BWD];
extern Word16 bitsno_B[PRM_SIZE_SID];
extern Word16 bitsno_D[PRM_SIZE_D];
extern Word16 bitsno_E_fwd[PRM_SIZE_E_fwd];
extern Word16 bitsno_E_bwd[PRM_SIZE_E_bwd];
extern Word16 hw[NRP+L_FRAME+M_BWD];
extern Word16 bitrates[2];
extern Word16 tab_log[17];
extern Word16 ipos[16];

extern Word16 gbk1_6k[NCODE1_6K][2];
extern Word16 gbk2_6k[NCODE2_6K][2];
extern Word16 coef_6k[2][2];
extern Word32 L_coef_6k[2][2];
extern Word16 thr1_6k[NCODE1_6K-NCAN1_6K];
extern Word16 thr2_6k[NCODE2_6K-NCAN2_6K];
extern Word16 map1_6k[NCODE1_6K];
extern Word16 imap1_6k[NCODE1_6K];
extern Word16 map2_6k[NCODE2_6K];
extern Word16 imap2_6k[NCODE2_6K];
extern Word16 trackTable0[16];
extern Word16 trackTable1[32];
extern Word16 grayEncode[32];
extern Word16 grayDecode[32];
extern Word16 ph_imp_low[L_SUBFR];
extern Word16 ph_imp_mid[L_SUBFR];

extern Word16 lspSid_reset[M];
