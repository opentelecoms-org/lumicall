/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* Version 1.2    Last modified: May 1998 */

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8e.h"

/* -------------------------------------------------------------------- */
/*                            TRACK_PIT                                 */
/*                                                                      */
/*    Pitch tracking by elimination of the multiples or sub-multiples   */
/*                                                                      */
/* -------------------------------------------------------------------- */

void track_pit( Word16 *T0,         /* I/O  Integer pitch delay */
                Word16 *T0_frac,     /* I/O  Non integer correction */
                Word16 *prev_pitch, /* I/O  previous pitch delay */
                Word16 *stat_pitch, /* I/O  pitch stationnarity indicator */
                Word16 *pitch_sta,  /* I/O  stationnary integer pitch delay */
                Word16 *frac_sta    /* I/O  stationnary fractional pitch */
)

{

    Word16 dist, dist_min, pitch_mult, temp;
    int     j, flag_mult;

    dist = sub(*T0, *prev_pitch);
    if(dist < 0) {
        flag_mult = 0;
        dist = negate(dist);
    }
    else {
        flag_mult = 1;
    }
    /* ------------------------ */
    /* Test pitch stationnarity */
    /* ------------------------ */

    if (dist < 5) {
      *stat_pitch += 1;
      if (*stat_pitch > 7) *stat_pitch = 7 ;
      *pitch_sta = *T0;
      *frac_sta = *T0_frac;
    }
    else {

      /* ------------------------------- */
      /* Find multiples or sub-multiples */
      /* ------------------------------- */
        dist_min =  dist;
        if( flag_mult == 0) {
            pitch_mult = add(*T0, *T0);
            for (j=2; j<5; j++) {
                dist = abs_s(sub(pitch_mult, *prev_pitch));
                temp = sub(dist, dist_min);
                if (temp <= 0) {
                    dist_min = dist;
                }
                pitch_mult = add(*T0, pitch_mult);
            }
        }
        else {
            pitch_mult = add(*prev_pitch, *prev_pitch);
            for (j=2; j<5; j++) {
                dist = abs_s(sub(pitch_mult, *T0));
                temp = sub(dist, dist_min);
                if (temp <= 0) {
                    dist_min = dist;
                }
                pitch_mult = add(*prev_pitch, pitch_mult);
            }
        }
        if (dist_min < 5) {   /* Multiple or sub-multiple detected */
            if (*stat_pitch > 0) {
                *T0 = *pitch_sta;
                *T0_frac = *frac_sta;
            }
            *stat_pitch -= 1;
            if (*stat_pitch < 0) *stat_pitch = 0 ;
        }
        else {
            *stat_pitch = 0;    /* No (sub-)multiple detected  => Pitch transition */
            *pitch_sta = *T0;
            *frac_sta = *T0_frac;
        }
    }

    *prev_pitch = *T0;

    return;
}
