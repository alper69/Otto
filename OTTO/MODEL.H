/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#include "..\hmm\tdef.h"

#define MAX_CCString    100

class HMM {

    protected:

        int N;          // number of states
        int M;          // number of obs. symbols

        INITIAL_STATE   pi;     // initial state prob. vector
        STATE_TRANS     a;      // state transition prob. matrix
        OBS_SYMBOL      b;      // obs. symbol prob. matrix

        int     ObsVector[MAX_CCString];
        int     ObsLength;
        Prob    **Alpha;
        Prob    **Beta;
        Prob    P_O_;

        int getObsVector (int *, int);

    public:

        HMM(int, int*, int);
        ~HMM();
        Prob cForward ();
};

