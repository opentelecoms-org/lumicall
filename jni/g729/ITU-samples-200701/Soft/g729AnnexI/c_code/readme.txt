ITU-T G.729 Software Package Release 2 (November 2006)
1 - TITLE
---------
/*
   ITU-T G.729 Annex I  - Reference C code for fixed point
   implementation of G.729 CS-ACELP at 6.4 kbit/s 8 kbit/s
   and 11.8 kb/s with DTX functionality
   (integration of Annexes B, D and E)
                           Version 1.2 of October 2006
*/

2 - VERSION
-----------
This is version 1.2
ITU-T G.729/Annex I was approved on 02/2000

-----------------------------------------------------
Differences between v1.1 and v1.2 :

file codld8cp.c :
*****************
According to COM16-D219, the following changes were made in v1.2

version 1.1 lines 83-84 were deleted :
-----------------------------------------
/* to tame the coder */
static Word 32 L_exc_err[4];
-----------------------------------------

version 1.1 line 175 :
---------------------------------------------------------
for(i=0; i<4; i++) L_exc_err[i]=0x00004000L;	/* Q14 */
---------------------------------------------------------
is replaced in v1.2 line 172 by :
---------------------------------------------------------
/* to tame the coder */
Init_exc_err();
---------------------------------------------------------

v1.2 line 202 :
---------------------------------------------------------
    if(dtx_enable == 1) {
        pastVad = 1;
        ppastVad = 1;
        seed = INIT_SEED;
        vad_init();
        Init_lsfq_noise();
	Init_Cod_cng();
    }
---------------------------------------------------------

file qsidlsf.c :
****************
According to COM16-D244, the following changes were made in v1.2

v1.1 lines 194-204 and 263-73:
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

are replaced in v1.2 lines 194-206 and 265-277 by :
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
---------------------------------------------------------

file decld8cp.c :
*****************

line 220 :
        if(ftyp == 1) ftyp = 0;

is replaced by :
        if(ftyp == 1) ftyp = 0;
        if(ftyp > 2) {
          if(ftyp == 3) parm[4] = 1;
          else {
            if(prev_lp_mode == 0) parm[5] = 1;
            else parm[3] = 1;
          }
        }



3 - COPYRIGHT AND INTELLECTUAL PROPERTY
---------------------------------------

This software package is provided as part of ITU-T Recommendation G.729 Annex I.

   ITU-T G.729 Annex  I fixed point ANSI C source code
   Copyright (C) 1999, AT&T, France Telecom, NTT, University of
   Sherbrooke, Conexant, Ericsson. All rights reserved.

   Original Copyright (c) 1995, AT&T, France Telecom, NTT,
   Universite de Sherbrooke.
   All rights reserved.
	 +
   Copyright (c) 1996, AT&T, France Telecom, NTT, Rockwell International,
   Universite de Sherbrooke.
   All rights reserved.
	+
   Copyright (c) 1997, Ericsson, NTT.
   All rights reserved.
	+
   Copyright (c) 1997, France Telecom, Universite de Sherbrooke.
   All rights reserved.

The copy of the source C code, version 1.2, is given under Copyright of the
authors, only for the purpose of establishing the specification of a codec.

All rights are reserved. Any other use of the material is prohibited.

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

For UNIX systems the following makefiles are provided

   coderi.mak
   decoderi.mak

Edit the makefiles coderi.mak and decoderi.mak to set the proper options
for your system.
The command to compile and link all code on a UNIX system is

         make -f coderi.mak
         make -f decoderi.mak

For other platforms, the *.mak files can be used to work out the
compilation procedures.

This code has been successfully compiled and run on the following
platforms:

Platform                   Operating System      Compiler
-----------------------------------------------------------------------------
DEC ALPHA                  OSF/1                 DEC OSF/1 cc
PC                         DOS 6.21              Borland 3.1
						 Watcom 9.6
						 Microsoft 8
PC                         Window95              MS Visual C++ 5.0
							CYGWIN32 			gcc egcs-2.91.57
PC                         WindowNT4             MS Visual C++ 6.0
HP-UX                      B.10.01               gcc
SGI-IRIX                   IRIX 5.3              SGI Rel 5.3 cc

Version 1.2 of this code has only been tested on a PC platform with Windows XP and compiled with Microsoft Visual C++ 6.0.

5.2 - DESCRIPTION
-----------------
This package includes the files needed to build the fixed point
version of G.729 annex I codec with lower and higher bit rate
extensions at 6.4 kbit/s and 11.8 kbit/s and with dtx functionality

It also includes:
- the PC executable (coderi.exe and decoderi.exe):
- a batch file (test.bat) and testvectors to validate the correct
  execution of the G.729 annex I (ANSI-C software version 1.1). The
  binary reference files are in PC format (for more details see
  readmetv.txt).

5.3 - USAGE
-----------
coderi  inputfile bitstreamfile dtx_option rate_option
decoderi bitstreamfile outputfile

The following files are used or generated
  inputfile    8 kHz sampled data file 16 bit PCM (binary)
  outputfile   8 kHz sampled data file 16 bit PCM (binary)
  bitstreamfile  binary file containing bitstream

The following parameters are used for the encoder
	dtx_option              = 0 dtx disabled (default)
			        = 1 dtx enabled
        rate_option             = 0 : lower rate (6.4 kb/s)
				= 1 : G729 (8.0 kb/s)
				= 2 : higher rate (11.8 kb/s)
				= file_rate_name : a binary file of 16 bit 
                                  word containing either 0, 1 or 2 to
                                  select the rate on a frame by frame
                                  basis the default is 1 (8 kb/s)


5.4 - BITSTREAM FORMAT
----------------------
The bitstream file contains for each 10 ms speech frame, For rate 8
kbit/s, 82 16-bit words or for rate 6.4 kbit/s, 66 16-bit words, or
for rate 11.8 kbit/s, 120 16-bit words, or for SID frame, 17(or 18)
16-bit words, or for not transmitted frame, 2 16-bit words.

The first word is the synchronization word SYNC_WORD.  The second word
contains the value serial_size which is 80 for rate 8 kbit/s, 64 for
rate 6.4 kbit/s, 118 for rate 11.8 kbit/s, 15( or 16) for SID frame, 0
for Non-transmitted frame.

For the rate 8 kbit/s, the next 80 words contain the parameters as
described in G729 recommendation text. For the other rates, the next
"serial_size" words are described in bitstrea.txt.

Bitstream information - all parameters start with MSB. The bits are
defined as follows:

#define SYNC_WORD (short)0x6b21
#define BIT_0     (short)0x007f /* definition of zero-bit in bit-stream     */
#define BIT_1     (short)0x0081 /* definition of one-bit in bit-stream      */

A bad frame is indicated by setting all "serial_size" bits to zero.
Except, for not transmitted frame, where the synchronization word
SYNC_WORD is set to (short)0x6B20.

--[END]