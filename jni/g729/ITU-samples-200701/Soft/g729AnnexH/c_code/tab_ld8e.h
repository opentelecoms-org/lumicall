/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.2    Last modified: May 1998 */

extern Word16 freq_prev_reset[M];
extern Word16 lag_h_bwd[M_BWD];
extern Word16 lag_l_bwd[M_BWD];
extern Word16 bitsno_E_fwd[PRM_SIZE_E_fwd];
extern Word16 bitsno_E_bwd[PRM_SIZE_E_bwd];
extern Word16 hw[NRP+L_FRAME+M_BWD];
extern Word16 bitrates[2];
extern Word16 tab_log[17];
extern Word16 ipos[16];
