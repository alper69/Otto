/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

class HMM {

    protected:

        char ModelName[13];
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

        void cForward ();
        void cBackward ();

    public:

        HMM(char *, int, int, A_prob iTransition[][nSTATE], B_prob iObs[][nSYMBOL], Pi_prob iState[]);
        ~HMM();
        int getObsVector (int *, int);
        int saveModel (char *);
};

class TrainLeft_to_Right : public HMM {

        STATE_TRANS nume_a;     // eqn. (109)
        A           denom_a;
        OBS_SYMBOL  nume_b;     // eqn. (110)
        B           denom_b;
        Prob    multP_O_;       // eqn. (108)
        int     fhTrn;          // file handle of obs. sequences for training

        void stepRe_estimate ();
        int Re_estimate ();

    public:

        TrainLeft_to_Right(char *, int, int, A_prob iTransition[][nSTATE], B_prob iObs[][nSYMBOL], Pi_prob iState[], char *);
        ~TrainLeft_to_Right();
        void multipleTrain (int);

};

class FileName {

        int fileExist (char *);

    public:

        FileName (char *, char * = NULL, char * = NULL, char * = "Enter filename", 
                          char * = "( [!]Dos command, [=]Exit ) : ");
};

class Heap {

    public :

        void far *alloc (long);
        int free (void far *);
        long avail ();
};
