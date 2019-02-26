/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#define MAX_CCString    100

struct component {
        long startPixel;
        long contourLength;
        int minX, minY, maxX, maxY;
        struct component *next;
};

struct lpCenter {
        long position;
        int removed;
        struct lpCenter *twin;
        struct lpCenter *next;
};

struct CutMark {
        long position;
        int CFdirection;       // contour following direction (clockwise)
                               // when 'position' is fixed
        struct CutMark *next;
};

struct MFSegment {
        int startX;             // x-cordinate of chain code starting point
        int startY;             // y-cordinate of chain code starting point
        char chainCode[MAX_CCString];   // chain codes of MFSg
};

struct Segment {
        long stCutMark;        // cutMark at the start of reaching branch
        long endCutMark;       // cutMark at the end of connecting branch
        int loopExist;         // TRUE or FALSE
        int CFdirection;       // initial contour following direction (clockwise)
                               // from 'stCutMark'
        long stFeature;        // position of start of feature in segment       
        long endFeature;       // position of end of feature in segment 
        struct MFSegment *mfsg; // pointer to data structure for MFSg
        struct Segment *next;
};


class CS {

        unsigned char huge *Cs;
        int Width, Depth;
        int NeighMask[9];
        struct component *MainCompo,    // CS component.
                         *CompoList;    // list of components.
        struct lpCenter *LoopCenter;    // list of loops.
        struct CutMark *MinList;        // list of cut-marks.
        struct Segment *lSegs;          // list of segments.
        long MinBaseLine;       // position of point where top of baseline interval starts
        long MaxBaseLine;       // position of point where bottom of baseline interval starts

        long Pivot, FB_STARTpoint, BF_STARTpoint, JointIndex, rightmostEP;
        int pvDirection;
        int IgnoreMin, CheckYE, KEFandYE, JointFound, LoopFound, HAfound;

        int floodfill ();
        long enlargeLoop (long);
        int markLoopPixels (long);
        int smooth ();
        int processMask (long, unsigned char [][3], unsigned char);
        int extractCompo (long, struct component *&);
        int processContour (long, int [], unsigned char);
        int processLoopContour (long, int [], unsigned char);
        void displayStep (long, int);
        int findFirstMin (long &, int &);
        int findMin (long &, int &);
        int findMax (long &);
        int findKFP (int, int &, long &);
        int findStP (long &);
        int findFarthestX (long &);
        int separate_YE ();
        int determineSegments ();
        int markMFSgPixels ();
        int removeReachingBranch (struct Segment *, long);
        int extract_all_pixel_MFSg (struct Segment *);
        int extractENDletterMFSg (struct Segment *);
        int END_SINGLE_MFSgPixels (long, int, int, struct Segment *);
        int branchLookAhead (int, int, int, long, int&);
        int branchLookAhead (int, int, int, long, int&, long);
        int extractSINGLEletterMFSg ();
        int extractMIDDLEletterMFSg (struct Segment *);
        int extractBEGINNINGletterMFSg (struct Segment *);
        int chainCode (struct MFSegment *, long, long, int);
        void smoothChainCodes_1 (char *);
        void smoothChainCodes_2 (char *);
        void showMFSgChainCodes (struct MFSegment *, int, int);
        int evaluateModel (const char prefix, char *mPath, char *inpVector);

    public:

        CS (int, int, unsigned char huge *, int, int);
        ~CS ();
        void Framefloodfill ();
        int loopProcess ();
        void Framesmooth ();
        int components ();
        int thin ();
        int stepThin ();
        int setLoops ();
        void keyFeaturePoints ();
        void displayStartPixel (int);
        void Show (int);
        int Min_Max_Partition ();
        void showCutMarks (int);
        void vertProject (int);
        int findMFSg ();
        int chain_code_MFSgs (int);
        int save_MFSgs (char *);
        int classify (char *mdlPath);
        void saveAsBMP ();
};