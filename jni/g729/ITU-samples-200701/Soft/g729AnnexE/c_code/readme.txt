/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729 with ANNEX E   Version 1.3    Last modified: September 1999 */


TITLE
-----
README file for the G.729 annex E software package
(Fixed-point description of Recommendation G.729 with ANNEX E)
(Recommendation G.729 + HIGHER BIT RATE EXTENSION AT 11.8 KB/S)

Coding of Speech using Conjugate-Structure Algebraic-Code-Excited
Linear-Prediction (CS-ACELP) at 8 kbit/s and 11.8 kbit/s using a
backward / forward LPC structure

SOFTWARE AND INTELLECTUAL PROPERTY
----------------------------------
This software package is provided as part of ITU-T Recommendation G.729E.

        Original Copyright (c) 1995, AT&T, France Telecom, NTT,
        Universite de Sherbrooke.
        All rights reserved.
        +
        Copyright (c) 1997, France Telecom, Universite de Sherbrooke.
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

For reporting problems, please contact the TSB helpdesk service at:
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
Edit the file typedef.h to comply to your target platform.

For UNIX systems the following makefiles are provided:

   codere.mak
   decodere.mak

Edit the makefiles codere.mak and decodere.mak to set the proper options
for your system.

The command to compile and link all code on a UNIX system is

         make -f codere.mak
         make -f decodere.mak

For other platforms, the *.mak files can be used to work out the
compilation procedures.

This code has been successfully compiled and run on the following
platforms:

Platform                   Operating System      Compiler
-----------------------------------------------------------------------------
DEC ALPHA 2000             Digital Unix V4.0D    cc DEC C v5.6-082
DEC                        Digital UNIX V4.0     cc DEC C V5.6-071
PC                         DOS 6.22              Borland 3.1
                                                 Watcom 9.6
                                                 Microsoft 8
PC                         WindowNT4             MS Visual C++ 6.0
                                                 Watcom IDE v11.0
                                                 Borland C++ builder v5.0
Linux                      Red Hat 2.2.5-22      gcc (EGCS 1.1.2)


DESCRIPTION
-----------
This package includes the files needed to build the fixed point
version of the G.729 codec with higher bit rate extensions 11.8 kbit/s
as described in ANNEX E.

The distribution also includes:
- PC executables compiled under WinNT4 with MS Visual C++ 6.0 
  for Win32 environment (codere.exe and decodere.exe)
- a batch file (test.bat)
- Testvectors to verify correct execution of G.729 Annex E 
  ANSI-C software Version 1.3 (see readmetv.txt). The binary
  reference files are in PC format.

USAGE
-----
The usage is as follows:
# codere   inputfile bsfile rate_option
# decodere bsfile outputfile

Where:
 inputfile   is the input 8 kHz sampled data file 16 bit PCM (binary)
 outputfile  is the decoded 8 kHz sampled data file 16 bit PCM (binary)
 bsfile      is a binary file containing bitstream

The following parameter is used for the encoder:
 rate_option   specifies the bit rate for the encoder:
               = 1 : higher rate (11.8 kb/s)
               = 0 : G729 (8.0 kb/s)
               = file_rate_name : a binary file with 16-bit words
                 containing either 0 or 1, to select the bitrate on a
                 frame by frame basis (0: 8 kbit/s; 1: 11.8 kbit/s). 
               The default is 0 (8 kb/s)

BITSTREAM FORMAT
----------------

The (binary) bitstream file is organized as 16-bit words. It contains,
for each 10 ms speech frame, 82 16-bit words for the 8 kbit/s rate, or
120 16-bit words for the 11.8 kbit/s rate. The first word in a frame is the
synchronization word SYNC_WORD (see below). The second word contains
the value serial_size (the payload size, in bits), which is 80
(decimal, 0x50 hexadecimal) for 8 kbit/s and 118 (decimal, or 0x76
hexadecimal) for 11.8 kbit/s. For 8 kbit/s operation, the next 80 words
contain the parameters as described in G729. For 11.8 kbit/s operation,
the next 118 words are described in bitstrea.txt.

Bitstream information - all parameters start with msb
The (soft)bits are defined as follows:

#define SYNC_WORD (short)0x6B21
#define BIT_0     (short)0x007F /* definition of zero-bit in bit-stream     */
#define BIT_1     (short)0x0081 /* definition of one-bit in bit-stream      */

A bad frame is indicated by setting all "serial_size" bits to zero.


SOFTWARE ORGANIZATION
----------------------
The files in this distribution can be classified into four groups:
  1) files identical to G729 software files, part of ITU-T G729
     recommendation.
  2) files similar to G729 software files, some minor modifications have
     been introduced to cope with annex E .
     The modications introduced are :
     moda:  list of arguments
     modb:  static variables and arrays formerly local are now passed as
            parameters
     modc: some temporary variables are now passed to the calling routine
           modd: former constants are now passed as variable parameters (LPC
           order, weighting factors of postfiler)
     Note that same type of modifications were used when developping annex A
     of G723.1 and annex B of G729.
  3) files adapted from G729 software files, some source code lines have
     been introduced to existing G729 files to deal with annex E.
  4) files specific to G729 annex E (new files)

For groups 2 and 3, the letter "e" has been added to the names of the
modified files and routines.

group 1 (files identical to G729 software) :
--------------------------------------------
typedef.h, basic_op.h, oper_32b.h, ld8k.h, tab_ld8k.h,
basic_op.c, oper_32b.c, dspfunc.c, gainpred.c,  lpcfunc.c, pred_lt3.c,
pre_proc.c, p_parity.c, qua_gain.c, pwf.c, pitch.c, tab_ld8k.c, util.c,
acelp_co.c, post_pro.c, de_acelp.c, dec_lag3.c

group 2 (files similar to G729 software) :
------------------------------------------
qua_lspe.c (from qua_lsp.c) :
     Qua_lsp -> Qua_lspe (moda , modb, modc)
     Lsp_encw_reset ->Lsp_encw_resete (moda, modb)
     Lsp_qua_cs -> Lsp_qua_cse (moda, modb)
     Relspwed -> Relspwede (moda, modb)
filtere.c   (from filter.c) :
     Syn_filt -> Syn_filte (moda and modd (LPC order) )
     Residu -> Residue (moda and modd (LPC order) )

group 3 (files adapted from G729 software):
------------------------------------------
codere.c (from coder.c),
cod_ld8e.c (from cod_ld8k.c)
     Init_Coder_ld8k -> Init_Coder_ld8e, Coder_ld8k -> Coder_ld8e
decodere.c (from decoder.c),
dec_ld8e.c (from dec_ld8k.c):
     Init_Decod_ld8k -> Init_Decod_ld8e, Decod_ld8k -> Decod_ld8e
bitse.c    (from bits.c):
     bits2prm_ld8k -> bits2prm_ld8e, prm2bits_ld8k-> prm2bits_ld8e
    (bitstream operations for higher rate + moda)
decgaine.c (from dec_gain.c):
     Dec_gain -> Dec_gaine (gains computation in case of erased frame
     for higher rate+ moda and modb)
lspgetqe.c (from lspgetq.c)
     Lsp_get_quant -> Lsp_get_quante (no call of  Lsp_prev_udate, buf is
     copied in freq_cur,routine argument + moda, modb, modc)
lpce.c (from lpc.c)
     Levinson -> Levinsone (test added before division + moda, modb, modd)
     The others routines are identical to G729 lpc.c routines.
pste.c (from pst.c) :
     pst_ltp -> pst_ltpe (computation of gamma_harm * num_gltp with
     gamma_harm variable, + moda and modd(harmonic weighting factor))
     pst_ltpe is the only routine adapted from G729 pst.c file, the others
     routines are either identical or similar to G729 pst.c routines.
     calc_st_filt -> calc_st_filte (moda and modd(LPC order + impulse
             response length))
     calc_rc0_h -> calc_rc0_he  (moda and modd(impulse response length))
     Post -> Poste( moda and modd (postfilter parameters))
   
lspdece.c (from lspdec.c) :
     Lsp_iqua_cs -> Lsp_iqua_cse (due to the suppression of Lsp_prev_update
     in Lsp_get_quante, Lsp_prev_update is now called in both cases
             (frame erased or not) +moda and modb)
     Lsp_iqua_cse is the only routine adapted from G729 lspdec.c file,
     the others routines are either identical or similar to G729 lspdec.c
     routines.
     Lsp_decw_reset -> Lsp_decw_resete (moda and modb)
     D_lsp -> D_lspe (moda and modb)

group 4 (files specific to G729 annex E) :
------------------------------------------
bwfw.c, bwfwfunc.c, pwfe.c, acelp_e.c, deacelpe.c, tab_ld8e.c, track_pi.c
tab_ld8e.h, ld8e.h.

ld8e.h: contains new constants definitions and
        prototypes of routines :
        - similar to G729 routines :
           Qua_lspe, Lsp_encw_resete, Lsp_qua_cse, Relspwede,
           Syn_filte, Residue, Poste, D_lspe, Lsp_decw_resete.
        - adapted from G729 routines :
           Init_Coder_ld8e, Coder_ld8e, Init_Decod_ld8e, Decod_ld8e,
           bits2prm_ld8e, prm2bits_ld8e, Levinsone, Dec_gaine,
           Lsp_get_quante, Lsp_iqua_cse.
        - specific to G729 annex E:
           Lag_window_bwd, Int_bwd, set_lpc_mode, ener_dB,
           tst_bwd_dominant, perc_vare, ACELP_12i40_44bits,
           ACELP_10i40_35bits, Dec_ACELP_12i40_44bits,
           Dec_ACELP_10i40_35bits, track_pit


CODE MODIFICATIONS OF ITU-T G.729 ANNEXE E FROM V1.2 TO V1.3 
------------------------------------------------------------

The Corrigendum (COM-16R 60-E), formally approved by SG 16 on February
8, 2000, describes the modifications that were implemented in ITU-T
G.729 Annex E sofware version 1.2 to bring it up to version 1.3. Only two
C source files (decodere.c and pste.c) were affected. The other files
but codere.c remain unchanged. The file codere.c was changed only to
print a banner on the screen that Version 1.3 is being run.


TEST VECTORS UPDATE OF ITU-T G.729 ANNEXE E FROM V1.2 TO V1.3
-------------------------------------------------------------

From v1.2 to v1.3, ONLY the decoder output test vectors have been
updated. The encoder (input and output) test vectors, as well as the
decoder _input_ test vectors of version 1.3, are bit-exact with those
of version 1.2.

[END]
