/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/
/*
 File : OCTET.H
*/

/* Definition for Octet Transmission mode */
/* When Annex B is used for transmission systems that operate on octet boundary, 
   an extra bit (with value zero) will be packed at the end of a SID bit stream. 
   This will change the number of bits in a SID bit stream from 15 bits to
   16 bits (i.e., 2 bytes).
*/

#define OCTET_TX_MODE
#define RATE_SID_OCTET    16     /* number of bits in Octet Transmission mode */



