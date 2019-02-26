/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */
 
#include <stdio.h>
#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <string.h>
#include <dir.h>
#include <graphics.h>
#include <conio.h>
#include "otto.h"
#include "classdef.h"
#include "cs.h"
#include "model.h"


extern int appError;
extern int retVal;
extern Message MESSAGE;
extern Heap HEAP;


HMM::HMM (int fhMdl, int *oSeq, int oLen)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i, j, k;

  a  = (STATE_TRANS) 0;
  b  = (OBS_SYMBOL)  0;
  pi = (INITIAL_STATE) 0;
  Alpha = Beta = (Prob **) 0;

  read(fhMdl, &N, sizeof(int));
  read(fhMdl, &M, sizeof(int));

  if ((a = (STATE_TRANS) HEAP.alloc(N * sizeof(A))) == NULL) {
    appError = HMM_ALLOC1;
    retVal = RTN_ERROR;
    return;
  }

  for (i=0; i<N; i++)   a[i] = (A) 0;

  if ((b = (OBS_SYMBOL) HEAP.alloc(N * sizeof(B))) == NULL) {
    appError = HMM_ALLOC2;
    retVal = RTN_ERROR;
    return;
  }

  for (j=0; j<N; j++)   b[j] = (B) 0;

  if ((pi = (INITIAL_STATE) HEAP.alloc(N * sizeof(Pi_prob))) == NULL) {
    appError = HMM_ALLOC3;
    retVal = RTN_ERROR;
    return;
  }

  for (i=0; i<N; i++) {
    if ((a[i] = (A) HEAP.alloc(N * sizeof(A_prob))) == NULL) {
      appError = HMM_ALLOC4;
      retVal = RTN_ERROR;
      return;
    }

    if ((b[i] = (B) HEAP.alloc(M * sizeof(B_prob))) == NULL) {
      appError = HMM_ALLOC5;
      retVal = RTN_ERROR;
      return;
    }

    // assign initial state probabilities
    read(fhMdl, &pi[i], sizeof(Pi_prob));
  }                     

  // assign transition probabilities
  for (i=0; i<N; i++)
    for (j=0; j<N; j++)
        read(fhMdl, &a[i][j], sizeof(A_prob));

  // assign obs. symbol probabilities
  for (j=0; j<N; j++)
    for (k=0; k<M; k++) 
        read(fhMdl, &b[j][k], sizeof(B_prob));

  getObsVector(oSeq, oLen);
}


int HMM::getObsVector (int *oSeq, int oLen)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int t;

  // copy the observation sequence
  for (t=0; t<oLen; t++)
    ObsVector[t] = oSeq[t];

  ObsLength = oLen;
  Alpha = Beta = (Prob **) 0;


  // allocate memory for Forward variable, proportional to
  // the length of observation sequence.

  if ((Alpha = (Prob **) HEAP.alloc(ObsLength * sizeof(Prob *))) == NULL) {
    appError = FVAR_ALLOC1;
    return(retVal=RTN_ERROR);
  }

  for (t=0; t<ObsLength; t++)   Alpha[t] = (Prob *) 0;


  for (t=0; t<ObsLength; t++) 

    if ((Alpha[t] = (Prob *) HEAP.alloc(N * sizeof(Prob))) == NULL) {
      appError = FVAR_ALLOC2;
      return(retVal=RTN_ERROR);
    }

  return(retVal=RTN_OK);
}


Prob HMM::cForward ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i, j, t, T;
  Prob sum;

  T = ObsLength - 1;

  // initialization step, eqn. (19)
  for (i=0; i<N; i++)
    Alpha[0][i] = pi[i] * b[i][ObsVector[0]];


  // induction step, eqn. (20)
  for (t=0; t<T; t++)
    for (j=0; j<N; j++) {

        sum = 0.0;

        for (i=0; i<N; i++)
            sum += Alpha[t][i] * a[i][j];

        Alpha[t+1][j] = sum * b[j][ObsVector[t+1]];
    }


  // termination step, eqn. (21)
  P_O_ = 0.0;

  for (i=0; i<N; i++)
    P_O_ += Alpha[T][i];

  return(P_O_);
}


HMM::~HMM()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i, t;

  if (Alpha != NULL) {

    for (t=0; t<ObsLength; t++)
        if (Alpha[t] != NULL)
            HEAP.free(Alpha[t]);

    HEAP.free(Alpha);
  }


  if (a != NULL) {

    for (i=0; i<N; i++)
        if (a[i] != NULL)
            HEAP.free(a[i]);

    HEAP.free(a);
  }

  if (b != NULL) {

    for (i=0; i<N; i++) 
        if (b[i] != NULL) 
            HEAP.free(b[i]);

    HEAP.free(b);
  }

  if (pi != NULL)       HEAP.free(pi);
}


int CS::classify (char *mdlPath)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ struct Segment *segTmp;

  if (lSegs->next == NULL) {
    // there is only one segment
    if (lSegs->mfsg != NULL)  // this is a check for loop in the segment
        return( evaluateModel('S', mdlPath, lSegs->mfsg->chainCode) );
  }

  else {
    // there are several segments
    if (lSegs->mfsg != NULL) {
        evaluateModel('B', mdlPath, lSegs->mfsg->chainCode);     _r_;
    }
    segTmp = lSegs->next;
    while (segTmp->next != NULL) {   // as long as the last segment is not reached

        if (segTmp->mfsg != NULL) {
            evaluateModel('M', mdlPath, segTmp->mfsg->chainCode); _r_;
        }
        segTmp = segTmp->next;
    }

    if (segTmp->mfsg != NULL)
        return( evaluateModel('E', mdlPath, segTmp->mfsg->chainCode) );
  }

  return(retVal=RTN_OK);
}


int CS::evaluateModel (const char prefix, char *mPath, char *inpVector)
// ----------------------------------------------------------------------------
// 'prefix' designates the type of character (single, beginning, middle, end)
// to be classified. 'mPath' is the directory of trained models.
// ----------------------------------------------------------------------------
{ int i, t, fhMdl, oSequence[MAX_CCString];
  struct {
          Prob p_O_;
          char model[13];
  } topFive[5];
  char mfName[80], *cPtr, wArg[]="*.*", dummy[]="nil", *ccPtr,
       uLine[]="\n------------  ------------  ------------  ------------  ------------\n",
       fmt1[]="\n\n", fmt2[]="%-14s", fmt3[]="%e  ";
  struct ffblk fBlk;
  Prob P_O_;


  // initialize the top five choices.
  for (i=0; i<5; i++) {
    topFive[i].p_O_ = 0.0;
    strcpy(topFive[i].model, dummy);
  }

  // obtain the integer type observation sequence from the character array.
  t = 0;
  while (inpVector[t]) {
    oSequence[t] = inpVector[t] - '0';
    t++;
  }

  // build the pathname to search for trained models.
  cPtr = stpcpy(mfName, mPath);
  *cPtr++ = '\\';
  *cPtr = prefix;
  strcpy(cPtr+1, wArg);


  if (findfirst(mfName, &fBlk, 0) == -1) {
    appError = NO_MODEL;
    return(retVal=RTN_ERROR);
  }


  do {

    strcpy (cPtr, fBlk.ff_name);

    if ((fhMdl = open(mfName, O_RDONLY | O_BINARY)) == -1) {
        appError = MDLF_OPEN;
        return(retVal=RTN_ERROR);
    }


    HMM hModel(fhMdl, oSequence, t);    _r_;
    P_O_ = hModel.cForward();
    close(fhMdl);

    if (P_O_ > topFive[4].p_O_) {

        for (i=4; i>0; i--)
            if (P_O_ > topFive[i-1].p_O_) {
                topFive[i].p_O_ = topFive[i-1].p_O_;
                strcpy(topFive[i].model, topFive[i-1].model);
            }
            else 
                break;

        topFive[i].p_O_ = P_O_;
        strcpy(topFive[i].model, fBlk.ff_name);
        ccPtr=strchr(topFive[i].model, '.');
        *ccPtr = '\x0';
    }

  } while ( findnext(&fBlk) == 0 );


  printf(fmt1);
  for (i=0; i<5; i++)
    printf(fmt2, topFive[i].model);
  printf(uLine);
  for (i=0; i<5; i++)
    printf(fmt3, topFive[i].p_O_);
 
  (void) getch();

  return(retVal=RTN_OK);
}