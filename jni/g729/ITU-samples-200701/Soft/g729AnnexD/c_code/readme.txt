/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX D version 1.3    Last modified: 17 Oct 2000 */


TITLE
-----
Explanation letter for the G.729 annex D software package
( Fixed-point description of Recommendation G.729 with ANNEX D)
(Recommendation G.729 + LOWER BIT RATE EXTENSION AT 6.4 KB/S)

Coding of Speech using Conjugate-Structure
Algebraic-Code-Excited Linear-Prediction (CS-ACELP)
at 8 kbit/s and 6.4 kb/s


SOFTWARE AND INTELLECTUAL PROPERTY
----------------------------------
This software package is provided as part of ITU-T Recommendation G.729D.

        Original Copyright (c) 1995, AT&T, France Telecom, NTT,
        Universite de Sherbrooke.
        All rights reserved.
        +
        Copyright (c) 1998, Ericsson, NTT.
        All rights reserved.

The copy of the source C code, version 1.3, is given under Copyright
of the authors, only for the purpose of establishing the specification
of a codec.

For distribution of update software, please contact:
 Sales Department
 ITU
 Place des Nations
 CH-1211 Geneve 20
 SWITZERLAND
 Email: sales@itu.int
 WWW: www.itu.int

For reporting problems, please contact TSB helpdesk service at:
 TSB Helpdesk service
 ITU
 Place des Nations
 CH-1211 Geneve 20
 SWITZERLAND
 Fax: +41 22 730 5853
 Email: tsbedh@itu.int


VERSION
-------
This is version 1.3.

COMPILATION
-----------
Edit the file typedef.h to comply to your target platform

For UNIX systems the following makefiles are provided:

   coderd.mak
   decoderd.mak

Edit the makefiles coderd.mak and decoderd.mak to set the proper
options for your system.

The command to compile and link all code on a UNIX system is

         make -f coderd.mak
         make -f decoderd.mak

For other platforms, the *.mak files can be used to work out the
compilation procedures.

This code has been successfully compiled and run on the following
platforms:

Platform                   Operating System      Compiler
-----------------------------------------------------------------------------
SUN                        Solaris               gcc
DEC ALPHA 2000             Digital Unix V4.0D    cc DEC C v5.6-082
PC                         DOS 6.22              Borland 3.1
                                                 Microsoft 8
PC                         WindowNT4             MS Visual C++ 6.0
                                                 Watcom IDE v11.0
                                                 Borland C++ builder v5.0


DESCRIPTION
-----------
This package includes the files needed to build the fixed point
version of the G.729 codec with the lower bit rate extensions 6.4
kbit/s as described in G.729 Annex D.

The distribution also includes the following test vectors to verify
the correct execution of the G.729 Annex D ANSI-C software Version
1.3:
- input files (testfile.raw), 
- bitstream files (testmid.1, testmid.2, testmid.3)
- decoder output files (testout.1, testout.2, testout.3)


USAGE
-----
The usage is as follows:
# coderd   rate_option inputfile bitstreamfile
# decoderd rate_option bitstreamfile outputfile

Where:
  inputfile      is the 8 kHz sampled data file 16 bit PCM (binary)
  outputfile     is the 8 kHz sampled data file 16 bit PCM (binary)
  bitstreamfile  is a binary file containing bitstream

The following parameter is used for the encoder
  rate_option   = 1 : lower rate (6.4 kb/s)
                = 2 : G729 (8.0 kb/s)
                = 3 : toggle mode every frame

BITSTREAM FORMAT
----------------

The (binary) bitstream file is organized as 16-bit words. It contains,
for each 10 ms speech frame, 82 16-bit words for the 8 kbit/s rate, or
66 16-bit words for the 6.4 kbit/s rate. The first word in a frame is
the synchronization word SYNC_WORD (see below). The second word
contains the value serial_size, which is 80 (decimal, 0x50
hexadecimal) for 8 kbit/s and 64 (decimal, or 0x40 hexadecimal) for
6.4 kbit/s. For 8 kbit/s operation, the next 80 words contain the
parameters as described in G.729. For 6.4 kbit/s operation, the next
64 words are described in bitstrea.txt.

Bitstream information - all parameters start with msb. The (soft)bits
are defined as follows:

#define SYNC_WORD (short)0x6B21
#define BIT_0     (short)0x007F /* definition of zero-bit in bit-stream     */
#define BIT_1     (short)0x0081 /* definition of one-bit in bit-stream      */

A bad frame is indicated by setting all "serial_size" bits to zero.


SOFTWARE ORGANIZATION
----------------------
the files can be classified into four groups:
  1) files identical to G729 software files, part of ITU-T G729
     recommendation.
  2) files similar to G729 software files, some minor modifications have
     been introduced to cope with annex D.
     The modications introduced are :
     moda: list of arguments
     modb: static variables and arrays formerly local are now passed as
           parameters
     modc: some temporary variables are now passed to the calling routine
     modd: former constants are now passed as variable parameters (LPC
          order, weighting factors of postfiler)
     Note that same type of modifications were used when developping annex A
     of G723.1 and annex B of G729.
  3) files adapted from G729 software files, some source code lines have
     been introduced to existing G729 files to deal with annex D.
  4) files specific to G729 annex D (new files)

For groups 2 and 3, the letter "d" has been added to the names of the
modified files and routines.

----------------------------------------------
group 1 (files identical to G729 software) :
----------------------------------------------
basic_op.h, ld8k.h, oper_32b.h, tab_ld8k.h, typedef.h,
basic_op.c, de_acelp.c, dspfunc.c, gainpred.c,
lpc.c, lpcfunc.c, lspdec.c, lspgetq.c, oper_32b.c, p_parity.c,
post_pro.c, pre_proc.c, pred_lt3.c, pst.c, pwf.c,
qua_lsp.c, tab_ld8k.c, util.c

----------------------------------------------
group 2 (files similar to G729 software) :
----------------------------------------------
None.


----------------------------------------------
group 3 (files adapted from G729 software) :
----------------------------------------------
acelpcod.c (from acelp_co.c),
    #ld8kd.h
    #tabld8kd.h
    ACELP_Codebook()  -> ACELP_CodebookD()
    Cor_h()           -> Cor_h_D()

bitsd.c (from bits.c),
    #ld8kd.h
    #tabld8kd.h
    bits2prm_ld8k() -> bits2prm_ld8kD()
    prm2bits_ld8k() -> prm2bits_ld8kD()

codld8kd.c (from cod_ld8k.c),
    #ld8kd.h
    #tabld8kd.h
    ACELP_Codebook() -> ACELP_CodebookD()
    Enc_lag3()       -> Enc_lag3D()
    Coder_ld8k()     -> Coder_ld8kD()
    Pitch_fr3()      -> Pitch_fr3D()

coderd.c (from coder.c),
    #ld8kd.h
    #tabld8kd.h
    Coder_ld8k()    -> Coder_ld8kD()
    prm2bits_ld8k() -> prm2bits_ld8kD()

declag3d.c (from dec_lag3.c),
    #ld8kd.h
    #tabld8kd.h
    Dec_lag3() -> Dec_lag3D()

decld8kd.c (from dec_ld8k.c),
    #ld8kd.h
    #tabld8kd.h
    Dec_lag3()   -> Dec_lag3D()
    Decod_ld8k() -> Decod_ld8kD()

decoderd.c (from decoder.c),
    #ld8kd.h
    #tabld8kd.h
    Decod_ld8k()    -> Decod_ld8kD()
    bits2prm_ld8k() -> bits2prm_ld8kD()

filterd.c (from filter.c),
    #ld8kd.h
    #tabld8kd.h

pitchd.c (from pitch.c),
    #ld8kd.h
    #tabld8kd.h
    Enc_lag3()  -> Enc_lag3D()
    Pitch_fr3() -> Pitch_fr3D()

----------------------------------------------
group 4 (files specific to G729 annex D) :
----------------------------------------------
ld8kd.h, tabld8kd.h, deacelpd.c, dec_g6k.c, qua_g6k.c, tabld8kd.c,
qua_g8k.c, dec_g8k.c


ld8kd.h = ld8k.h(6.4k) - ld8k.h(G729)
tabld8kd.h = tab_ld8k.h(6.4k) - tab_ld8k.h(G729)

deacelpd.c = de_acelp.c(6.4k) - de_acelp.c(G729)
   #ld8kd.h 

tabld8kd.c = tab_ld8k.c(6.4k) - tab_ld8k.c(G729)
   #ld8kd.h 
   #tabld8kd.h 

dec_g6k.c = dec_g6k.c(6.4k)
qua_g6k.c = qua_g6k.c(6.4k)

qua_g8k.c (from qua_gain.c):
  Qua_gain() -> Qua_gain8k()

dec_g8k.c (from dec_gain.c):
  Dec_gain() -> Dec_gain8k()


CODE MODIFICATIONS OF ITU-T G.729 ANNEX D FROM V1.2 TO V1.3 
------------------------------------------------------------
The code modifications from v1.2 to v1.3 follows the corrigendum
COM-16R 60-E. The files affected were: 

 - acelpcod.c   - decld8kd.c   
 - coderd.c     - decoderd.c   
 - coderd.mak   - decoderd.mak 
 - codld8kd.c   - ld8kd.h      
 - deacelpd.c   - qua_g6k.c    
 - dec_g6k.c    - qua_gain.c   
 - dec_gain.c

The files qua_gain.c and dec_gain.c were renamed to qua_g8k.c and
dec_g8k.c respectively in order to avoid confusion with the original
G.729 software files.  

The files coderd.c and decoderd.c were only edited so that the right
version number is displayed during coding/decoding. The makefiles were
changed due to the new file names.


TEST VECTORS UPDATE OF ITU-T G.729 ANNEX D FROM V1.2 TO V1.3 
------------------------------------------------------------
The test vectors for mode 1 and 3 (toggle mode) have been updated for
version 1.3.

[END]