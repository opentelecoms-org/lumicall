 ITU-T G.729 Software Package Release 2 (November 2006) 
1 - TITLE
---------
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

2 - VERSION
-----------
This is version 1.01
ITU-T G.729/Annex C was approved on 09/1998


3 - COPYRIGHT AND INTELLECTUAL PROPERTY
---------------------------------------

This software package is provided as part of ITU-T Recommendation G.729 Annex C.

   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.

The copy of the source C code, version 1.01, is given under Copyright of the
authors, only for the purpose of establishing the specification of a codec.

All rights are reserved. All other use of the material is prohibited.

4 - SUPPORT
------------
For distribution of update software, please contact:

    Sales Department
    ITU
    Place des Nations
    CH-1211 Geneve 20
    SWITZERLAND
    email: sales@itu.int

For reporting problems, please contact TSB helpdesk service at:

    TSB Helpdesk service
    ITU
    Place des Nations
    CH-1211 Geneve 20
    SWITZERLAND 
    fax: +41 22 730 5853
    email: tsbedh@itu.int

5 - TECHNICAL DETAILS
---------------------

5.1 - COMPILATION
-----------------
Edit the file typedef.h to comply to your target platform

#-----------------------------------------------------------------------
# Makefile for compiler the encoder/decoder of the 
# G.729 Annex C Floating point implementation of 
# the 8 kbit/s G.729 (main body) algorithm
# This makefile was prepared for Unix systems
#-----------------------------------------------------------------------

   coder.mak
   decod.mak

#-----------------------------------------------------------------------
# Makefile for compiler the encoder of the 
# G.729 Annex C Floating point implementation of 
# the 8 kbit/s G.729 Annex A algorithm
# This makefile was prepared for Unix systems
#-----------------------------------------------------------------------

   codera.mak
   decoda.mak


Edit the makefiles *.mak to set the proper options
for your system.
The command to compile and link all code on a UNIX system is

         make -f coder.mak
         make -f decod.mak

         make -f codera.mak
         make -f decoda.mak


For other platforms, the *.mak files can be used to work out the
compilation procedures.


5.2 - DESCRIPTION
-----------------
  This package includes the files needed to build the Reference ans annex A
  floating-point implementation for G.729 CS-ACELP 8 kbit/s speech coding


5.3 - USAGE
-----------

coder  speech_file  bitstream_file
decoder bitstream_file  outputspeech_file

Format for speech_file:
   Speech is read form a binary file of 16 bits data.
Format for bitstream_file: 
   One word (2-bytes) to indicate erasure.
   One word (2 bytes) to indicate bit rate
   80 words (2-bytes) containing 80 bits.
Format for outputspeech_file:
   Synthesis is written to a binary file of 16 bits data.

codera and decodera works the same as coder and decoder.

5.4 - BITSTREAM FORMAT
----------------------

For G729 main floating point implementation :
************************************************

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


For G729 Annex A floating point implementation :
************************************************

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


