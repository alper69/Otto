/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#include "tdef.h"

#define FALSE   0
#define TRUE    1

#define NOHANDLE        -2

// general function return values
#define RTN_ERROR       0
#define RTN_OK          1


#define _o_     if (appError)    return(appError)
#define _f_     if (retVal == RTN_ERROR)    return(appError)


// index constant mnemonics for application error messages
#define TRNSET_OPEN     1
#define HMM_ALLOC1      2
#define HMM_ALLOC2      3
#define HMM_ALLOC3      4
#define HMM_ALLOC4      5
#define HMM_ALLOC5      6
#define END_PROGRAM     7
#define HMM_ALLOC6      8
#define HMM_ALLOC7      9
#define HMM_ALLOC8      10
#define HMM_ALLOC9      11
#define HMM_ALLOC10     12
#define HMM_ALLOC11     13
#define FVAR_ALLOC1     14
#define BVAR_ALLOC1     15
#define FVAR_ALLOC2     16
#define BVAR_ALLOC2     17
#define NO_MDLDIR       18
#define MODF_OPEN       19


#define nSTATE  5
#define nSYMBOL 8

#define MAX_CCString    100     // this value has to be same as in CS.H