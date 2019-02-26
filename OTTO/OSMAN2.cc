/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphics.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>

#include "otto.h"
#include "classdef.h"
#include "cs.h"

extern void beep (void);

// system-wide global variables

int appError = 0;
int retVal = RTN_OK;
Message MESSAGE;
Heap HEAP;

char *appErrList[] = {
        "",
        "Mouse driver not installed.",
        "**********************************************************************",
        "Undefined screen mode request in Screen::mode.",
        "Not enough memory to allocate Screen object.",
        "Error opening image file.",
        "File is not in PCX format.",
        "Not a binary image.",
        "Image too large.",
        "Error creating temporary image file.",
        "Error reading PCX file.",
        "Error writing temporary image file.",
        "PCX file corrupted (bytes_per_line error).",
        "Menu cannot be displayed in this screen mode.",
        "Image cannot be displayed in this screen mode.",
        "Not enough memory to allocate menu CmdXBorders object.",
        "Not enough memory to allocate menu CmdBoxImgBuf object.",
        "\b\b\b\b\b\b\b\b              \n\n\t\t--- NORMAL TERMINATION ---\n",
        "A menu should be created before image is displayed.",
        "Not enough memory to allocate CutOut Frame.",
        "Next contour was not determined in CS::findLoopCenter().",
        "Next contour was not determined in CS::markLoopPixels().",
        "Not enough memory to allocate 'struct lpCenter' in CS::enlargeLoop().",
        "Unknown pixel encountered in CS::enlargeLoop().",
        "Next contour was not determined in CS::extractCompo().",
        "Not enough memory to allocate 'struct component' in CS::extractCompo().",
        "Next contour was not determined in CS::thin().",
        "Next loop pixel was not determined in CS::setLoops().",
        "Twin of loop was not determined in CS::setLoops().",
        "No components to process...",
        "Heap is corrupted !",
        "Error allocating from free heap in Image::decompressPCX().",
        "Not enough memory to allocate 'struct CutMark' in CS::Min_Max_Partition().",
        "Unexpected return value from CS::findMin() in CS::Min_Max_Partition().",
        "Unexpected return value from CS::findMax() in CS::Min_Max_Partition().",
        "Valid starting point not found in CS::firstMin_MaxStep().",
        "Unexpected return value from CS::findMin() in CS::firstMin_MaxStep().",
        "Unexpected return value from CS::findFirstMax() in CS::firstMin_MaxStep().",
        "Next pixel was not determined in CS::findMin().",
        "Next pixel was not determined in CS::findMax().",
        "Next pixel was not determined in CS::findFirstMax().",
        "Next pixel was not determined in CS::findKFP().",
        "Next pixel was not determined in CS::findFarthestX().",
        "Next pixel was not determined in CS::loopAdvance().",
        "Next pixel was not determined in CS::separate_YE().",
        "Not enough memory to allocate 'struct Segment' in CS::Min_Max_Partition().",
        "A dangling loop was found, but last segment already has a loop.",
        "Next pixel was not determined in CS::determineSegments().",
        "Next pixel was not determined in CS::extractMIDDLEletterMFSg().",
        "Next pixel was not determined in CS::extract_all_pixel_MFSg().",
        "Next pixel was not determined in CS::removeReachingBranch().",
        "Next pixel was not determined in CS::END_SINGLE_MFSgPixels().",
        "Next pixel was not determined in CS::branchLookAhead().",
        "Next pixel was not determined in CS::extractBEGINNINGletterMFSg().",
        "Next pixel was not determined in CS::extractENDletterMFSg().",
        "Neighbour MFSg pixel could not be found in CS::chainCode().",
        "Next pixel was not determined in CS::chainCode().",
        "Grid spacing has to be an even number ( CS::chainCode() reports. )",
        "Not enough memory to allocate 'struct MFSegment' in CS::chain_code_MFSgs().",
        "Specified directory for training set does not exist !",
        "Error opening MFSg training set file in CS::save_MFSgs().",
        "Not enough memory to allocate 'a' in HMM::HMM().",
        "Not enough memory to allocate 'b' in HMM::HMM().",
        "Not enough memory to allocate 'pi' in HMM::HMM().",
        "Not enough memory to allocate 'a[i]' in HMM::HMM().",
        "Not enough memory to allocate 'b[j]' in HMM::HMM().",
        "Not enough memory to allocate 'Alpha' in HMM::getObsVector().",
        "Not enough memory to allocate 'Alpha[t]' in HMM::getObsVector().",
        "There are no trained models of the specified type.",
        "Error opening trained model file."
};

#define HEAP_CONTROL        sprintf(msg, "Avail. mem. %ld", HEAP.avail()); \
                            MESSAGE.Show(msg, REFRESH);

#define Heap_Control        if (farheapcheck() == _HEAPCORRUPT) {     \
                                appError = HEAP_ERROR;                \
                                return(retVal=RTN_ERROR);             \
                            }

void exitFunc1 (void)
// ----------------------------------------------------------------------------
// first registered (in main) exit function by 'atexit'
// ----------------------------------------------------------------------------
{ char prompt1[]="Error : %s\n";

  if (appError)
    fprintf(stderr, prompt1, appErrList[appError]);
}                  

main (int argc, char *argv[]) 
// ----------------------------------------------------------------------------
//                                 M  A  I  N
// ----------------------------------------------------------------------------
{ 
#define mVOID   0
#define mGO     1
#define mNEW    2
#define mCUTOUT 3
#define mDB     4
#define mEXIT   5
#define mLEFT   6
#define mRIGHT  7
#define mUP     8
#define mDOWN   9

#define EVALmode        0
#define SAVEmode        1

int execMode = SAVEmode;

char *menuCommands[] = { 
                "Go", "New", "CutOut", "DB", "Exit",
                "\x1B", "\x1A", "\x18", "\x19"
};

#define MENU_ITEMS      sizeof(menuCommands) / sizeof(char *)

  char strImgFile[60], msg[80], bgiDir[60]="c:\\tcpp\\bgi",
        trainSetDir[80]=".\\trainset", modelDir[80]=".\\model",
        prompt1[]="Enter image filename";
  int mosPos, mnuCmd, x, y;
  int width, depth, baseTop, baseBottom, i;
  unsigned char huge *frameCut;

  extern int CS_Ops (CS *, Screen *, char *, char *, int);

  atexit(exitFunc1);

  for (i=1; i<argc; i++) 

    if (*argv[i] == '-')

        switch( *(argv[i]+1) ) {

            case 'g': strcpy(bgiDir, argv[i]+2);
                      break;

            case 't': strcpy(trainSetDir, argv[i]+2);
                      break;

            case 'm': strcpy(modelDir, argv[i]+2);
                      break;

            case 'x': execMode = EVALmode;
                      break;

            default : break;    
        }

 
  Screen screen(bgiDir);                          _o_; 

  while (appError != END_PROGRAM) {

    screen.mode(TEXT_8025);                       _f_; 

    FileName imgFile(strImgFile, prompt1);
    if (appError == END_PROGRAM)
        continue;

    screen.mode(GRAPH_2);                         _f_; 

    Menu menu(MENU_ITEMS, menuCommands, &screen); _o_; 

    Image image(strImgFile, &screen);             _o_; 

    screen.showMouseCursor();

    do {
        image.showImgSize(REFRESH);
        sprintf(msg, ",  Avail. mem. %ld", HEAP.avail()); 
        MESSAGE.Show(msg, CONCAT);

        mnuCmd = mVOID;
        mosPos = screen.controlScr(x, y);

        if (mosPos == ON_MENU) {
            mnuCmd = menu.menuCommandSelect(x, y, &screen);

            switch(mnuCmd) {

                case mEXIT   : appError = END_PROGRAM;
                               break;

                case mNEW    : break;

                case mLEFT   : screen.hideMouseCursor();
                               image.leftPan();
                               screen.showMouseCursor();
                               break;

                case mRIGHT  : screen.hideMouseCursor();
                               image.rightPan();
                               screen.showMouseCursor();
                               break;

                case mUP     : screen.hideMouseCursor();
                               image.scrollUp();
                               screen.showMouseCursor();
                               break;

                case mDOWN   : screen.hideMouseCursor();
                               image.scrollDown();
                               screen.showMouseCursor();
                               break;

                case mDB     : image.baselineGen(&screen);
                               break;
                                
                case mCUTOUT : frameCut = NULL;
                        
                               image.frameCutOut(width, depth, frameCut, baseTop, baseBottom,
                                                 menu.getMenuHeight());   _f_;

                               if (frameCut == NULL)
                                   // selected frame was not large enough.
                                   break;
                               
                               CS coStruct (width, depth, frameCut, baseTop, baseBottom);

                               CS_Ops (&coStruct, &screen, trainSetDir, modelDir, execMode);  _f_;

                               HEAP.free(frameCut);

                               screen.mode(GRAPH_2);              _f_;
                               menu.Show(menuCommands, &screen);  _f_;
                               image.Show();
                               screen.showMouseCursor();

                               break;
                                
                case mGO     : beep();
                               break;
                                 
                default      : break;
            }
        }

        else if (mosPos == ON_IMAGE)
            image.frameGen(x, y, &screen);

    } while (mnuCmd != mNEW  &&  mnuCmd != mEXIT);

    screen.hideMouseCursor();
  }

  return(EXIT_SUCCESS);
}


int CS_Ops (CS *coStruct, Screen *scr, char *trnDir, char *modelDir, int xMode)
// ----------------------------------------------------------------------------
// xMode == 0   then    Eval appears (recognition mode)
// xMode == 1   then    Save appears (training data gathering mode)
// ----------------------------------------------------------------------------
{ char *menuCommands[] = { 

        "Flood", "LP", "Smooth", "Compo", "Thin", "Loops", "KFP", 
        "Seg", "MFSg", "Cc", "Save", "Return"
  };

#define MENU_ITEMS          sizeof(menuCommands) / sizeof(char *)

#define CONDITIONAL_break1  if (mnuCmd < lastOp)    mnuCmd++;          \
                            else {                                     \
                                coStruct->Show(mnuH);                  \
                                break;  }

#define CONDITIONAL_break2  if (mnuCmd < lastOp)    mnuCmd++;          \
                            else {                                     \
                                coStruct->Show(mnuH);                  \
                                coStruct->displayStartPixel(mnuH);     \
                                break;  }

#define CONDITIONAL_break3  if (mnuCmd < lastOp)    mnuCmd++;          \
                            else {                                     \
                                coStruct->Show(mnuH);                  \
                                coStruct->showCutMarks(mnuH);          \
                                coStruct->displayStartPixel(mnuH);     \
                                break;  }

#define CONDITIONAL_break4  coStruct->Show(mnuH);                      \
                            coStruct->displayStartPixel(mnuH);         \
                            coStruct->showCutMarks(mnuH);              \
                            if (mnuCmd < lastOp)    mnuCmd++;          \
                            else   break
                                

#define mFLOODFILL      1
#define mLP             2
#define mSMOOTH         3
#define mCOMPO          4
#define mTHIN           5
#define mLOOPS          6
#define mKFP            7
#define mSEGMENT        8
#define mMFSG           9
#define mCC             10
#define mSAVE_or_EVAL   11
// #define mEVAL                11
#define mRETURN         12



  int mosPos, mnuCmd, x, y, mnuH, lastOp, ch;

  if (xMode == EVALmode)
        strcpy(menuCommands[10], "Eval");

  scr->mode(GRAPH_16);                           _f_;

  DumbMenu dmenu(MENU_ITEMS, menuCommands, scr); _o_;

  mnuH = dmenu.getMenuHeight();
  coStruct->Show(mnuH);
  coStruct->vertProject(mnuH);

  scr->showMouseCursor();
  do {

    mnuCmd = mVOID;
    mosPos = scr->controlScr(x, y);

    if (mosPos == ON_MENU) {
        mnuCmd = dmenu.menuCommandSelect(x, y, scr, lastOp);

        scr->hideMouseCursor();
        switch(mnuCmd) {

            case mFLOODFILL: coStruct->Framefloodfill();
                             CONDITIONAL_break1

            case mLP       : coStruct->loopProcess();     _f_;
                             CONDITIONAL_break1

            case mSMOOTH   : coStruct->Framesmooth();     
                             CONDITIONAL_break1

            case mCOMPO    : coStruct->components();          _f_;
                             CONDITIONAL_break2

            case mTHIN     : coStruct->thin();                _f_;
                             CONDITIONAL_break2

            case mLOOPS    : coStruct->setLoops();            _f_;
                             CONDITIONAL_break2

            case mKFP      : coStruct->keyFeaturePoints();
                             if ((ch=getch()) == 'b')  coStruct->saveAsBMP();
                             CONDITIONAL_break2  

            case mSEGMENT  : coStruct->Min_Max_Partition();   _f_;
                             CONDITIONAL_break3

            case mMFSG     : coStruct->findMFSg();            _f_;
                             CONDITIONAL_break4;

            case mCC       : coStruct->chain_code_MFSgs(2);   _f_;
                             if (mnuCmd < lastOp)    mnuCmd++;
                             else                    break;

            case mSAVE_or_EVAL :

                           if (xMode == SAVEmode) {     

                             coStruct->save_MFSgs(trnDir);    _f_;
                             if (mnuCmd < lastOp)    mnuCmd++;
                             else                    break;
                           }

                           else {

                             scr->mode(TEXT_8025);            _f_; 

                             coStruct->classify(modelDir);    _f_;

                             scr->mode(GRAPH_16);             _f_;
                             dmenu.Show(menuCommands, scr);   _f_;
                             coStruct->Show(mnuH);
                             coStruct->showCutMarks(mnuH);
                             break;
                           }

            case mRETURN   : return(retVal=RTN_OK);

            default        : break;
        }

        scr->showMouseCursor();
    }

  } while (TRUE);

}


void CS::saveAsBMP()
// ----------------------------------------------------------------------------// ----------------------------------------------------------------------------
{ long matrixLen = (long)Width * Depth;
  unsigned char huge *csPtr = Cs;
  struct BMPHEAD {
        char id[2];
        long filesize;
        int reserved[2];
        long headersize;
        long infoSize;
        long width;
        long depth;
        int biPlanes;
        int bits;
        long biCompression;
        long biSizeImage;
        long biX;
        long biY;
        long biClrUsed;
        long biClrImp;
        long palette[16];
  };
  struct BMPHEAD bh;
  int fhBmp, i, j;
  char dummyBmp[]="c:\\windows\\boxes.bmp", fBmp[]="cs.bmp";
  unsigned char twoPixel;

  if ((fhBmp = open(dummyBmp, O_RDONLY | O_BINARY)) == NULL)   return;
  read(fhBmp, &bh, sizeof(struct BMPHEAD));
  close(fhBmp);

  bh.width = (Width/8) * 8;
  bh.depth = Depth;
  bh.biSizeImage = bh.width * bh.depth / 2;
  bh.filesize = bh.headersize + bh.biSizeImage;

  fhBmp = open(fBmp, O_CREAT | O_BINARY | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
  write(fhBmp, &bh, sizeof(struct BMPHEAD));

  csPtr += matrixLen;

  for (i=Depth; i>0; i--) {

    csPtr -= Width;

    for (j=0; j<bh.width; j += 2) {

        if ((*(csPtr+j) | foreGrnd) == foreGrnd)
            twoPixel = 0x00;
        else
            twoPixel = 0xF0;

        if ((*(csPtr+j+1) | foreGrnd) == foreGrnd)
            twoPixel &= 0xF0;
        else
            twoPixel |= 0x0F;

        write(fhBmp, &twoPixel, sizeof(unsigned char));
    }

  }

  close(fhBmp);
} 
/*
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ long matrixLen = (long)Width * Depth;
  unsigned char huge *csPtr = Cs;
  struct BMPHEAD {
        char id[2];
        long filesize;
        int reserved[2];
        long headersize;
        long infoSize;
        long width;
        long depth;
        int biPlanes;
        int bits;
        long biCompression;
        long biSizeImage;
        long biX;
        long biY;
        long biClrUsed;
        long biClrImp;
        long palette[16];
  };
  struct BMPHEAD bh;
  int fhBmp, wdth, i, j, k, enlargeF=6, wCount;
  char dummyBmp[]="c:\\windows\\boxes.bmp", fBmp[]="cs.bmp";
  unsigned char fPix[6][3] = { 
                        { 0x00, 0x00, 0x00 },
                        { 0x0F, 0xFF, 0xF0 },
                        { 0x0F, 0xFF, 0xF0 },
                        { 0x0F, 0xFF, 0xF0 },
                        { 0x0F, 0xFF, 0xF0 },
                        { 0x00, 0x00, 0x00 }
  };
  unsigned char bPix[3] = { 0xFF, 0xFF, 0xFF };
//  long bmPos, pSav, offSet1;

  if ((fhBmp = open(dummyBmp, O_RDONLY | O_BINARY)) == NULL)   return;
  read(fhBmp, &bh, sizeof(struct BMPHEAD));
  close(fhBmp);

  wdth = Width * enlargeF;

  bh.width = (wdth/8) * 8;
  bh.depth = Depth * enlargeF;
  bh.biSizeImage = bh.width * bh.depth / 2;
  bh.filesize = bh.headersize + bh.biSizeImage;

  fhBmp = open(fBmp, O_CREAT | O_BINARY | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
  write(fhBmp, &bh, sizeof(struct BMPHEAD));


  csPtr += matrixLen;

  for (i=Depth; i>0; i--) {

    csPtr -= Width;

    for (k=0; k<enlargeF; k++) {

        wCount = 0;

        for (j=0; j<Width; j++) {

            wCount += enlargeF;

            if (wCount > bh.width) {
                wCount -= enlargeF;
                if (wCount < bh.width)
                    for ( ; wCount < bh.width; wCount += 2)
                        write(fhBmp, bPix, 1);

                break;
            }

            if ((*(csPtr+j) | foreGrnd) == foreGrnd) 
                write(fhBmp, &fPix[k][0], 3);
            else
                write(fhBmp, bPix, 3);
        }

    }

  }

  close(fhBmp);
}
*/
