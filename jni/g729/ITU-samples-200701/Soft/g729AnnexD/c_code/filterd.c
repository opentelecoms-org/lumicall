/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX D   Version 1.3    Last modified: Oct 2000 */

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"
#include "ld8kd.h"
#include "tab_ld8k.h"
#include "tabld8kd.h"

/*--------------------------------------------------------*
 *         Static memory allocation.                      *
 *--------------------------------------------------------*/

static Word16 gainMem[6] = {0, 0, 0, 0, 0, 0};
static Word16 prevState = 0;
static Word16 prevCbGain = 0;
static Word16 onset = 0;

/*-------------------------------------------------------------------*
 * Function  Convolve:                                               *
 *           ~~~~~~~~~                                               *
 *-------------------------------------------------------------------*
 * Perform the convolution between two vectors x[] and h[] and       *
 * write the result in the vector y[].                               *
 * All vectors are of length N.                                      *
 *-------------------------------------------------------------------*/

void Convolve(
  Word16 x[],      /* (i)     : input vector                           */
  Word16 h[],      /* (i) Q12 : impulse response                       */
  Word16 y[],      /* (o)     : output vector                          */
  Word16 L         /* (i)     : vector size                            */
)
{
   Word16 i, n;
   Word32 s;

   for (n = 0; n < L; n++)
   {
     s = 0;
     for (i = 0; i <= n; i++)
       s = L_mac(s, x[i], h[n-i]);

     s    = L_shl(s, 3);                   /* h is in Q12 and saturation */
     y[n] = extract_h(s);
   }

   return;
}

/*-----------------------------------------------------*
 * procedure Syn_filt:                                 *
 *           ~~~~~~~~                                  *
 * Do the synthesis filtering 1/A(z).                  *
 *-----------------------------------------------------*/


void Syn_filt(
  Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
  Word16 x[],     /* (i)     : input signal                             */
  Word16 y[],     /* (o)     : output signal                            */
  Word16 lg,      /* (i)     : size of filtering                        */
  Word16 mem[],   /* (i/o)   : memory associated with this filtering.   */
  Word16 update   /* (i)     : 0=no update, 1=update of memory.         */
)
{
  Word16 i, j;
  Word32 s;
  Word16 tmp[80];     /* This is usually done by memory allocation (lg+M) */
  Word16 *yy;

  /* Copy mem[] to yy[] */

  yy = tmp;

  for(i=0; i<M; i++)
  {
    *yy++ = mem[i];
  }

  /* Do the filtering. */

  for (i = 0; i < lg; i++)
  {
    s = L_mult(x[i], a[0]);
    for (j = 1; j <= M; j++)
      s = L_msu(s, a[j], yy[-j]);

    s = L_shl(s, 3);
    *yy++ = round(s);
  }

  for(i=0; i<lg; i++)
  {
    y[i] = tmp[i+M];
  }

  /* Update of memory if update==1 */

  if(update != 0)
     for (i = 0; i < M; i++)
     {
       mem[i] = y[lg-M+i];
     }

  return;
}

/*-----------------------------------------------------------*
 * Update64 - Updates state machine for phase dispersion     *
 * in 6.4 kbps mode, when running in 8.0 kbps mode.          *
 *-----------------------------------------------------------*/
void Update64(
  Word16 ltpGain,   /* (i) Q14 : pitch gain                  */
  Word16 cbGain     /* (i) Q1  : codebook gain               */
)
{
  Word16 i;

  for (i = 5; i > 0; i--) gainMem[i] = gainMem[i-1];
  gainMem[0] = ltpGain;
  prevState = 2;
  prevCbGain = cbGain;

  onset = 0;

  return;
}



/*-----------------------------------------------------------*
 * syn_filt64 - filter with synthesis filter 1/A(z)            *
 *-----------------------------------------------------------*/

void Syn_filt64(
 Word16 a[],     /* (i)   Q12 : predictor coefficients a[0:m]    */
 Word16 x[],     /* (i)   Q0  : excitation signal                */
 Word16 y[],     /* (o)   Q0  : filtered output signal           */
 Word16 lg,      /* (i)       : vector dimension                 */
 Word16 mem[],   /* (i/o) Q0  : filter memory                    */
 Word16 update,  /* (i)       : 0 = no memory update, 1 = update */
 Word16 cbGain,  /* (i)   Q1  : Codebook gain                    */
 Word16 ltpGain, /* (i)   Q14 : LTP gain                         */
 Word16 inno[]   /* (i)   Q13 : Innovation vector                */
)
{
   Word16 i,j;
   Word32 s;
   Word16 tmp[80];     /* This is usually done by memory allocation (lg+M) */
   Word16 tmp1;
   
   Word16 *yy;

   Word16 ScaledLtpEx[L_SUBFR];
   Word16 newTotEx[L_SUBFR];
   Word16 inno_sav[L_SUBFR];
   Word16 ps_poss[L_SUBFR];
   Word16 nze, iii, nPulse, i1, j1, i2, j2, ppos;
   Word16 dState, testState;
   
   /* phase dispersion: ph_imp_low[] and ph_imp_mid[] are given in
      tab_ld8k.c and All-Pass filter ph_imp_high[] is calculated here */

   Word16 *ph_imp;   /* Pointer to phase dispersion filter */
   Word16 ph_imp_high[L_SUBFR];

   ph_imp_high[0]=32767;
   for (i = 1; i < L_SUBFR;  i++) ph_imp_high[i] = 0;
   
   for (i = 0; i < L_SUBFR;  i++) {
       /* ScaledLtpEx[i] = x[i] - cbGain*inno[i] */
       
       tmp1 = round(L_shl(L_mult(cbGain, inno[i]), 1));
       ScaledLtpEx[i] = sub(x[i], tmp1);
       inno_sav[i] = inno[i];
       inno[i] = 0;
   }
   
   nze=0;
   for (iii=0; iii<L_SUBFR; iii++)
       if (inno_sav[iii] != 0)
           ps_poss[nze++] = iii;

   if (sub(ltpGain, 14745) < 0) {    /* if (ltpGain < 0.9) */
       if (sub(ltpGain, 9830) > 0) { /* if (ltpGain > 0.6 */
	   dState = 1;
       }
       else {
	   dState = 0;
       }
   }
   else {
     dState = 2;
   }

   for (i = 5; i > 0; i--) gainMem[i] = gainMem[i-1];
   gainMem[0] = ltpGain;

   if (sub(shr(cbGain, 1), prevCbGain) > 0)
     onset = 2;
   else {
     onset--;
     if (onset < 0) onset = 0;
   }

   i1 = 0;
   for (i = 0; i < 6; i++)
     if (sub(gainMem[i], 9830) < 0) i1 += 1;
   if (sub(i1, 2) > 0)
     if (!onset) dState = 0;

   testState = add(prevState, 1);
   if (sub(dState, testState) > 0)
     if (!onset) dState -= 1;

   if (onset)
     if (sub(dState, 2) < 0) dState++;

   prevState = dState;
   prevCbGain = cbGain;

   if (sub(dState, 2) < 0) {
     if (dState) {
       ph_imp = ph_imp_mid;
     }
     else {
       ph_imp = ph_imp_low;
     }
   }
   else {
     ph_imp = ph_imp_high;
   }
   
   for (nPulse=0; nPulse<nze; nPulse++) {
     ppos = ps_poss[nPulse];
      
     for (i1=ppos, j1=0; i1<L_SUBFR; i1++, j1++) {
       /* inno[i1] += inno_sav[ppos] * ph_imp[i1-ppos] */
       tmp1 = mult(inno_sav[ppos], ph_imp[j1]);
       inno[i1] = add(inno[i1], tmp1);
     }    
  
     j2=sub(L_SUBFR, ppos);
     for (i2=0; i2 < ppos; i2++, j2++) {
       /* inno[i2] += inno_sav[ppos] * ph_imp[L_SUBFR-ppos+i2] */
       tmp1 = mult(inno_sav[ppos], ph_imp[L_SUBFR-ppos+i2]);
       inno[i2] = add(inno[i2], tmp1);
     }
   }

   for (i = 0; i < L_SUBFR;  i++) {
     /* newTotEx[i] = ScaledLtpEx[i] + cbGain*inno[i]; */
     tmp1 = round(L_shl(L_mult(cbGain, inno[i]), 1));
     newTotEx[i] = add(ScaledLtpEx[i], tmp1);
   }

   /* Copy mem[] to yy[] */

   yy = tmp;
   
   for(i=0; i<M; i++)
   {
       *yy++ = mem[i];
   }

   /* Filtering */

   for (i = 0; i < lg; i++)
   {
       s = L_mult(newTotEx[i], a[0]);
       for (j = 1; j <= M; j++)
           s = L_msu(s, a[j], yy[-j]);
       
       s = L_shl(s, 3);
       *yy++ = round(s);
   }

   for(i=0; i<lg; i++)
   {
       y[i] = tmp[i+M];
   }
   
   /* Update of memory if update==1 */
   
  if(update != 0)
      for (i = 0; i < M; i++)
      {
          mem[i] = y[lg-M+i];
      }
  
  return;
}

/*-----------------------------------------------------------------------*
 * procedure Residu:                                                     *
 *           ~~~~~~                                                      *
 * Compute the LPC residual  by filtering the input speech through A(z)  *
 *-----------------------------------------------------------------------*/

void Residu(
  Word16 a[],    /* (i) Q12 : prediction coefficients                     */
  Word16 x[],    /* (i)     : speech (values x[-m..-1] are needed         */
  Word16 y[],    /* (o)     : residual signal                             */
  Word16 lg      /* (i)     : size of filtering                           */
)
{
  Word16 i, j;
  Word32 s;

  for (i = 0; i < lg; i++)
  {
    s = L_mult(x[i], a[0]);
    for (j = 1; j <= M; j++)
      s = L_mac(s, a[j], x[i-j]);

    s = L_shl(s, 3);
    y[i] = round(s);
  }
  return;
}
