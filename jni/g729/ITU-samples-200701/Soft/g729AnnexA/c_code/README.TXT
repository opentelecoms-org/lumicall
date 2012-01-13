ITU-T G.729 Software Package Release 2 (November 2006)
/* G.729a  Version 1.1    Last modified: September 1996 */

TITLE
-----
Fixed-point description of Recommendation G.729A

Coding of Speech at 8 kbit/s using Conjugate-Structure
Algebraic-Code-Excited Linear-Prediction (CS-ACELP)


SOFTWARE AND INTELLECTUAL PROPERTY
----------------------------------
This software package is provided as part of ITU-T Recommendation G.729A.

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke
   All rights reserved.

The copy of the source C code, version 1.1, is given under Copyright of the
authors, only for the purpose of establishing the specification of a codec.


VERSION
-------
This is version 1.1.

Changes from version 1.0:

-The decoder (decoder.c) now use the same way of detecting the frame erasure
 than the G.729. This have change one of the test vectors "overflow.pst".
-Add some initializations of variable to remove warning from some compilers.
-In typedef.h add the compilator (_TURBOC_).

DESCRIPTION
-----------

  This package include the files needed to build the fixed point version
of the G.729A codec.

 It includes also the PC executable (coder.exe and decoder.exe), a batch file
(test.bat), speech (test.inp) and data files (test.bit and test.syn) to verify
the execution. More files (test vectors) are available to verify the
bit-exactnes of the implementation. They are presently available at the
following ftp site:

ftp.research.bell_labs.com
login: anonymous
cd /dist/g729a

The zip file "TV729APC.ZIP" contains the PC version of the test vectors.



SIMILARITIES AND DIFFERENCES WITH G.729
---------------------------------------

 The serial stream of this version is directly interroperable with the G729.

Common files with G.729:
~~~~~~~~~~~~~~~~~~~~~~~~
basic_op.c
basic_op.h
oper_32b.c
oper_32b.h
pre_proc.c
post_pro.c
bits.c
util.c
filter.c
dspfunc.c
qua_lsp.c
lspdec.c
lspgetq.c
qua_gain.c
dec_gain.c
gainpred.c
de_acelp.c
pred_lt3.c
p_parity.c
dec_lag3.c

Files will small changes from G.729:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

lpc.c
    ->In function Az_lsp() division of the interval by 2 instead of 4.
    Note: The vectors rc[] and old_rc[] of the function Levinson()
          are not used in the G.729A version. These vectors were kept
          to minimize the difference between the G.729 and the G.729A.
          If you implement only the G.729a version, you could remove
          these vectors.

lpcfunc.c
    ->Function Int_lpc() removed.
    ->Function Int_qlpc() is differant.

coder.c and decoder.c
   ->Call to functions with differants names.
   ->The switch HARW and SYNC have been removed. These switch were
     for devellopement and testing purpose only.

Files changed from G.729:
~~~~~~~~~~~~~~~~~~~~~~~~~

ld8a.h      replace  ld8k.h
cod_ld8a.c  replace  cod_ld8k.c
dec_ld8a.c  replace  dec_ld8k.c
acelp_ca    replace  acelp_co.c (New fixed codebook search)
tab_ld8a.h  replace  tab_ld8k.h
tab_ld8a.c  replace  tab_ld8k.c
postfilt.c  replace  pst.c
pitch_a.c   replace  pitch.c (New pitch functions)

New files:
~~~~~~~~~~

Taming.c    The taming functions have been removed from the file
            cod_ld8k.c(G.729) and put together in file taming.c
Cor_func.c  Contains functions:
            Corr_xy2() previously in file cod_ld8k.c (G.729)
            Cor_h_x()  previously in file acelp_ca.c (G.729)
                       Now also used by the pitch functions.


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
PC                         DOS 6.21              Microsoft QuickC
                                                 Borland 3.1
                                                 Watcom 9.6
                                                 Microsoft 8
                                                 MS Visual C++ 1.5



USAGE
-----
The following files are used or generated
  inputfile    8 kHz sampled data file 16 bit PCM (binary)
  outputfile   8 kHz sampled data file 16 bit PCM (binary)
  bitstreamfile  binary file containing bitstream

coder  inputfile bitstreamfile
decoder bitstreamfile outputfile


BITSTREAM FORMAT
----------------
The bitstreamfile contains for each 10 ms speech frame,
82 16-bit words.
The first word is the syncword SYNC_WORD
The second word is the framesize and contains the fixed value 80
The next 80 words contain the following parameters:

01      LPC1-   MA predictor switch
02      LPC1-   1st codebook           7 bit
03      LPC1-
04      LPC1-
05      LPC1-
06      LPC1-
07      LPC1-
08      LPC1-
09      LPC2-   2nd codebook  low         5 bit
10      LPC2-
11      LPC2-
12      LPC2-
13      LPC2-
14      LPC2-   2nd codebook  high         5 bit
15      LPC2-
16      LPC2-
17      LPC2-
18      LPC2-
19      M_1     pitch period                8 bit
20      M_1
21      M_1
22      M_1
23      M_1
24      M_1
25      M_1
26      M_1
27              parity check on 1st period  1 bit
28      CB_1    codebook pulse positions    13 bit
29      CB_1
30      CB_1
31      CB_1
32      CB_1
33      CB_1
34      CB_1
35      CB_1
36      CB_1
37      CB_1
38      CB_1
39      CB_1
40      CB_1
41      S_1     codebook pulse signs       4 bit
42      S_1
43      S_1
44      S_1
45      G_1     pitch and codebook gains  3 bit stage 1
46      G_1
47      G_1
48      G_1     pitch and codebook gains  4 bit stage 2
49      G_1
50      G_1
51      G_1
52      M_2     pitch period (relative)     5 bit
53      M_2
54      M_2
55      M_2
56      M_2
57      CB_2    codebook pulse positions    13 bit
58      CB_2
59      CB_2
60      CB_2
61      CB_2
62      CB_2
63      CB_2
64      CB_2
65      CB_2
66      CB_2
67      CB_2
68      CB_2
69      CB_2
70      S_2     codebook pulse signs       4 bit
71      S_2
72      S_2
73      S_2
74      G_2     pitch and codebook gains  3 bit stage 1
75      G_2
76      G_2
77      G_2     pitch and codebook gains  4 bit stage 2
78      G_2
79      G_2
80      G_2

Bitstream information - all parameters start with msb
The bits are defined as follows:

#define SYNC_WORD (short)0x6b21
#define BIT_0     (short)0x007f /* definition of zero-bit in bit-stream     */
#define BIT_1     (short)0x0081 /* definition of one-bit in bit-stream      */


