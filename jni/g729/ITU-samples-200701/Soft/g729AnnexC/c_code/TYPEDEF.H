/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, Universite of
   Sherbrooke.  All rights reserved.

----------------------------------------------------------------------
*/

/*
 File : TYPEDEF.H
 Used for the floating point version of both
 G.729 main body and G.729A
*/

/*
**
** File:        "typedef.h"
**
*/
/*
   Types definitions
*/
#if defined(__BORLANDC__) || defined (__WATCOMC__) || defined(_MSC_VER) || defined(__ZTC__) || defined(__HIGHC__)
typedef  long  int   INT32;
typedef  short int   INT16;
#elif defined (__sun__) || defined (__sun)
typedef short  INT16;
typedef long  INT32;
#elif defined(__alpha)
typedef short INT16;
typedef int   INT32;
#elif defined(VMS) || defined(__VMS) || defined(VAX)
typedef short  INT16;
typedef long  INT32;
#elif defined (__unix__) || defined (__unix)
typedef short INT16;
typedef int   INT32;
#else
#error  COMPILER NOT TESTED typedef.h needs to be updated, see readme
#endif

#if defined (_single) || defined (single)
typedef  float  FLOAT;
#else
typedef  double  FLOAT;
#endif
#define F   FLOAT
