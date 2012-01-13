/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX D   Version 1.1    Last modified: March 1998 */

extern Word16 CODEC_MODE;
extern Word16 toggleFlag;
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

extern Word16 bitsno_6k[PRM_SIZE_6K];
