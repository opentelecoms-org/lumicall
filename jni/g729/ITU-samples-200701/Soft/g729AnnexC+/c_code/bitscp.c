/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/
                                   

/*
File : BITSCP.C
Used for the floating point version of G729B main body, G729D and G729E
*/

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "tab_ld8k.h"
#include "tabld8cp.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"
#include "octet.h"

/* prototypes for local functions */

static void   int2bin(int  value, int  no_of_bits, INT16 *bitstream);
static int    bin2int(int  no_of_bits, INT16 *bitstream);

/*----------------------------------------------------------------------------
* prm2bits_ld8c -converts encoder parameter vector into vector of serial bits
* bits2prm_ld8c - converts serial received bits to  encoder parameter vector
*----------------------------------------------------------------------------
*
* G.729 main Recommendation: 8 kb/s
*
* The transmitted parameters are:
*
*     LPC:     1st codebook           7+1 bit
*              2nd codebook           5+5 bit
*
*     1st subframe:
*          pitch period                 8 bit
*          parity check on 1st period   1 bit
*          codebook index1 (positions) 13 bit
*          codebook index2 (signs)      4 bit
*          pitch and codebook gains   4+3 bit
*
*     2nd subframe:
*          pitch period (relative)      5 bit
*          codebook index1 (positions) 13 bit
*          codebook index2 (signs)      4 bit
*          pitch and codebook gains   4+3 bit
*
*----------------------------------------------------------------------------
*
* G.729 Annex D: 6.4 kb/s
*
* The transmitted parameters are:
*
*     LPC:     1st codebook           7+1 bit
*              2nd codebook           5+5 bit
*
*     1st subframe:
*          pitch period                 8 bit
*          codebook index1 (positions)  9 bit
*          codebook index2 (signs)      2 bit
*          pitch and codebook gains   3+3 bit
*
*     2nd subframe:
*          pitch period (relative)      4 bit
*          codebook index1 (positions)  9 bit
*          codebook index2 (signs)      2 bit
*          pitch and codebook gains   3+3 bit
*
*----------------------------------------------------------------------------
*
* G.729 Annex E: 11.8 kb/s
*
* The transmitted parameters in forward mode are:
*
*     mode  (including parity)        1+1 bit
*
*     LPC:     1st codebook           7+1 bit
*              2nd codebook           5+5 bit
*
*     1st subframe:
*          pitch period                 8 bit
*          parity check on 1st period   1 bit
*          codebook index             7x5 bit
*          pitch and codebook gains   4+3 bit
*
*     2nd subframe:
*          pitch period (relative)      5 bit
*          codebook index             7x5 bit
*          pitch and codebook gains   4+3 bit
*
* The transmitted parameters in backward mode are:
*
*     mode  (including parity)        1+1 bit
*
*     1st subframe:
*          pitch period                 8 bit
*          parity check on 1st period   1 bit
*          codebook index     13+10+7+7+7 bit
*          pitch and codebook gains   4+3 bit
*
*     2nd subframe:
*          pitch period (relative)      5 bit
*          codebook index     13+10+7+7+7 bit
*          pitch and codebook gains   4+3 bit
*----------------------------------------------------------------------------
*/

void prm2bits_ld8c(
    int  prm[],            /* input : encoded parameters  */
    INT16 bits[]           /* output: serial bits         */
)
{
    int i;
    INT16 *ptr_bits;
    
    ptr_bits  = bits;
    *ptr_bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */
        
        
    switch(prm[0]){
                
                
        case 0 : {        /* DTX: no bits transmitted */
            *ptr_bits = RATE_0;
            break;
        }
            
        case 1 : {        /* SID: 15 bits (or 16 bits if OCTET mode) */
                
#ifndef OCTET_TX_MODE
            *ptr_bits++ = RATE_SID;
            for (i = 0; i < PRM_SIZE_SID; i++) {
                int2bin(prm[i+1], bitsno_B[i], ptr_bits);
                ptr_bits += bitsno_B[i];
            }
#else
            *ptr_bits++ = RATE_SID_OCTET;
            for (i = 0; i < PRM_SIZE_SID; i++) {
                int2bin(prm[i+1], bitsno_B[i], ptr_bits);
                ptr_bits += bitsno_B[i];
            }
            *ptr_bits++ = BIT_0;
#endif
                
            break;
         }
                
        case 2 : {
            *ptr_bits++ = RATE_6400;
            for (i = 0; i < PRM_SIZE_D; i++) {
                int2bin(prm[i+1], bitsno_D[i], ptr_bits);
                ptr_bits += bitsno_D[i];
            }
            break;
         }
                
        case 3 : {
            *ptr_bits++ = RATE_8000;
            for (i = 0; i < PRM_SIZE; i++) {
                int2bin(prm[i+1], bitsno[i], ptr_bits);
                ptr_bits += bitsno[i];
            }
            break;
         }
            
        case 4 : {
            *ptr_bits++ = RATE_11800;
            /* case 11.8 kbps */
            if((prm[1]) == 0) { /* mode parameter */
                *ptr_bits++ = BIT_0;     /* "Backward / Forward" Indication bit */
                *ptr_bits++ = BIT_0;       /* Parity bit */
                for (i = 0; i < PRM_SIZE_E_fwd-1; i++) {
                    int2bin(prm[i+2], bitsno_E_fwd[i], ptr_bits);
                    ptr_bits += bitsno_E_fwd[i];
                }
            }
            else {
                *ptr_bits++ = BIT_1;     /* "Backward / Forward" Indication bit */
                *ptr_bits++ = BIT_1;       /* Parity bit */
                for (i = 0; i < PRM_SIZE_E_bwd-1; i++) {
                    int2bin(prm[i+2], bitsno_E_bwd[i], ptr_bits);
                    ptr_bits += bitsno_E_bwd[i];
                }
            }
            break;
         }
                
        default : {
            printf("Unrecognized frame type\n");
            exit(-1);
      }
        
    }
    
    return;
}

/*----------------------------------------------------------------------------
* int2bin convert integer to binary and write the bits bitstream array
*----------------------------------------------------------------------------
*/
static void int2bin(
    int  value,             /* input : decimal value */
    int  no_of_bits,        /* input : number of bits to use */
    INT16 *bitstream        /* output: bitstream  */
                    )
{
    INT16 *pt_bitstream;
    int    i, bit;
        
    pt_bitstream = bitstream + no_of_bits;
    
    for (i = 0; i < no_of_bits; i++) {
        bit = value & 0x0001;      /* get lsb */
        if (bit == 0)
            *--pt_bitstream = BIT_0;
        else
            *--pt_bitstream = BIT_1;
        value >>= 1;
    }
    return;
}

/*----------------------------------------------------------------------------
*  bits2prm_ld8c - converts serial received bits to  encoder parameter vector
*----------------------------------------------------------------------------
*/
void bits2prm_ld8c(
    INT16 bits[],          /* input : serial bits        */
    int  prm[]             /* output: decoded parameters */
)
{
    int  i, mode, parity_mode;
    int nb_bits;
    
    nb_bits = (int)*bits++;        /* Number of bits in this frame       */
    
    switch(nb_bits){
        
        case RATE_0 : {        /* DTX: no bits transmitted */
            prm[1] = 0;
            break;
         }
                
#ifndef OCTET_TX_MODE
        case RATE_SID : {        /* SID: 15 bits (not in OCTET mode) */
            prm[1] = 1;
            for (i = 0; i < PRM_SIZE_SID; i++) {
                prm[i+2] = bin2int(bitsno_B[i], bits);
                bits += bitsno_B[i];
            }
            break;
        }
#else
            /* the last bit of the SID bit stream under octet mode will be discarded */
        case RATE_SID_OCTET : {        /* SID: 16 bits (OCTET mode) */
            prm[1] = 1;
            for (i = 0; i < PRM_SIZE_SID; i++) {
                prm[i+2] = bin2int(bitsno_B[i], bits);
                bits += bitsno_B[i];
            }
            break;
        }
#endif
        case RATE_6400 : {        /* G729 Annex D: 64 bits*/
            prm[1] = 2;
            for (i = 0; i < PRM_SIZE_D; i++) {
                prm[i+2] = bin2int(bitsno_D[i], bits);
                bits += bitsno_D[i];
            }
            break;
        }
                
        case RATE_8000 : {        /* G729: 80 bits*/
            prm[1] = 3;
            for (i = 0; i < PRM_SIZE; i++) {
                prm[i+2] = bin2int(bitsno[i], bits);
                bits += bitsno[i];
            }
            break;
         }
                
        case RATE_11800 : {        /* G729 Annex E: 118 bits*/
            prm[1] = 4;
            
            if( *bits++ == BIT_0) {
                mode = 0;
            }
            else {
                mode = 1;
            }
            if( *bits++ == BIT_0) {
                parity_mode = 0;
            }
            else {
                parity_mode = 1;
            }
            
            if (mode != parity_mode) { /* ==> frame erased */
                for (i=0; i<RATE_11800-2; i++) bits[i] = 0;
            }
            
            prm[2] = mode;
            
            if (mode == 0) {
                for (i = 0; i < PRM_SIZE_E_fwd-1; i++) {
                    prm[i+3] = bin2int(bitsno_E_fwd[i], bits);
                    bits += bitsno_E_fwd[i];
                }
            }
            else {
                for (i = 0; i < PRM_SIZE_E_bwd-1; i++) {
                    prm[i+3] = bin2int(bitsno_E_bwd[i], bits);
                    bits += bitsno_E_bwd[i];
                }
            }
            break;
        }
                
        default : {
            printf("Unrecognized frame type: uncorrect value of serial_size\n");
            exit(-1);
        }
            
    }
        
    return;
}
                                   
                                   
/*----------------------------------------------------------------------------
* bin2int - read specified bits from bit array  and convert to integer value
*----------------------------------------------------------------------------
*/
static int  bin2int(            /* output: decimal value of bit pattern */
    int  no_of_bits,        /* input : number of bits to read       */
    INT16 *bitstream        /* input : array containing bits        */
                                   )
{
    int    value, i;
    int  bit;
    
    value = 0;
    for (i = 0; i < no_of_bits; i++) {
        value <<= 1;
        bit = *bitstream++;
        if (bit == BIT_1)  value += 1;
    }
    return(value);
}
