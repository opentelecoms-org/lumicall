/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : TRACK_PI.C
*/

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"

/* -------------------------------------------------------------------- */
/*                            TRACK_PIT                                 */
/*                                                                      */
/*    Pitch tracking by elimination of the multiples or sub-multiples   */
/*                                                                      */
/* -------------------------------------------------------------------- */

void track_pit( 
    int *t0,         /* I/O  Integer pitch delay */
    int *t0_frac,     /* I/O  Non integer correction */
    int *prev_pitch, /* I/O  previous pitch delay */
    int *stat_pitch, /* I/O  pitch stationnarity indicator */
    int *pitch_sta,  /* I/O  stationnary integer pitch delay */
    int *frac_sta    /* I/O  stationnary fractional pitch */
)
{
    int dist, dist_min, pitch_mult;
    int j, flag_mult;
    
    dist = (*t0) - (*prev_pitch);
    if(dist < 0) {
        flag_mult = 0;
        dist = - dist;
    }
    else {
        flag_mult = 1;
    }
    /* ------------------------ */
    /* Test pitch stationnarity */
    /* ------------------------ */
    if (dist < 5) {
        (*stat_pitch)++;
        if (*stat_pitch > 7) *stat_pitch = 7 ;
        *pitch_sta = *t0;
        *frac_sta = *t0_frac;
    }
    else {
        /* ------------------------------- */
        /* Find multiples or sub-multiples */
        /* ------------------------------- */
        dist_min =  dist;
        if( flag_mult == 0) {
            pitch_mult = 2 * (*t0);
            for (j=2; j<5; j++) {
                dist = abs(pitch_mult - (*prev_pitch));
                if (dist <= dist_min) {
                    dist_min = dist;
                }
                pitch_mult += (*t0);
            }
        }
        else {
            pitch_mult = 2 * (*prev_pitch);
            for (j=2; j<5; j++) {
                dist = abs(pitch_mult - (*t0));
                if (dist <= dist_min) {
                    dist_min = dist;
                }
                pitch_mult += (*prev_pitch);
            }
        }
        if (dist_min < 5) {   /* Multiple or sub-multiple detected */
            if (*stat_pitch > 0) {
                *t0      = *pitch_sta;
                *t0_frac = *frac_sta;
            }
            *stat_pitch -= 1;
            if (*stat_pitch < 0) *stat_pitch = 0 ;
        }
        else {
            *stat_pitch = 0;    /* No (sub-)multiple detected  => Pitch transition */
            *pitch_sta = *t0;
            *frac_sta = *t0_frac;
        }
    }
    
    *prev_pitch = *t0;
    
    return;
}
