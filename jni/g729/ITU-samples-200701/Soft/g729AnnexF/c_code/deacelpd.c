/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* version 1.3 modified september 1999 */
/* G.729 with ANNEX D   Version 1.1    Last modified: March 1998 */



#include "typedef.h"

#include "basic_op.h"

#include "ld8k.h"

#include "ld8kd.h"

#include "tabld8kd.h"



/*-----------------------------------------------------------*

 *  Function  decod_ACELP64()                                *

 *  ~~~~~~~~~~~~~~~~~~~~~~~                                  *

 *   Algebraic codebook decoder.                             *

 *----------------------------------------------------------*/

 

void Decod_ACELP64(

 Word16 sign,              /* input : signs of 2 pulses     */

 Word16 index,             /* input : positions of 2 pulses */

 Word16 cod[]          /* Q13 output: innovative codevector */

)

{

   Word16 pos[4];

   Word16 i, j;

 

   /* decode the positions of 4 pulses */

 

   i = index & (Word16) 15;

   pos[0] = trackTable0[ grayDecode[i] ];

   

   index = shr(index, 4);

   

   i = index & (Word16) 31;

   pos[1] = trackTable1[ grayDecode[i] ];

 

   /* find the algebraic codeword */

 

   for (i = 0; i < L_SUBFR; i++) cod[i] = 0;

 

   /* decode the signs of 2 pulses */



   for (j=0; j<2; j++)

   {



     i = sign & (Word16) 1;

     

     sign = shr(sign, 1);

     

     if (i != 0) {

       cod[pos[j]] += 8191; /* 1 in Q13 */

     }

     else {

       cod[pos[j]] -= 8192; /* -1 in Q13 */

     }

   }

 

   return;

}

