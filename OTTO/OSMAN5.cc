/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#include <stdio.h>
#include <conio.h>
#include <graphics.h>
#include <string.h>
#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>
#include "otto.h"
#include "classdef.h"
#include "cs.h"

extern int appError;
extern int retVal;
extern Message MESSAGE;
extern Heap HEAP;

int CS::chain_code_MFSgs (int gridSpace)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ struct Segment *segTmp=lSegs;
  struct MFSegment *mfsTmp;  //  *lastMfs;

  while (segTmp != NULL) {

    if (segTmp->loopExist == FALSE) {

        mfsTmp = (struct MFSegment *) HEAP.alloc(sizeof(struct MFSegment));
        if (mfsTmp == NULL) {
            appError = MFSG_ALLOC;
            return(retVal=RTN_ERROR);
        }

        chainCode(mfsTmp, segTmp->stFeature, segTmp->endFeature, gridSpace);

        if (retVal != RTN_OK)    return(retVal);

        segTmp->mfsg = mfsTmp;

//      showMFSgChainCodes(mfsTmp, gridSpace, 16);
//      (void) getch();
//      Show(16);

        smoothChainCodes_1(mfsTmp->chainCode);
        smoothChainCodes_2(mfsTmp->chainCode);

        showMFSgChainCodes(mfsTmp, gridSpace, 16);
        (void) getch();
    }

    segTmp = segTmp->next;
  }

  return(retVal=RTN_OK);
}
        

void CS::showMFSgChainCodes (struct MFSegment *fCC, int gridSpacing, int menuHeight)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int left, top, scrHeight, xSize, xFactor, yFactor;
  int y, x, prev_y, prev_x, ccIdx, gridFactor;
  char msg[]="Frame too large to be displayed !", chain_code, msgline[80],
       prompt[]=" , length=%2d";

  scrHeight = getmaxy() - menuHeight - MESSAGE.height() - 1;

  if ((scrHeight / Depth) < 1) {
      MESSAGE.Show(msg, REFRESH);
      return;
  }
  else if ((scrHeight / Depth) < 2)
      yFactor = 1;

  else
      yFactor = 2;


  xSize = getmaxx() / Width;
  if (xSize < 2)
      xFactor = 1;

  else if (xSize < 4)
      xFactor = 2;

  else
      xFactor = 4;

  left = (getmaxx() - (Width * xFactor)) / 2;
  top  = (scrHeight - (Depth * yFactor)) / 2  +  menuHeight;


  setcolor(WHITE);
  setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

  ccIdx = 0;
  gridFactor = gridSpacing + 1;
#pragma warn -sig
  prev_y = top  +  (fCC->startY * yFactor);
  prev_x = left +  (fCC->startX * xFactor);
#pragma warn +sig

  while (chain_code=fCC->chainCode[ccIdx]) {

    switch(chain_code) {

        case '0': x = prev_x;    
                  y = prev_y - (gridFactor * yFactor);
                  break;

        case '1': x = prev_x + (gridFactor * xFactor);
                  y = prev_y - (gridFactor * yFactor);
                  break;

        case '2': x = prev_x + (gridFactor * xFactor);
                  y = prev_y;
                  break;

        case '3': x = prev_x + (gridFactor * xFactor);
                  y = prev_y + (gridFactor * yFactor);
                  break;

        case '4': x = prev_x;    
                  y = prev_y + (gridFactor * yFactor);
                  break;

        case '5': x = prev_x - (gridFactor * xFactor);
                  y = prev_y + (gridFactor * yFactor);
                  break;

        case '6': x = prev_x - (gridFactor * xFactor);
                  y = prev_y;
                  break;

        case '7': x = prev_x - (gridFactor * xFactor);
                  y = prev_y - (gridFactor * yFactor);
                  break;

    }

    line(prev_x, prev_y, x, y);

    prev_x = x;
    prev_y = y;
    ccIdx++;
  }

  MESSAGE.Show(fCC->chainCode, REFRESH);
  sprintf(msgline, prompt, ccIdx);
  MESSAGE.Show(msgline, CONCAT);
}


int CS::chainCode (struct MFSegment *fCC, long stFeature, long endFeature, 
                   int gridSpacing)
// ----------------------------------------------------------------------------
// We perform chain-coding via counter-clockwise skeleton tracing on MFSg pixels.
// Direction of skeleton tracing is important, because clockwise tracing may
// lead to a jump to next segment before the current one is exhausted.
// ----------------------------------------------------------------------------
{ int k, direction, found, minX, minY, xVal, yVal, gridFactor, midGridSpace,
      ccIdx, gridX, gridY, prev_gridX, prev_gridY;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long j;
  char chain_code;


  // 'gridSpacing' is the number of pixels between two nodes on the new
  // resampling grid. This value has to be an even number. 
  if (gridSpacing % 2  ==  1) {
    appError = BAD_GRIDSP;
    return(retVal=RTN_ERROR);
  }

  j = stFeature;

  found = FALSE;
  for (k=0; found == FALSE  &&  k<8; )  
    if ( (*(Cs+j+neighOffset[k]) & MFSgPixel) == MFSgPixel   &&
         (*(Cs+j+neighOffset[k]) & visitedMFSgPixel) != visitedMFSgPixel )
            found = TRUE;
    else
        k++;


  if (found == FALSE) {
    appError = NEXT_MFSGPIX;
    return(retVal=RTN_ERROR);
  }


  pvDirection = direction = (8 + k - 2) % 8;

  // find the position of leftmost and top MFSg pixels.
#pragma warn -sig
  minX = j % Width;
  minY = j / Width;
#pragma warn +sig

  do {

    direction = (8 + direction + 2) % 8;  // initially add -90 deg.
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+j+neighOffset[direction]) & MFSgPixel) == MFSgPixel)
            found = TRUE;
        else
            direction = (8 + direction - 1) % 8;  // add +45 deg.

    if (found) {
      
        j += neighOffset[direction];

#pragma warn -sig
        xVal = j % Width;
        yVal = j / Width;
#pragma warn +sig

        if (xVal < minX)    
            minX = xVal;

        if (yVal < minY)    
            minY = yVal;


        if (j == endFeature)
            break;

      }

      else {
          appError = NEXT_POINT16;
          return(retVal=RTN_ERROR);
      }
  
  } while (TRUE);


  j = stFeature;

  gridFactor = gridSpacing + 1;
  midGridSpace = gridSpacing / 2;

#pragma warn -sig
  xVal = j % Width;
  yVal = j / Width;
#pragma warn +sig

  prev_gridX = minX  +  ((xVal - minX) / gridFactor) * gridFactor;
  if ( (xVal - minX) % gridFactor  >  midGridSpace)
      prev_gridX += gridFactor;

  prev_gridY = minY  +  ((yVal - minY) / gridFactor) * gridFactor;
  if ( (yVal - minY) % gridFactor  >  midGridSpace)
      prev_gridY += gridFactor;

  fCC->startX = prev_gridX;
  fCC->startY = prev_gridY;

  *(Cs+j) |= visitedMFSgPixel;
  direction = pvDirection;
  ccIdx = 0;

  do {

    direction = (8 + direction + 2) % 8;  // initially add -90 deg.
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+j+neighOffset[direction]) & MFSgPixel) == MFSgPixel  &&
             (*(Cs+j+neighOffset[direction]) & visitedMFSgPixel) != visitedMFSgPixel )  
            found = TRUE;
        else
            direction = (8 + direction - 1) % 8;  // add +45 deg.

    if (found) {

        j += neighOffset[direction];

        *(Cs+j) |= visitedMFSgPixel;

#pragma warn -sig
        xVal = j % Width;
        yVal = j / Width;
#pragma warn +sig

        gridX = minX  +  ((xVal - minX) / gridFactor) * gridFactor;
        if ( (xVal - minX) % gridFactor  >  midGridSpace)
            gridX += gridFactor;

        gridY = minY  +  ((yVal - minY) / gridFactor) * gridFactor;
        if ( (yVal - minY) % gridFactor  >  midGridSpace)
            gridY += gridFactor;


        if (gridX > prev_gridX)
            if (gridY > prev_gridY) 
                chain_code = '3';
            else if (gridY == prev_gridY)
                chain_code = '2';
            else
                chain_code = '1';
        else if (gridX == prev_gridX)
            if (gridY > prev_gridY) 
                chain_code = '4';
            else if (gridY == prev_gridY)
                chain_code = '*';
            else
                chain_code = '0';
        else {
            if (gridY > prev_gridY) 
                chain_code = '5';
            else if (gridY == prev_gridY)
                chain_code = '6';
            else
                chain_code = '7';
        }


        if (chain_code != '*') {
            fCC->chainCode[ccIdx++] = chain_code;
            prev_gridX = gridX;
            prev_gridY = gridY;
        }

        if (j == endFeature) {
            fCC->chainCode[ccIdx] = '\x0';  // terminate chain code string.
            return(retVal=RTN_OK);
        }

    }

    else {
        appError = NEXT_POINT16;
        return(retVal=RTN_ERROR);
    }
  
  } while (TRUE);

}

void CS::smoothChainCodes_1 (char *ccStr)
// ----------------------------------------------------------------------------
// This fcn. removes step-like shapes from the string of chain codes. 
// The resultant string becomes smoother and shorter.
// ----------------------------------------------------------------------------
{ char wrkStr[MAX_CCString];
  int i, j;

  i = j = 0;
  strcpy(wrkStr, ccStr);

  while (ccStr[i]) 

    if ( (ccStr[i] == '0'  &&  ccStr[i+1] == '2') || (ccStr[i] == '2'  &&  ccStr[i+1] == '0') ) {
      wrkStr[j++] = '1';
      i += 2;
    }
    else if ( (ccStr[i] == '2'  &&  ccStr[i+1] == '4') || (ccStr[i] == '4'  &&  ccStr[i+1] == '2') ) {
      wrkStr[j++] = '3';
      i += 2;
    }
    else if ( (ccStr[i] == '4'  &&  ccStr[i+1] == '6') || (ccStr[i] == '6'  &&  ccStr[i+1] == '4') ) {
      wrkStr[j++] = '5';
      i += 2;
    }
    else if ( (ccStr[i] == '6'  &&  ccStr[i+1] == '0') || (ccStr[i] == '0'  &&  ccStr[i+1] == '6') ) {
      wrkStr[j++] = '7';
      i += 2;
    }

    else
      wrkStr[j++] = ccStr[i++];


  wrkStr[j] = '\x0';
  strcpy(ccStr, wrkStr);
}

void CS::smoothChainCodes_2 (char *ccStr)
// ----------------------------------------------------------------------------
// This fcn. is a second pass for smoothing the string chain codes.
// The resultant string becomes even smoother and shorter.
// ----------------------------------------------------------------------------
{ char wrkStr[MAX_CCString];
  int i, j;

  i = j = 0;
  strcpy(wrkStr, ccStr);

  while (ccStr[i]) 

    if ( (ccStr[i] == '1'  &&  ccStr[i+1] == '4') || (ccStr[i] == '0'  &&  ccStr[i+1] == '3') ||
         (ccStr[i] == '4'  &&  ccStr[i+1] == '1') || (ccStr[i] == '3'  &&  ccStr[i+1] == '0') ) {
      wrkStr[j++] = '2';
      i += 2;
    }
    else if ( (ccStr[i] == '1'  &&  ccStr[i+1] == '6') || (ccStr[i] == '2'  &&  ccStr[i+1] == '7') ||
              (ccStr[i] == '6'  &&  ccStr[i+1] == '1') || (ccStr[i] == '7'  &&  ccStr[i+1] == '2') ) {
      wrkStr[j++] = '0';
      i += 2;
    }
    else if ( (ccStr[i] == '0'  &&  ccStr[i+1] == '5') || (ccStr[i] == '7'  &&  ccStr[i+1] == '4') ||
              (ccStr[i] == '5'  &&  ccStr[i+1] == '0') || (ccStr[i] == '4'  &&  ccStr[i+1] == '7') ) {
      wrkStr[j++] = '6';
      i += 2;
    }
    else if ( (ccStr[i] == '3'  &&  ccStr[i+1] == '6') || (ccStr[i] == '2'  &&  ccStr[i+1] == '5') ||
              (ccStr[i] == '6'  &&  ccStr[i+1] == '3') || (ccStr[i] == '5'  &&  ccStr[i+1] == '2') ) {
      wrkStr[j++] = '4';
      i += 2;
    }

    else
      wrkStr[j++] = ccStr[i++];


  wrkStr[j] = '\x0';
  strcpy(ccStr, wrkStr);
}


int CS::save_MFSgs (char *trainSetDir)
// ----------------------------------------------------------------------------
// This fcn. generates the set of MFSg files that will be used to train HMMs.
// File names of corresponding models for each segment in the 'lSegs' list 
// are input from user. If the user does not want a specific segment to add 
// to its corresponding training set, he has to enter a single space as filename.
// A segment that already appears in the training set is not saved again.
// ----------------------------------------------------------------------------
{ struct Segment *segTmp=lSegs;
  char mfsFile[80], fname[30], fmt[]="Enter file name for segment %1d: ",
       bSlash[]="\\", dExt[]=".TRN",
       prompt[50], CRLF='\xA', EOF_char='\x1A', ch, buf[MAX_CCString], *bPtr;
  int mCount=1, fh, sameStringFound;

  if (access(trainSetDir, 0) == -1) {
    appError = NO_TRNDIR;
    return(retVal=RTN_ERROR);
  }

  while (segTmp != NULL) {

    if (segTmp->mfsg != NULL) {

        sprintf(prompt, fmt, mCount);
        MESSAGE.getInp(prompt, fname);

        if (fname[0] != ' '  &&  fname[0] != '\x0') {

            strcpy(mfsFile, trainSetDir);
            strcat(mfsFile, bSlash);
            strcat(mfsFile, fname);
            strcat(mfsFile, dExt);

            if (access(mfsFile, 0) == 0) {
                // file exists
                if ((fh=open(mfsFile, O_RDWR | O_TEXT)) == NULL) {
                    appError = MFSGF_OPEN;
                    return(retVal=RTN_ERROR);
                }
            }
        
            else {

                if ((fh=open(mfsFile, O_RDWR | O_CREAT | O_TEXT, S_IREAD | S_IWRITE)) == NULL) {
                    appError = MFSGF_OPEN;
                    return(retVal=RTN_ERROR);
                }
                write(fh, &EOF_char, sizeof(char));
                lseek(fh, 0L, SEEK_SET);
            }

            bPtr = buf;
            sameStringFound = FALSE;

            while (read(fh, &ch, sizeof(char)) != 0)

                if (ch == CRLF) {

                    *bPtr = '\x0';

                    if (strcmp(segTmp->mfsg->chainCode, buf) == 0) {
                        close(fh);
                        sameStringFound = TRUE;
                        break;
                    }

                    bPtr = buf;
                }

                else
                    *bPtr++ = ch;


            if (sameStringFound == FALSE) {
                lseek(fh, -1L, SEEK_END);
                write(fh, segTmp->mfsg->chainCode, strlen(segTmp->mfsg->chainCode));
                write(fh, &CRLF, sizeof(char));
                write(fh, &EOF_char, sizeof(char));
                close(fh);
            }

        }
    }

    mCount++;
    segTmp = segTmp->next;
  }

  return(retVal=RTN_OK);
}