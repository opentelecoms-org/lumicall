ITU-T G.729 Software Package Release 2 (November 2006)

   ITU-T G.729 Annex B     ANSI-C Source Code
   Version 1.5    Last modified: October 2006

TITLE
-----
Fixed-point description of Recommendation G.729 with ANNEX B

Coding of Speech at 8 kbit/s using Conjugate-Structure
Algebraic-Code-Excited Linear-Prediction (CS-ACELP) with
Voice Activity Decision(VAD), Discontinuous Transmission(DTX), 
and Comfort Noise Generation(CNG).


SOFTWARE AND INTELLECTUAL PROPERTY
----------------------------------
This software package is provided as part of ITU-T Recommendation G.729B.

 Copyright (c) 1996, AT&T, France Telecom, Lucent Technologies, NTT, 
                     Rockwell International, Universite de Sherbrooke.
 All rights reserved.

The copy of the source C code, version 1.5, is given under Copyright of the 
authors, only for the purpose of establishing the specification of a codec.


VERSION
-------
This is version 1.5.

----------------------------------------------------------------------------
   Differences between Version 1.2 and Version 1.1 :

 In version 1.2, tab_dtx.h, tab_dtx.c, vad.c and calcexc.c were updated as
per Corrigendum to annex B of G.729 published in COM 16-R20 of june 1997.

----------------------------------------------------------------------------

----------------------------------------------------------------------------
----------------------------------------------------------------------------
   Differences between Version 1.3 and Version 1.2 :

file : DEC_LD8K.C
*****************
Version 1.2 lines 147 to 149 :
----------------------------------------------------------------------------
  if(bfi == 1)
    if(past_ftyp == 1) ftyp = 1;
    else ftyp = 0;
----------------------------------------------------------------------------
replaced in Version 1.3 by lines 147 to 149 :
----------------------------------------------------------------------------
  if(bfi == 1) {
    if(past_ftyp == 1) ftyp = 1;
    else ftyp = 0;
    *parm = ftyp;  /* modification introduced in version V1.3 */
  }

file : BITS.C
*************
function : read_frame()

----------------------------------------------------------------------------
Version 1.2 line 221-226 :
----------------------------------------------------------------------------
  /* the hardware detects frame erasures by checking if all bits
     are set to zero */

  parm[0] = 0;           /* No frame erasure */
  for (i=0; i < serial[1]; i++)
    if (serial[i+2] == 0 ) parm[0] = 1;  /* frame erased     */
----------------------------------------------------------------------------
replaced in Version 1.3 by lines 219-230 :
----------------------------------------------------------------------------
  /* This part was modified for version V1.3 */
  /* for speech and SID frames, the hardware detects frame erasures
     by checking if all bits are set to zero */
  /* for untransmitted frames, the hardware detects frame erasures
     by testing serial[0] */

  parm[0] = 0;           /* No frame erasure */
  if(serial[1] != 0) {
   for (i=0; i < serial[1]; i++)
     if (serial[i+2] == 0 ) parm[0] = 1;  /* frame erased     */
  }
  else if(serial[0] != SYNC_WORD) parm[0] = 1;

----------------------------------------------------------------------------
----------------------------------------------------------------------------
   Differences between Version 1.4 and Version 1.3 :

In Version 1.4 the initialization of lspSid in dec_sid.c has been updated
according to the Corrigendum to Annex B of G.729 published in COM 16-R 60-E.

In terms of functionality, only the file dec_sid.c has changed.  For 
compilation purposes the two makefiles were changed, and the version number
was changed in coder.c and decoder.c.

The changes from Version 1.3 to Version 1.4 do not affect the test vectors.

----------------------------------------------------------------------------
----------------------------------------------------------------------------
   Differences between v1.5 and v1.4 :
----------------------------------------------------------------------------

file qsidlsf.c :
****************
v1.4 lines 194-204 and 263-73:
---------------------------------------------------------
  for (q=0; q<K; q++){
    for (p=0; p<J; p++)
      for (m=0; m<MQ; m++)
        if (sub(sum[p*MQ+m], min[q]) < 0){
          min[q] = sum[p*MQ+m];
          min_indx_p[q] = p;
          min_indx_m[q] = m;
        }
    
    sum[min_indx_p[q]*MQ+min_indx_m[q]] = MAX_16;
  }
---------------------------------------------------------

are replaced in v1.5 lines 194-206 and 265-277 by :
---------------------------------------------------------
  for (q=0; q<K; q++){
    min_indx_p[q] = 0;
    min_indx_m[q] = 0;
    for (p=0; p<J; p++)
      for (m=0; m<MQ; m++)
        if (sub(sum[p*MQ+m], min[q]) < 0){
          min[q] = sum[p*MQ+m];
          min_indx_p[q] = p;
          min_indx_m[q] = m;
        }
    sum[min_indx_p[q]*MQ+min_indx_m[q]] = MAX_16;
  }

file dec_ld8k.c :
*****************

line 148 :
    if(past_ftyp == 1) ftyp = 1;

is replaced by :
    if(past_ftyp == 1) {
      ftyp = 1;
      parm[4] = 1;
    }


DESCRIPTION
-----------
  This package includes the files needed to build the fixed point version
of the G.729 codec with VAD/DTX/CNG as described in ANNEX B.

 It includes also the PC executable (coder.exe and decoder.exe), a batch file
(test.bat), speech (test.inp) and data files (test.bit and test.syn) to verify
the execution.  The binary reference files are in PC format.

SIMILARITIES AND DIFFERENCES WITH G.729
---------------------------------------

Common files with G.729
~~~~~~~~~~~~~~~~~~~~~~~
acelp_co.c
basic_op.c
de_acelp.c
dec_gain.c
dec_lag3.c
dspfunc.c
filter.c
gainpred.c
lpcfunc.c
lspgetq.c
oper_32b.c
p_parity.c
pitch.c
post_pro.c
pre_proc.c
pred_lt3.c
pwf.c
qua_gain.c
basic_op.h
oper_32b.h
typedef.h

File extracted from G.729 file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
taming.c

Files in G.729 but modified for ANNEX B
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bits.c
cod_ld8k.c
coder.c
dec_ld8k.c
decoder.c
lpc.c
lspdec.c
pst.c
qua_lsp.c
tab_ld8k.c
util.c
ld8k.h
tab_ld8k.h

Files only in ANNEX B
~~~~~~~~~~~~~~~~~~~~~
calcexc.c
dec_sid.c
dtx.c
qsidgain.c
qsidlsf.c
tab_dtx.c
vad.c
dtx.h
octet.h
sid.h
tab_dtx.h
vad.h


COMPILATION
-----------

Edit the file typedef.h to comply to your target platform


For UNIX systems the following makefiles are provided

   coder.mak
   decoder.mak



Edit the makefiles coder.mak and decoder.mak to set the proper options
for your system.
The command to compile and link all code on a UNIX system is

     make -f coder.mak
     make -f decoder.mak

For other platforms, the *.mak files can be used to work out the
compilation procedures.

This code has been successfully compiled and run on the following
platforms:

Platform                   Operating System      Compiler
-----------------------------------------------------------------------------
DEC ALPHA                  OSF/1                 DEC OSF/1 cc
SGI                        IRIX 5.2              cc
SUN                        SUNOS 4.1.3           gcc
PC                         DOS 6.2               Borland 4.02
                                                 Microsoft Quick C 2.5
                                                 Microsoft Visual C++ 1.51
                                                 Watcom 10.6
PC                         DOS 6.21              Borland 3.1
                                                 Microsoft 8
                                                 Watcom 9.6

Version 1.5 of this code has only been tested on a PC platform with Windows XP and compiled with Microsoft Visual C++ 6.0.

USAGE
-----
The following files are used or generated
  inputfile    8 kHz sampled data file 16 bit PCM (binary)
  outputfile   8 kHz sampled data file 16 bit PCM (binary)
  bitstreamfile  binary file containing bitstream
The following parameter is used for the encoder
  dtx_option   = 1 : DTX enabled   0 : DTX disabled

coder  inputfile bitstreamfile dtx_option
decoder bitstreamfile outputfile

