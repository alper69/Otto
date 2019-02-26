/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <dir.h>
#include <conio.h>
#include <float.h>
#include "train.h"
#include "trclass.h"

extern unsigned _stklen = 16384U;

// system-wide global variables

int appError = 0;
int retVal = RTN_OK;
Heap HEAP;

char *appErrList[] = {
        "",
        "Error opening training set file.",
        "Not enough memory to allocate 'a' in HMM::HMM()",
        "Not enough memory to allocate 'b' in HMM::HMM()",
        "Not enough memory to allocate 'pi' in HMM::HMM()",
        "Not enough memory to allocate 'a[i]' in HMM::HMM()",
        "Not enough memory to allocate 'b[j]' in HMM::HMM()",
        "\b\b\b\b\b\b\b\b              \n\n\t\t---   End of program   ---\n",
        "Not enough memory to allocate 'nume_a' in TrainLeft_to_Right::TrainLeft_to_Right()",
        "Not enough memory to allocate 'denom_a' in TrainLeft_to_Right::TrainLeft_to_Right()",
        "Not enough memory to allocate 'nume_b' in TrainLeft_to_Right::TrainLeft_to_Right()",
        "Not enough memory to allocate 'denom_b' in TrainLeft_to_Right::TrainLeft_to_Right()",
        "Not enough memory to allocate 'nume_a[i]' in TrainLeft_to_Right::TrainLeft_to_Right()",
        "Not enough memory to allocate 'nume_b[i]' in TrainLeft_to_Right::TrainLeft_to_Right()",
        "Not enough memory to allocate 'Alpha' in HMM::getObsVector()",
        "Not enough memory to allocate 'Beta' in HMM::getObsVector()",
        "Not enough memory to allocate 'Alpha[t]' in HMM::getObsVector()",
        "Not enough memory to allocate 'Beta[t]' in HMM::getObsVector()",
        "Specified directory for models does not exist.",
        "Error opening model file."
};


void exitFunc1 (void)
// ----------------------------------------------------------------------------
// first registered (in main) exit function by 'atexit'
// ----------------------------------------------------------------------------
{ char prompt1[]="Error : %s\n";

  if (appError)
    fprintf(stderr, prompt1, appErrList[appError]);
}                  

void far *Heap::alloc (long size)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ unsigned seg;
  int allocStat;
  long limit = 0xFFFFFL;  // long constant of 1 Mb.

  if (size > limit) {
      retVal = RTN_ERROR;
      return(NULL);
  }
      
  allocStat = allocmem( (unsigned)(size/16) + 1, &seg);

  if (allocStat == -1) {
    // success...
    retVal = RTN_OK;
    return( MK_FP(seg, 0x0) );
  }

  else {
    retVal = RTN_ERROR;
    return(NULL);
  }

}

int Heap::free (void far *block)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int freeStat;

  freeStat = freemem( FP_SEG(block) );

  if (freeStat == 0)
      return(retVal=RTN_OK);
  else
      return(retVal=RTN_ERROR);

}

long Heap::avail ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ unsigned seg;

  return ( (long)16 * (allocmem(65535U, &seg) - 1) );

}

FileName::FileName (char *fName, char *fDir, char *defExt, char *reqMsg, char *pasteMsg)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ char dosCommand[60], prompt1[]="\nDOS Command: ",
                       prompt2[]="\nFile does not exist!\a\n",
                       prompt3[]="use", bSlash[]="\\",
                       Fname[80], ext[MAXEXT];
  int fReturn;


  while (TRUE) {
    printf("\n%s %s", reqMsg, pasteMsg);

    switch(*gets(fName)) {

        case '\x0' : break;

        case '='   : appError = END_PROGRAM;
                     return;

        case '!'   : while (TRUE) {
                         printf(prompt1);

                         if (*gets(dosCommand) == '\x0')
                             break;
                         else
                             system(dosCommand);
                     }
                     break;

        default    : if (fDir != NULL) {
                         strcpy(Fname, fDir);
                         strcat(Fname, bSlash);
                         strcat(Fname, fName);
                     }
                     else
                         strcpy(Fname, fName);

                     fReturn = fnsplit(fName, NULL, NULL, NULL, ext);
                     if ( (fReturn & EXTENSION) == EXTENSION)
                         strcat(Fname, ext);
                     else
                         strcat(Fname, defExt);

                     if (stricmp(Fname, prompt3) == 0)
                         return;
                                
                     
                     if ( fileExist(Fname) )
                         return;
                     else
                         printf(prompt2);

                     break;
    }

  }
}

int FileName::fileExist (char *fn)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  return ( access(fn, 0) == 0  ?  TRUE : FALSE );
}



TrainLeft_to_Right::TrainLeft_to_Right (char *model,
        int nState, int nSymbol,  A_prob iTransition[][nSTATE], B_prob iObs[][nSYMBOL],
        Pi_prob iState[], char *fName) : 
        HMM (model, nState, nSymbol, iTransition, iObs, iState)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i, j;

  fhTrn = NOHANDLE;

  if (appError)  // an error has occured during base class constructor
    return;

  if ((fhTrn=open(fName, O_RDONLY | O_TEXT)) == -1) {
    appError = TRNSET_OPEN;
    return;
  }

  nume_a  = (STATE_TRANS) 0;
  denom_a = (A) 0;
  nume_b  = (OBS_SYMBOL) 0;
  denom_b = (B) 0;


  if ((nume_a = (STATE_TRANS) HEAP.alloc(N * sizeof(A))) == NULL) {
    appError = HMM_ALLOC6;
    return;
  }

  if ((denom_a = (A) HEAP.alloc(N * sizeof(A_prob))) == NULL) {
    appError = HMM_ALLOC7;
    return;
  }

  for (i=0; i<N; i++)   nume_a[i] = (A) 0;


  if ((nume_b = (OBS_SYMBOL) HEAP.alloc(N * sizeof(B))) == NULL) {
    appError = HMM_ALLOC8;
    return;
  }

  if ((denom_b = (B) HEAP.alloc(M * sizeof(B_prob))) == NULL) {
    appError = HMM_ALLOC9;
    return;
  }

  for (j=0; j<N; j++)   nume_b[j] = (B) 0;


  for (i=0; i<N; i++) {

    if ((nume_a[i] = (A) HEAP.alloc(N * sizeof(A_prob))) == NULL) {
      appError = HMM_ALLOC10;
      return;
    }

    if ((nume_b[i] = (B) HEAP.alloc(M * sizeof(B_prob))) == NULL) {
      appError = HMM_ALLOC11;
      return;
    }

  }

}

void TrainLeft_to_Right::multipleTrain (int iterate)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i, j, k;
  A_prob aSum[nSTATE];
  B_prob bSum[nSTATE];

  while (iterate--) {

    Re_estimate();

    printf("\n%E\n", multP_O_);

    // assign re-estimated parameters to model parameters
    for (i=0; i<N; i++) {

        for (j=0; j<N; j++)
            a[i][j] = nume_a[i][j] / denom_a[i];

        for (k=0; k<M; k++)
            b[i][k] = nume_b[i][k] / denom_b[i];
    }

    lseek(fhTrn, 0L, SEEK_SET);
  }

  for (i=0; i<N; i++)
    aSum[i] = bSum[i] = 0.0;
    
  printf("\n\nk   b[1][k]       b[2][k]       b[3][k]       b[4][k]       b[5][k]\n");
  printf("--  ---------     ---------     ---------     ---------     ---------");

  for (k=0; k<M; k++) {

    printf("\n%1d   ", k);

    for (i=0; i<N; i++) {
        printf("%e  ", b[i][k]);
        bSum[i] += b[i][k];
    }
  }

  printf("\n    ---------     ---------     ---------     ---------     ---------");
  printf("\nSum ");

  for (i=0; i<N; i++) {

    printf("%e  ", bSum[i]);

    for (j=0; j<N; j++)
        aSum[i] += a[i][j];
  }

  printf("\n'a' ");
  for (i=0; i<N; i++)
    printf("%e  ", aSum[i]);

  printf("\n");

}


int TrainLeft_to_Right::Re_estimate ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ char CRLF='\xA', ch;
  int t, oSequence[MAX_CCString], i, j, k;

  multP_O_ = 1.0;

  for (i=0; i<N; i++) {

    denom_a[i] = denom_b[i] = 0.0;

    for (j=0; j<N; j++)         nume_a[i][j] = 0.0;
    for (k=0; k<M; k++)         nume_b[i][k] = 0.0;
  }
                  

  t = 0;

  while (read(fhTrn, &ch, sizeof(char)) != 0)

      if (ch == CRLF) {

          printf("* ");

          if (getObsVector(oSequence, t) != RTN_OK)
              return(retVal);

          cForward();
          cBackward();
            
          multP_O_ *= P_O_;

          stepRe_estimate();

          t = 0;
      }

      else
          oSequence[t++] = ch - '0';

            
  return(retVal=RTN_OK);
}


void TrainLeft_to_Right::stepRe_estimate ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i, j, k, t, T;
  Prob nSum, dSum;

  T = ObsLength - 1;

  for (i=0; i<N; i++) {

    // calculate the denominator of eqn. (109)
    dSum = 0.0;

    for (t=0; t<T; t++) 
        dSum += Alpha[t][i] * Beta[t][i];

    denom_a[i] += dSum / P_O_;


    // calculate the numerator of eqn. (109)
    for (j=0; j<N; j++) {

        nSum = 0.0;

        for (t=0; t<T; t++) 
            nSum += Alpha[t][i] * a[i][j] * b[j][ObsVector[t+1]] * Beta[t+1][j];

        nume_a[i][j] += nSum / P_O_;
    }

    // calculate the denominator of eqn. (110), using previous value of 'dSum'
    dSum += Alpha[T][i] * Beta[T][i];
    denom_b[i] += dSum / P_O_;

    // calculate the numerator of eqn. (110)
    // Note: the indices are not the same as in the original eqn. I used 'k'
    //       for 'l', and 'i' for 'j'.
    for (k=0; k<M; k++) {
        
        nSum = 0.0;

        for (t=0; t<=T; t++)
            if (ObsVector[t] == k)
                nSum += Alpha[t][i] * Beta[t][i];

        nume_b[i][k] += nSum / P_O_;
    }

  }

}


TrainLeft_to_Right::~TrainLeft_to_Right ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i;

  if (fhTrn != NOHANDLE)
    close(fhTrn);

  if (nume_a != NULL) { 

    for (i=0; i<N; i++) 
        if (nume_a[i] != NULL) 
            HEAP.free(nume_a[i]);

    HEAP.free(nume_a);
  }

  if (nume_b != NULL) { 

    for (i=0; i<N; i++) 
        if (nume_b[i] != NULL) 
            HEAP.free(nume_b[i]);

    HEAP.free(nume_b);
  }

  if (denom_a != NULL)  HEAP.free(denom_a);
  if (denom_b != NULL)  HEAP.free(denom_b);

}


HMM::HMM (char *model, int nState, int nSymbol, A_prob iTransition[][nSTATE], 
          B_prob iObs[][nSYMBOL], Pi_prob iState[]) : N(nState), M(nSymbol)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i, j, k;

  strcpy(ModelName, model);

  a  = (STATE_TRANS) 0;
  b  = (OBS_SYMBOL)  0;
  pi = (INITIAL_STATE) 0;
  Alpha = Beta = (Prob **) 0;

  if ((a = (STATE_TRANS) HEAP.alloc(N * sizeof(A))) == NULL) {
    appError = HMM_ALLOC1;
    return;
  }

  for (i=0; i<N; i++)   a[i] = (A) 0;

  if ((b = (OBS_SYMBOL) HEAP.alloc(N * sizeof(B))) == NULL) {
    appError = HMM_ALLOC2;
    return;
  }

  for (j=0; j<N; j++)   b[j] = (B) 0;

  if ((pi = (INITIAL_STATE) HEAP.alloc(N * sizeof(Pi_prob))) == NULL) {
    appError = HMM_ALLOC3;
    return;
  }

  for (i=0; i<N; i++) {
    if ((a[i] = (A) HEAP.alloc(N * sizeof(A_prob))) == NULL) {
      appError = HMM_ALLOC4;
      return;
    }

    if ((b[i] = (B) HEAP.alloc(M * sizeof(B_prob))) == NULL) {
      appError = HMM_ALLOC5;
      return;
    }

    // assign initial state probabilities
    pi[i] = iState[i];
  }

  // assign transition probabilities
  for (i=0; i<N; i++)
    for (j=0; j<N; j++)
        a[i][j] = iTransition[i][j];

  // assign obs. symbol probabilities
  for (j=0; j<N; j++)
    for (k=0; k<M; k++) 
        b[j][k] = iObs[j][k];

}


int HMM::getObsVector (int *oSeq, int oLen)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int t;

  if (Alpha != NULL) {

    for (t=0; t<ObsLength; t++)
        if (Alpha[t] != NULL)
            HEAP.free(Alpha[t]);

    HEAP.free(Alpha);
  }

  if (Beta != NULL) {

    for (t=0; t<ObsLength; t++)
        if (Beta[t] != NULL)
            HEAP.free(Beta[t]);

    HEAP.free(Beta);
  }

  // copy the observation sequence
  for (t=0; t<oLen; t++)
    ObsVector[t] = oSeq[t];

  ObsLength = oLen;
  Alpha = Beta = (Prob **) 0;


  // allocate memory for Forward and Backward variables, proportional to
  // the length of observation sequence.

  if ((Alpha = (Prob **) HEAP.alloc(ObsLength * sizeof(Prob *))) == NULL) {
    appError = FVAR_ALLOC1;
    return(retVal=RTN_ERROR);
  }

  for (t=0; t<ObsLength; t++)   Alpha[t] = (Prob *) 0;

  if ((Beta = (Prob **) HEAP.alloc(ObsLength * sizeof(Prob *))) == NULL) {
    appError = BVAR_ALLOC1;
    return(retVal=RTN_ERROR);
  }

  for (t=0; t<ObsLength; t++)   Beta[t] = (Prob *) 0;


  for (t=0; t<ObsLength; t++) {

    if ((Alpha[t] = (Prob *) HEAP.alloc(N * sizeof(Prob))) == NULL) {
      appError = FVAR_ALLOC2;
      return(retVal=RTN_ERROR);
    }

    if ((Beta[t] = (Prob *) HEAP.alloc(N * sizeof(Prob))) == NULL) {
      appError = BVAR_ALLOC2;
      return(retVal=RTN_ERROR);
    }

  }

  return(retVal=RTN_OK);
}


void HMM::cForward ()
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

}

void HMM::cBackward ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i, j, t, T;
  Prob sum;

  T = ObsLength - 1;

  // initialization step, eqn. (24)
  for (i=0; i<N; i++)
    Beta[T][i] = 1.0;

  // induction step, eqn. (25)
  for (t=T-1; t>=0; t--)
    for (i=0; i<N; i++) {

        sum = 0.0;

        for (j=0; j<N; j++)
            sum += a[i][j] * b[j][ObsVector[t+1]] * Beta[t+1][j];

        Beta[t][i] = sum;
    }

}

int HMM::saveModel (char *modelDir)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int fhMdl, i, j, k;
  char mdlName[80], bSlash[]="\\", dExt[]=".MDL", ch;

  printf("\n\nSave model ? [y/n] : ");
  while ((ch=getch()) != 'y'  &&  ch != 'n'  &&  ch != 'Y'  &&  ch != 'N');
  printf("%c\n", ch);  
  if (ch == 'n'  ||  ch == 'N')
    return(retVal=RTN_OK);

  if (access(modelDir , 0) == -1) {
    appError = NO_MDLDIR;
    return(retVal=RTN_ERROR);
  }

  strcpy(mdlName, modelDir);
  strcat(mdlName, bSlash);
  strcat(mdlName, ModelName);
  strcat(mdlName, dExt);

  if ((fhMdl = open(mdlName, O_RDWR | O_CREAT | O_TRUNC | O_BINARY,
                             S_IREAD | S_IWRITE)) == -1) {
    appError = MODF_OPEN;
    return(retVal=RTN_ERROR);
  }

  write(fhMdl, &N, sizeof(int));
  write(fhMdl, &M, sizeof(int));

  for (i=0; i<N; i++)
    write(fhMdl, &pi[i], sizeof(Pi_prob));

  for (i=0; i<N; i++)
    for (j=0; j<N; j++)
        write(fhMdl, &a[i][j], sizeof(A_prob));

  for (j=0; j<N; j++)
    for (k=0; k<M; k++) 
        write(fhMdl, &b[j][k], sizeof(B_prob));

  close(fhMdl);

  return(retVal=RTN_ERROR);
}



HMM::~HMM()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int i;

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


main (int argc, char *argv[])
// ----------------------------------------------------------------------------
//                                 M  A  I  N
// ----------------------------------------------------------------------------
{ int i, j, k;
  int   nState  = nSTATE,
        nSymbol = nSYMBOL;

#define m13_nSTATE      3

  A_prob i_m13_Transition[nSTATE][nSTATE] = {
                { 1.0/2.0, 1.0/2.0,     0.0,     0.0,     0.0 },
                {     0.0, 1.0/2.0, 1.0/2.0,     0.0,     0.0 },
                {     0.0,     0.0,     1.0,     0.0,     0.0 },
                {     0.0,     0.0,     0.0,     0.0,     0.0 },
                {     0.0,     0.0,     0.0,     0.0,     0.0 }
  };

  A_prob iTransition[nSTATE][nSTATE] = {
                { 1.0/3.0, 1.0/3.0, 1.0/3.0,     0.0,     0.0 },
                {     0.0, 1.0/3.0, 1.0/3.0, 1.0/3.0,     0.0 },
                {     0.0,     0.0, 1.0/3.0, 1.0/3.0, 1.0/3.0 },
                {     0.0,     0.0,     0.0, 1.0/2.0, 1.0/2.0 },
                {     0.0,     0.0,     0.0,     0.0,     1.0 }
  };

  B_prob pObs = 1.0 / nSymbol;
  B_prob iObs[nSTATE][nSYMBOL];

  Pi_prob iState[nSTATE] = { 1.0, 0.0, 0.0, 0.0, 0.0 };

  int iCount=5;
  char trainSetDir[80]=".\\trainset", modelDir[80]=".\\model",
       prompt1[]="Enter model name", defExt[]=".TRN", bSlash[]="\\", 
       mdlName[MAXFILE], modelF[80], fName[40];


  atexit(exitFunc1);

  // turn on underflow and precision loss detection  
  _control87(MCW_EM, MCW_EM);

  // initial observation symbol probabilities
  for (j=0; j<nState; j++)
    for (k=0; k<nSymbol; k++)
        iObs[j][k] = pObs;


  for (i=1; i<argc; i++) 

    if (*argv[i] == '-')

        switch( *(argv[i]+1) ) {

            case 't': strcpy(trainSetDir, argv[i]+2);
                      break;

            case 'm': strcpy(modelDir, argv[i]+2);
                      break;
                        
            case 'e': strcpy(&defExt[1], argv[i]+2);
                      break;

            case 'i': iCount = atoi(argv[i]+2);
                      break;
                        
            default : break;    
        }



  while (appError != END_PROGRAM) {

    FileName trnFile(fName, trainSetDir, defExt, prompt1);
    if (appError == END_PROGRAM)
        continue;

    strcpy(modelF, trainSetDir);
    strcat(modelF, bSlash);
    strcat(modelF, fName);

    if ((fnsplit(mdlName, NULL, NULL, mdlName, NULL) & EXTENSION) != EXTENSION)
        strcat(modelF, defExt);


    if (strcmp(fName, "m13") == 0) {
        memcpy(iTransition, i_m13_Transition, nState*nState*sizeof(A_prob));
        nState = m13_nSTATE;
    }

    TrainLeft_to_Right tModel(fName, nState, nSymbol, iTransition, iObs, iState, modelF); _o_;
    tModel.multipleTrain(iCount);
    tModel.saveModel(modelDir);

   }

  return(EXIT_SUCCESS);
}