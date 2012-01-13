ITU-T G.729 Software Package Release 2 (November 2006)
/* Version 3.3    Last modified: December 26, 1995 */

TITLE
-----
Explanation letter for the G.729 software package

Coding of Speech at 8 kbit/s using Conjugate-Structure
Algebraic-Code-Excited Linear-Prediction (CS-ACELP)

SOFTWARE AND INTELLECTUAL PROPERTY
----------------------------------
This software package is provided as part of ITU-T Recommendation G.729.

 Copyright (c) 1995, AT&T, France Telecom, NTT, Universite de Sherbrooke.
 All rights reserved.

VERSION
-------
This version is a 16 bit fixed-point implementation of the floating
point version that was tested in January 1995. Version 2.1 of the
fixed point software was tested in October 1995. This is version 3.3.

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
Silicon Graphics R4400     Unix 5.2              cc
Silicon Graphics R4400     Unix 5.2              gcc 2.6.3
SUN Sparc2                 Unix 5.1              gcc 2.6.3
DEC ALPHA                  OSF/1                 DEC OSF/1 cc
HP                         Unix                  c89
PC                         DOS 6.21              Microsoft QuickC
                                                 Borland 3.1
                                                 Watcom 9.6
                                                 Zortec 3.1
                                                 Microsoft 8
                                                 MS Visual C++ 1.5

Additional flags:

All ITU tests were done using 13 bit input and output. This mode can
be set by defining the HARDW flag.
By setting the flag SYNC the input and output files are
aligned. Otherwise there will be a 40 sample delay between input and
output. All test files were generated without these flags defined.


USAGE
-----
The following files are used or generated
  inputfile    8 kHz sampled data file 16 bit PCM  (binary)
  outputfile   8 kHz sampled data file 16 bit PCM (binary)
  bitstreamfile  binary file containing bitstream

coder  inputfile bitstreamfile
decoder bitstreamfile outputfile

VERIFICATION
------------
To verify correct compilation and execution on your target machine
the following test files can be obtained (temporarily) from
ftp.research.att.com

speech.in    - speech signal
algthm.in    - artificial signal to test certain parts of the code
fixed.in     - artificial signal to test codebook search
lsp.in       - artificial signal to test lsp quantization
pitch.in     - artificial signal to test long-term predictor
tame.in      - sinewave to test the taming procedure

The corresponding bitstream files and output files have the same name
but extentions *.bit and *.pst, respectively.

In addition, the following bitstream and output files are provided:
parity.bit   - test file for parity errors
erasure.bit  - test file for frame erasures
overflow.bit - test file to excercise the overflow detection

WARNING: These testvectors are provided to verify correct execution of
the software on the target platform. They cannot be used to verify
compliance to the standard.

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

A bad frame is indicated by setting all 80 bits to zero.

