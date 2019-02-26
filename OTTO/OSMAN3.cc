/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#include <stdio.h>
#include <graphics.h>
#include <mem.h>
#include <conio.h>

#include "otto.h"
#include "classdef.h"
#include "cs.h"

extern int appError;
extern int retVal;
extern Message MESSAGE;
extern Heap HEAP;

extern void beep (void);


CS::CS (int w, int d, unsigned char huge *p, int baseTop, int baseBottom) : 
        Width(w), Depth(d), Cs(p)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int offset[9] = {
                -Width-1, -Width, -Width+1,
                      -1,      0,        1,
                 Width-1,  Width,  Width+1  
  };
  int i=-1;

  MinBaseLine = (long)baseTop * Width;
  MaxBaseLine = (long)baseBottom * Width;

  MainCompo = CompoList = (struct component *) 0;
  LoopCenter = (struct lpCenter *) 0;
  MinList = (struct CutMark *) 0;
  lSegs = (struct Segment *) 0;
//  CCFeature = (struct MFSegment *) 0;

  while (++i < 9)
    NeighMask[i] = offset[i];
}

CS::~CS ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ struct component *compTmp;
  struct lpCenter *lpTmp;
  struct CutMark *ctTmp;
  struct Segment *segTmp;
//  struct MFSegment *mfsTmp;

/*
  while (CCFeature != NULL) {
    mfsTmp = CCFeature->next;
    HEAP.free(CCFeature);
    CCFeature = mfsTmp;
  }
*/

  while (lSegs != NULL) {
    segTmp = lSegs->next;
    if (lSegs->mfsg != NULL)    
        HEAP.free(lSegs->mfsg);
    HEAP.free(lSegs);
    lSegs = segTmp;
  }

  while (MinList != NULL) {
    ctTmp = MinList->next;
    HEAP.free(MinList);
    MinList = ctTmp;
  }

  while (LoopCenter != NULL) {
    lpTmp = LoopCenter->next;
    HEAP.free(LoopCenter);
    LoopCenter = lpTmp;
  }

  while (CompoList != NULL) {
    compTmp = CompoList->next;
    HEAP.free(CompoList);
    CompoList = compTmp;
  }

}


void CS::Show (int menuHeight)
// ----------------------------------------------------------------------------
// displays CS frame on screen. 640x200, 16-color mode is assumed.
// ----------------------------------------------------------------------------
{ int fillColor = -1;
  int t, l, b, r, i, j;
  int left, top, xSize, xFactor, yFactor, scrHeight;
  unsigned char huge *csPtr = Cs;
  char msg[]="Frame too large to be displayed !";

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


  for (t=top, b=top+yFactor-1, j=0; j<Depth; t+=yFactor, b+=yFactor, j++)

    for (l=left, r=left+xFactor-1, i=0; i<Width; l+=xFactor, r+=xFactor, i++) {

        switch (*csPtr) {

            case BACKGRND  : if (fillColor != WHITE) {
                                 fillColor = WHITE;
                                 setfillstyle(SOLID_FILL, WHITE);
                             }
                             break;

            case FLOODed   : if (fillColor != BLUE) {
                                 fillColor = BLUE;
                                 setfillstyle(SOLID_FILL, BLUE);
                             }
                             break;

            case FOREGRND  : if (fillColor != BLACK) {
                                 fillColor = BLACK;
                                 setfillstyle(SOLID_FILL, BLACK);
                             }
                             break;

            case LPCONTOUR : fillColor = LIGHTGRAY;
                             setfillstyle(SOLID_FILL, LIGHTGRAY);
                             break;

            case CONTOUR   : fillColor = YELLOW;
                             setfillstyle(SOLID_FILL, YELLOW);
                             break;

            case TWINLPPIX : fillColor = LIGHTBLUE;
                             setfillstyle(SOLID_FILL, LIGHTBLUE);
                             break;
        
            case LPCENTER  : fillColor = GREEN;
                             setfillstyle(SOLID_FILL, GREEN);
                             break;
        
            default        : if ( (*csPtr & EndPoint) == EndPoint) {
                                 fillColor = LIGHTCYAN;
                                 setfillstyle(SOLID_FILL, LIGHTCYAN);
                             }
                             else if ( (*csPtr & BranchPoint) == BranchPoint) {
                                 fillColor = LIGHTRED;
                                 setfillstyle(SOLID_FILL, LIGHTRED);
                             }
                             else if ( (*csPtr & CrossPoint) == CrossPoint) {
                                 fillColor = YELLOW;
                                 setfillstyle(SOLID_FILL, YELLOW);
                             }

                             if ( (*csPtr & MFSgPixel) == MFSgPixel) {
                                 fillColor = BROWN;
                                 setfillstyle(SOLID_FILL, BROWN);
                             }

                             break;
        }

        bar(l, t, r, b);
        csPtr++;
    }

}

/*
void CS::displayStep (long i, int color)
// ----------------------------------------------------------------------------
{ int left, top, scrHeight, yFactor, xFactor, xSize;
  int menuHeight = 16;
  int t, l;
  unsigned char stPVal = *(Cs+i);
  char msg[]="Frame too large to be displayed !";

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


#pragma warn -sig
  t = top + (i / Width) * yFactor;
  l = left + (i % Width) * xFactor;
#pragma warn +sig

  setfillstyle(SOLID_FILL, color);
  bar (l, t, l+xFactor-1, t+yFactor-1);
  (void) getch();

  switch (stPVal) {

        case FOREGRND  : setfillstyle(SOLID_FILL, BLACK);
                             break;

        case LPCONTOUR : setfillstyle(SOLID_FILL, LIGHTGRAY);
                             break;

        case CONTOUR   : setfillstyle(SOLID_FILL, YELLOW);
                             break;

        case TWINLPPIX : setfillstyle(SOLID_FILL, LIGHTBLUE);
                             break;
        
        default        : if ( (stPVal & EndPoint) == EndPoint)
                                 setfillstyle(SOLID_FILL, LIGHTCYAN);
                         else if ( (stPVal & BranchPoint) == BranchPoint) 
                                 setfillstyle(SOLID_FILL, LIGHTRED);
                         else if ( (stPVal & CrossPoint) == CrossPoint) 
                                 setfillstyle(SOLID_FILL, YELLOW);

                         break;
  }

  bar (l, t, l+xFactor-1, t+yFactor-1);

}
*/

#pragma argsused

void CS::displayStep (long i, int color)
{
  return;
}


void CS::displayStartPixel (int menuHeight)
// ----------------------------------------------------------------------------
{ int left, top, xFactor, yFactor, xSize, scrHeight;
  int t, l;
  unsigned char stPVal;
  char msg[]="Frame too large to be displayed !";

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

#pragma warn -sig
  t = top + (MainCompo->startPixel / Width) * yFactor;
  l = left + (MainCompo->startPixel % Width) * xFactor;
#pragma warn +sig


  setfillstyle(SOLID_FILL, LIGHTGREEN);
  bar (l, t, l+xFactor-1, t+yFactor-1);
//  (void) getch();

  stPVal = *(Cs + MainCompo->startPixel);
  switch (stPVal) {

        case FOREGRND  : setfillstyle(SOLID_FILL, BLACK);
                             break;

        case LPCONTOUR : setfillstyle(SOLID_FILL, LIGHTGRAY);
                             break;

        case CONTOUR   : setfillstyle(SOLID_FILL, YELLOW);
                             break;

        case TWINLPPIX : setfillstyle(SOLID_FILL, LIGHTBLUE);
                             break;
        
        default        : if ( (stPVal & EndPoint) == EndPoint) 
                                 setfillstyle(SOLID_FILL, LIGHTCYAN);
                         else if ( (stPVal & BranchPoint) == BranchPoint) 
                                 setfillstyle(SOLID_FILL, LIGHTRED);
                         else if ( (stPVal & CrossPoint) == CrossPoint)
                                 setfillstyle(SOLID_FILL, YELLOW);

                         if ( (stPVal & MFSgPixel) == MFSgPixel) 
                             setfillstyle(SOLID_FILL, BROWN);

                         break;
  }

  bar (l, t, l+xFactor-1, t+yFactor-1);
}


void CS::showCutMarks (int menuHeight)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int left, top, scrHeight, xSize, xFactor, yFactor;
  int y, x;
  struct CutMark *ctPtr = MinList;
  char msg[]="Frame too large to be displayed !";

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


  setcolor(YELLOW);

  while (ctPtr != NULL) {

#pragma warn -sig
      y = top + (ctPtr->position / Width) * yFactor  - 1;
      x = left + (ctPtr->position % Width) * xFactor + 1;
#pragma warn +sig

      line(x, y, x, y+3);

      ctPtr = ctPtr->next;
  }

}


void CS::vertProject (int menuHeight)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int top, scrHeight, yFactor;
  int t, j, i, pixCount, max_pixCount, max_pixY, r = getmaxx();
  unsigned char huge *csPtr = Cs;
  char msg[]="Frame too large to be displayed !";

  scrHeight = getmaxy() - menuHeight - MESSAGE.height() - 1;

  if ((scrHeight / Depth) < 1) {
      MESSAGE.Show(msg, REFRESH);
      return;
  }
  else if ((scrHeight / Depth) < 2)
      yFactor = 1;

  else
      yFactor = 2;

  top  = (scrHeight - (Depth * yFactor)) / 2  +  menuHeight;


  setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
  setcolor(LIGHTMAGENTA);
  max_pixCount = 0;

  for (t=top, j=0; j<Depth; t+=yFactor, j++) {

    pixCount = 0;

    for (i=0; i<Width; i++)
        if (*csPtr++ == FOREGRND)
            pixCount++;

    if (pixCount > 0) {

        line(r - pixCount, t, r, t);

        if (pixCount > max_pixCount) {
            max_pixCount = pixCount;
            max_pixY = t;
        }
    }

  }

  setcolor(YELLOW);
  line(r - max_pixCount, max_pixY, r, max_pixY);

}


int CS::floodfill ()
// ----------------------------------------------------------------------------
// flood-fills the frame. This is a two-pass process : top-down flooding,
// and bottom-up flooding.
// To properly floodfill the entire frame, this function should be called
// repetitively until it returns FALSE.
// Called from 'CS::Framefloodfill()'.
// In order for a pixel to be flooded, it has to be 4-connected to flood.
// ----------------------------------------------------------------------------
{ long j;
  int i, k;
  long matrixLen = (long)Width * Depth;
  int d, w, marked=FALSE;

  w = Width - 1;
  d = Depth - 1;

  // top-down floodfill
  j = Width + 1;

  for (i=1; i<d; i++) {

    for (k=1; k<w; k++, j++)
        if ( *(Cs+j) == BACKGRND)
            // check if background pixel is 4-connected to flooded region
            if ( *(Cs+j - Width) == FLOODed  ||  *(Cs+j - 1) == FLOODed  ||
                 *(Cs+j + Width) == FLOODed  ||  *(Cs+j + 1) == FLOODed 
               )
             {
               *(Cs+j) = FLOODed;
               marked = TRUE;
             }

    j += 2;
  }

  if (marked == FALSE)    return(marked);

  // bottom-up floodfill
  j = matrixLen - Width - 2;

  for (i=d; i>1; i--) {

    for (k=w; k>1; k--, j--)
        if ( *(Cs+j) == BACKGRND)
            // check if background pixel is 4-connected to flooded region
            if ( *(Cs+j + Width) == FLOODed  ||  *(Cs+j + 1) == FLOODed  ||
                 *(Cs+j - Width) == FLOODed  ||  *(Cs+j - 1) == FLOODed 
               ) 
            {
               *(Cs+j) = FLOODed;
               marked = TRUE;
            }

    j -= 2;
  }

  return(marked);
}

void CS::Framefloodfill ()
// ----------------------------------------------------------------------------
{ long matrixLen = (long)Width * Depth;
  int d, w, i;

  w = Width - 1;
  d = Depth - 1;
  
  // floodfill the edges by default
  memset(Cs, FLOODed, w);                       // floodfill top edge.
  memset(Cs + matrixLen - w, FLOODed, Width);   // floodfill bottom edge.

  // floodfill left and right edges.
  for (i=1; i<=d; i++)                          
    memset(Cs + w + ((long)Width * (i-1)), FLOODed, 2);


  while ( floodfill() );

}

int CS::loopProcess ()
// ----------------------------------------------------------------------------
// this fcn. finds loops in the frame, enlarges  their hole so that 
// they do not get filled after smoothing, and extracts a center point 
// for each loop. 
// Also, it marks loop contour pixels for its own use, i.e. not to re-count
// a previously extracted loop. These marks are removed at the end.
// ---------------------------------------------------------------------------- 
{ long j=0, center;
  unsigned char huge *centerPix;
  struct lpCenter *lpItem;
  long matrixLen = (long)Width * Depth;
  
  while (j < matrixLen) 

    if ( *(Cs+j) == FLOODed  ||  *(Cs+j) == FOREGRND)
        j++;

    else if ( *(Cs+j) == BACKGRND) {

        // loop found
        if ((center=enlargeLoop(j-1)) == RTN_ERROR)
            return(retVal=RTN_ERROR);

        while ( *(Cs+j) == BACKGRND)    j--;

        if (markLoopPixels(j) == RTN_ERROR)
            return(retVal=RTN_ERROR);

        centerPix  = Cs + center;

        if ((lpItem = (struct lpCenter *) HEAP.alloc (sizeof(struct lpCenter)) ) == NULL) {
            appError = LPCNT_ALLOC;
            return(retVal=RTN_ERROR);
        }

        lpItem->position = center;
        lpItem->removed = TRUE;
        lpItem->twin = NULL;

        if (LoopCenter == NULL) {
            lpItem->next = NULL;
            LoopCenter = lpItem;
        }
        else {
            lpItem->next = LoopCenter;
            LoopCenter = lpItem;
        }
            
        *centerPix = LPCENTER;
//      Show(16);
//      (void) getch();
    }

    else if ( *(Cs+j) == LPCONTOUR)
        // an already extracted loop encountered. Advance until you
        // get out of it.
        while ( *(Cs+j) != FLOODed  &&  *(Cs+j) != FOREGRND)
            j++;

    else {
        appError = BAD_PIXEL;
        return(retVal=RTN_ERROR);
    }

  // remove loop contour marks
  j = -1;
  while (++j < matrixLen)
    if ( *(Cs+j) == LPCONTOUR)    *(Cs+j) = FOREGRND;

  return(retVal=RTN_OK);
}

long CS::enlargeLoop (long i)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int direction=NORTH, found, k, m, minX, maxX, minY, maxY, xVal, yVal; 
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1
  };
  long n, j, centerX, centerY;

  j=i;

#pragma warn -sig
  minX = maxX = i % Width;
  minY = maxY = i / Width;
#pragma warn +sig

  do {

    direction = (8 + direction + 2) % 8;  // initially add -90 deg.
    found = FALSE;

    for (k=0; k<8  &&  found == FALSE; k++) {
        n = j + neighOffset[direction];
        if ( *(Cs+n) == FOREGRND  ||  *(Cs+n) == LPCONTOUR)
            for (m=0; m<8  &&  found == FALSE; m += 2)
                if ( *(Cs+n+neighOffset[m]) == BACKGRND)
                    found = TRUE;

        if (found == FALSE)
            direction = (8 + direction - 1) % 8;  // add +45 deg.
    }

    if (found) {

        j = n;

#pragma warn -sig
        xVal = j % Width;
        yVal = j / Width;
#pragma warn +sig

        if      (xVal < minX)    minX = xVal;
        else if (xVal > maxX)    maxX = xVal;

        if      (yVal < minY)    minY = yVal;
        else if (yVal > maxY)    maxY = yVal;

    }

    else {
        appError = NEXT_CONTOUR1;
        return(retVal=RTN_ERROR);
    }

  } while (j != i);

  centerX = (maxX + (long)minX) / 2;
  centerY = (maxY + (long)minY) / 2;

  i = j = centerY*Width + centerX;

  *(Cs+j) = *(Cs+j+1) = BACKGRND;    j += Width;
  *(Cs+j) = *(Cs+j+1) = BACKGRND;

  return(i);
}

int CS::markLoopPixels (long i)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int direction=NORTH, found, k, m;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long n, j;

  j=i;

  do {

    direction = (8 + direction + 2) % 8;    // initially add -90 deg.
    found = FALSE;

    for (k=0; k<8  &&  found == FALSE; k++) {
        n = j + neighOffset[direction];
        if ( *(Cs+n) == FOREGRND  ||  *(Cs+n) == LPCONTOUR)
            for (m=0; m<8  &&  found == FALSE; m += 2)
                if ( *(Cs+n+neighOffset[m]) == BACKGRND ||
                     *(Cs+n+neighOffset[m]) == LPCENTER )
                    found = TRUE;

        if (found == FALSE)
            direction = (8 + direction - 1) % 8;  // add +45 deg.
    }

    if (found) {
        j = n;
        *(Cs+j) = LPCONTOUR;
    }
    else {
        appError = NEXT_CONTOUR2;
        return(retVal=RTN_ERROR);
    }

  } while (j != i);

  return (retVal=RTN_OK);
}

void CS::Framesmooth ()
// ----------------------------------------------------------------------------
// removes FLOODed and LPCENTER pixels from frame, then performs smoothing.
// ---------------------------------------------------------------------------- 
{ long matrixLen = (long)Width * Depth, j;
  struct lpCenter *lpItem;

  // restore floodfilled region to previous state (FLOODed --> BACKGRND).
  j=-1;
  while (++j < matrixLen)
    if ( *(Cs+j) == FLOODed)    *(Cs+j) = BACKGRND;

  // restore LPCENTER values back to BACKGRND.
  lpItem = LoopCenter;
  while (lpItem != NULL) {
      *(Cs + lpItem->position) = BACKGRND;
      lpItem = lpItem->next;
  }

  // now the frame is binary again, i.e. only BACKGRND and FOREGRND pixels 

  while ( smooth() );

}

int CS::smooth ()
// ----------------------------------------------------------------------------
// image frame is smoothed by applying the provided masks. 
// A match of 'peakMask' removes the pixel, match of 'holeMask' fills the
// pixel. The masks are actually 3x2 and 2x3 subset combinations of the
// given masks as utilized by 'processMask'.
// This function should be called repetitively until it returns FALSE.
// Called from 'CS::Framesmooth()'.
// ----------------------------------------------------------------------------  
{ static unsigned char peakMask[3][3] = {
                        { BACKGRND, BACKGRND, BACKGRND },
                        { BACKGRND, FOREGRND, BACKGRND },
                        { BACKGRND, BACKGRND, BACKGRND }
  };
  static unsigned char holeMask[3][3] = {
                        { FOREGRND, FOREGRND, FOREGRND },
                        { FOREGRND, BACKGRND, FOREGRND },
                        { FOREGRND, FOREGRND, FOREGRND }
  };

  long stPixel = Width+1,
       endPixel = (long)Width * (Depth-1) - 1;
  long i;
  int result = FALSE;
    
  // remove spurious pixels
  for (i=stPixel; i<endPixel; i++) 
    if ( *(Cs+i) == FOREGRND)
        result |= processMask(i, peakMask, BACKGRND);

  // fill holes
  for (i=stPixel; i<endPixel; i++)
    if ( *(Cs+i) == BACKGRND)
        result |= processMask(i, holeMask, FOREGRND);

  return(result);
}

int CS::processMask (long center, unsigned char mask[][3], unsigned char pixVal)
// ----------------------------------------------------------------------------
// apply 2x3 and 3x2 subset combinations of 'mask' around the pixel at
// 'center'. When a match occurs, change the pixel at 'center' to 'pixVal'.
// Called from 'CS::smooth'.
// ----------------------------------------------------------------------------
{ register int j, i;
  unsigned char huge *csPtr=Cs+center;

  for (i=0; i<3; i++)
    for (j=0; j<2; j++)
        if ( *(csPtr+NeighMask[i*3 + j]) != mask[i][j])
            goto next1;

  *(csPtr) = pixVal;
  return(TRUE);

next1:
  for (i=1; i<3; i++)
    for (j=0; j<3; j++)
        if ( *(csPtr+NeighMask[i*3 + j]) != mask[i][j])
            goto next2;

  *(csPtr) = pixVal;
  return(TRUE);

next2:
  for (i=0; i<3; i++)
    for (j=1; j<3; j++)
        if ( *(csPtr+NeighMask[i*3 + j]) != mask[i][j])
            goto next3;

  *(csPtr) = pixVal;
  return(TRUE);

next3:
  for (i=0; i<2; i++)
    for (j=0; j<3; j++)
        if ( *(csPtr+NeighMask[i*3 + j]) != mask[i][j])
            return(FALSE);

  *(csPtr) = pixVal;
  return(TRUE);
}

int CS::components ()
// ----------------------------------------------------------------------------
// scans frame vertically from left to right, and extracts components
// as encountered. Components are stored in a linked list, and the one
// with the longest contour length is selected for CS.
// ----------------------------------------------------------------------------
{ int i,j;
  long index, maxcontLen=0;
  struct component *compo;
  int compoCount=0;
  char msg[30];

  // Smoothing stage removed flood from frame, but this stage needs it 
  // back again.
  Framefloodfill();

  for (i=0; i<Width; i++) {

    j=0;
    index = i;

    while(j < Depth) {

        if ( *(Cs+index) == FLOODed) {
            // proceed vertically
            j++;
            index += Width;
        }

        else if ( *(Cs+index) == FOREGRND) {

            if (extractCompo(index, compo) == RTN_ERROR)
                return(retVal=RTN_ERROR);

            compoCount++;

            if (CompoList == NULL)
                CompoList = compo;
            else {
                compo->next = CompoList;
                CompoList = compo;
            }

            if (compo->contourLength > maxcontLen) {
                maxcontLen = compo->contourLength;
                MainCompo = compo;
            }
        }

        else 
            // CONTOUR encountered.
            while ( *(Cs+index) != FLOODed) {
                // proceed vertically
                j++;
                index += Width;
            }
    } 

  }

  if (compoCount == 0) {
        appError = NO_COMPO;
        return(retVal=RTN_ERROR);
  }
  else {
        if (compoCount == 1)
            sprintf(msg, "%2d component", compoCount);
        else
            sprintf(msg, "%2d components", compoCount);

        MESSAGE.Show(msg, REFRESH);
        return(retVal=RTN_OK);
  }

}

int CS::extractCompo (long i, struct component *& item)
// ----------------------------------------------------------------------------
// extracts the component starting at pixel 'i' by contour following, fills
// the component structure and returns it in 'item'. 
// ----------------------------------------------------------------------------
{ int direction=NORTH, found, k, m, minX, maxX, minY, maxY, xVal, yVal;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long n, j, contLen;

  *(Cs+i) = CONTOUR;

  contLen = 1;

#pragma warn -sig
  minX = maxX = i % Width;
  minY = maxY = i / Width;
#pragma warn +sig

  j = i;

  do {

    direction = (8 + direction - 2) % 8;  // initially add +90 deg.
    found = FALSE;

    for (k=0; k<8  &&  found == FALSE; k++) {
        n = j + neighOffset[direction];
        if ( *(Cs+n) == FOREGRND  ||  *(Cs+n) == CONTOUR)
            for (m=0; m<8  &&  found == FALSE; m += 2)
                if ( *(Cs+n+neighOffset[m]) == FLOODed)
                    found = TRUE;

        if (found == FALSE)
            direction = (8 + direction + 1) % 8;  // add -45 deg.
    }

    if (found) {

        j = n;
        *(Cs+j) = CONTOUR;
//      Show(16);
//      (void) getch();
        contLen++;

#pragma warn -sig
        xVal = j % Width;
        yVal = j / Width;
#pragma warn +sig

        if      (xVal < minX)    minX = xVal;
        else if (xVal > maxX)    maxX = xVal;

        if      (yVal < minY)    minY = yVal;
        else if (yVal > maxY)    maxY = yVal;
    }

    else {
        appError = NEXT_CONTOUR3;
        return(retVal=RTN_ERROR);
    }

  } while (j != i);


  if ((item = (struct component *) HEAP.alloc (sizeof(struct component)) ) == NULL) {
    appError = COMPO_ALLOC;
    return(retVal=RTN_ERROR);
  }

  item->startPixel = i;
  item->contourLength = contLen;
  item->minX = minX;
  item->maxX = maxX;
  item->minY = minY;
  item->maxY = maxY;
  item->next = NULL;
          
  return(retVal=RTN_OK);
}


int CS::stepThin ()
//
//
{ int direction=NORTH, found, k, m, pass=0, removed, ignoreThis,
      lastRemoved, setStartPixel=FALSE;
  int neighOffset[9] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1, -Width 
  };
  long n, j, i=(long)Width * Depth;
  struct lpCenter *loop;

  // restore CONTOUR values back to FOREGRND 
  j=-1;
  while (++j < i)
    if ( *(Cs+j) == CONTOUR)    *(Cs+j) = FOREGRND;

  // now there are FOREGRND, BACKGRND and FLOODed pixels in frame.

  i = j = MainCompo->startPixel;
  Show(16);
  displayStep(i, CYAN);
  displayStep(i, BLACK);
  while (pass < 2) {

    removed = FALSE;
    do {

        ignoreThis = FALSE;
        direction = (8 + direction - 2) % 8;  // initially add +90 deg.
        found = FALSE;

        for (k=0; k<8  &&  found == FALSE; k++) {
            n = j + neighOffset[direction];
            if ( *(Cs+n) == FOREGRND)
                for (m=0; m<8  &&  found == FALSE; m += 2)
                    if ( *(Cs+n+neighOffset[m]) == FLOODed)
                        found = TRUE;

            if (found == FALSE)
                direction = (8 + direction + 1) % 8;  // add -45 deg.
        }

        if (found) {
            // next contour found
            j = n;
            displayStep(j, CYAN);
            if (setStartPixel) {

                MainCompo->startPixel = i = j;
                setStartPixel = FALSE;
                ignoreThis = TRUE;
                displayStep(j, BLACK);
            }
            else {
                removed |= (lastRemoved = processContour(j, neighOffset, FLOODed));
                if (lastRemoved)    displayStep(j, LIGHTMAGENTA);
                else                displayStep(j, BLACK);      
            }

        }
        else {
            appError = NEXT_CONTOUR4;
            return(retVal=RTN_ERROR);
        }

    } while (j != i  ||  ignoreThis);

    if (removed) {
        pass = 0;
        if (lastRemoved)
            setStartPixel = TRUE;
    }
    else
        pass++;


    loop = LoopCenter;
    while (loop != NULL) {

        if (loop->removed) {

            int direction=NORTH;
            long i, j, n; 

            // go left from loop center until a FOREGRND(loop contour)
            // is encountered.
            i = loop->position;
            while ( *(Cs+i) == BACKGRND)    i--;

            j = i;
            displayStep(i, CYAN);
            displayStep(i, BLACK);

            removed = FALSE;

            do {

                direction = (8 + direction + 2) % 8;  // initially add -90 deg.
                found = FALSE;

                for (k=0; k<8  &&  found == FALSE; k++) {
                    n = j + neighOffset[direction];
                    if ( *(Cs+n) == FOREGRND)
                        for (m=0; m<8  &&  found == FALSE; m += 2)
                            if ( *(Cs+n+neighOffset[m]) == BACKGRND)
                                found = TRUE;

                    if (found == FALSE)
                        direction = (8 + direction - 1) % 8;  // add +45 deg.
                }

                if (found) {
                    // next loop contour found
                    j = n;
                    displayStep(j, CYAN);
                    removed |= (lastRemoved=processLoopContour(j, neighOffset, BACKGRND));
                    if (lastRemoved)    displayStep(j, WHITE);
                    else                displayStep(j, BLACK);  

                }
                else {
                    appError = NEXT_CONTOUR4;
                    return(retVal=RTN_ERROR);
                }

            } while (j != i);

            loop->removed = removed;
        }

        loop = loop->next;
    }

  }

  return(retVal=RTN_OK);
}

int CS::thin ()
//
//
{ int direction=NORTH, found, k, m, pass=0, removed, ignoreThis,
      lastRemoved, setStartPixel=FALSE;
  int neighOffset[9] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1, -Width 
  };
  long n, j, i=(long)Width * Depth;
  struct lpCenter *loop;

  // restore CONTOUR values back to FOREGRND 
  j=-1;
  while (++j < i)
    if ( *(Cs+j) == CONTOUR)    *(Cs+j) = FOREGRND;

  // now there are FOREGRND, BACKGRND and FLOODed pixels in frame.

  i = j = MainCompo->startPixel;

  while (pass < 2) {

    removed = FALSE;
    do {

        ignoreThis = FALSE;
        direction = (8 + direction - 2) % 8;  // initially add +90 deg.
        found = FALSE;

        for (k=0; k<8  &&  found == FALSE; k++) {
            n = j + neighOffset[direction];
            if ( *(Cs+n) == FOREGRND)
                for (m=0; m<8  &&  found == FALSE; m += 2)
                    if ( *(Cs+n+neighOffset[m]) == FLOODed)
                        found = TRUE;

            if (found == FALSE)
                direction = (8 + direction + 1) % 8;  // add -45 deg.
        }

        if (found) {
            // next contour found
            j = n;
            if (setStartPixel) {

                MainCompo->startPixel = i = j;
                setStartPixel = FALSE;
                ignoreThis = TRUE;
            }
            else 
                removed |= (lastRemoved = processContour(j, neighOffset, FLOODed));

        }
        else {
            appError = NEXT_CONTOUR4;
            return(retVal=RTN_ERROR);
        }

    } while (j != i  ||  ignoreThis);

    if (removed) {
        pass = 0;
        if (lastRemoved)
            setStartPixel = TRUE;
    }
    else
        pass++;


    loop = LoopCenter;
    while (loop != NULL) {

        if (loop->removed) {

            int direction=NORTH;
            long i, j, n; 

            // go left from loop center until a FOREGRND(loop contour)
            // is encountered.
            i = loop->position;
            while ( *(Cs+i) == BACKGRND)    i--;

            j = i;
            removed = FALSE;

            do {

                direction = (8 + direction + 2) % 8;  // initially add -90 deg.
                found = FALSE;

                for (k=0; k<8  &&  found == FALSE; k++) {
                    n = j + neighOffset[direction];
                    if ( *(Cs+n) == FOREGRND)
                        for (m=0; m<8  &&  found == FALSE; m += 2)
                            if ( *(Cs+n+neighOffset[m]) == BACKGRND)
                                found = TRUE;

                    if (found == FALSE)
                        direction = (8 + direction - 1) % 8;  // add +45 deg.
                }

                if (found) {
                    // next loop contour found
                    j = n;
                    removed |= processLoopContour(j, neighOffset, BACKGRND);
                }

                else {
                    appError = NEXT_CONTOUR4;
                    return(retVal=RTN_ERROR);
                }

            } while (j != i);

            loop->removed = removed;
        }

        loop = loop->next;
    }

  }

  return(retVal=RTN_OK);
}

int CS::processContour (long i, int neighOffset[], unsigned char pixVal)
//
//
{ int k, F_to_B=0, onB=FALSE, onF=FALSE, FCount=0;
  unsigned char huge *csPtr = Cs+i;

  for (k=NORTH; k<=N_WEST; k++)
        if ( *(csPtr+neighOffset[k]) == FOREGRND)    FCount++;

  if (FCount == 1)    return(FALSE);

  k = NORTH;

  if ( *(csPtr+neighOffset[k]) == FOREGRND) {
        onF = TRUE;
        k += 2;
  }
  else {
        onB = TRUE;
        k++;
  }

  while (k < 9) {

    if ( *(csPtr+neighOffset[k]) == FOREGRND) {

        if (onB) {
                onB = FALSE;
                onF = TRUE;
        }

        if ((k % 2) == 0)     k++;
    }

    else if (onF) {
                F_to_B++;
                onB = TRUE;
                onF = FALSE;
    }

    k++;
  }

  if (F_to_B > 1)
        return(FALSE);

  else {
        *(Cs+i) = pixVal;
        return(TRUE);
  }

}

int CS::processLoopContour (long i, int neighOffset[], unsigned char pixVal)
//
//
{ int k, F_to_B=0, onB=FALSE, onF=FALSE;
  unsigned char huge *csPtr = Cs+i;

  k = NORTH;

  if ( *(csPtr+neighOffset[k]) == FOREGRND  ||
       *(csPtr+neighOffset[k]) == LPCONTOUR ) {
        onF = TRUE;
        k += 2;
  }
  else {
        onB = TRUE;
        k++;
  }

  while (k < 9) {

    if ( *(csPtr+neighOffset[k]) == FOREGRND  ||
         *(csPtr+neighOffset[k]) == LPCONTOUR ) {

        if (onB) {
                onB = FALSE;
                onF = TRUE;
        }

        if ((k % 2) == 0)     k++;
    }

    else if (onF) {
                F_to_B++;
                onB = TRUE;
                onF = FALSE;
    }

    k++;
  }

  if (F_to_B > 1)
        return(FALSE);

  else {
        *(Cs+i) = pixVal;
        return(TRUE);
  }

}

int CS::setLoops ()
//
//
{ int direction, found, twinExist, k, twinCount=0;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1
  };
  long i, j; 
  struct lpCenter *loop;
  char msg[30];

  loop = LoopCenter;

  while (loop != NULL) {

    direction = NORTH;

    // go left from loop center until a FOREGRND(loop contour)
    // is encountered.
    i = loop->position;
    while ( *(Cs+i) == BACKGRND)    i--;

//    displayStep(i, CYAN);

    j = i;
    twinExist = FALSE;

    do {

        direction = (8 + direction + 2) % 8;  // initially add -90 deg.
        found = FALSE;

        for (k=0; k<8  &&  found == FALSE; k++) 
            if ( (*(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
                found = TRUE;
            else
                direction = (8 + direction - 1) % 8;  // add +45 deg.

        if (found) {
            // next loop contour found
            j = j + neighOffset[direction];

            if ( (*(Cs+j) & loopPixel) == loopPixel) {
                // this is connected to another loop (twin loops).
                // This situation occurs in letter 'he'.
                *(Cs+j) |= twinLoopPixel;
                twinExist = TRUE;
            }
            else    // {
                *(Cs+j) |= loopPixel;
                    // displayStep(j, DARKGRAY);
                    // }
        }

        else {
            appError = NEXT_LPIXEL;
            return(retVal=RTN_ERROR);
        }

    } while (j != i);

    if (twinExist) {

        struct lpCenter *lpPair;
        int twinFound=FALSE;
        // this loop is part of a 'he' letter. So find the other part
        // scanning the loops linked list from start.

        lpPair = LoopCenter;

        while (lpPair != loop  &&  twinFound == FALSE) {

            if (lpPair->twin != NULL) {
                // this loop already has a twin, so skip it.
                lpPair = lpPair->next;
                continue;
            }

            direction = NORTH;
            // go left from loop center until a FOREGRND(loop contour)
            // is encountered.
            i = loop->position;
            while ( *(Cs+i) == BACKGRND)    i--;

            j = i;

            do {

                direction = (8 + direction + 2) % 8;  // initially add -90 deg.
                found = FALSE;

                for (k=0; k<8  &&  found == FALSE; k++) 
                    if ( (*(Cs+j+neighOffset[direction]) & loopPixel) == loopPixel)
                        found = TRUE;
                    else
                        direction = (8 + direction - 1) % 8;  // add +45 deg.

                if (found) {
                    // next loop contour found
                    j = j + neighOffset[direction];

                    if ( (*(Cs+j) & twinLoopPixel) == twinLoopPixel) {
                        // twin found.
                        loop->twin = lpPair;
                        lpPair->twin = loop;
                        twinFound = TRUE;
                        twinCount++;
                    }

                }

                else {
                    appError = NEXT_LPIXEL;
                    return(retVal=RTN_ERROR);
                }

            } while (j != i  &&  twinFound == FALSE);

            lpPair = lpPair->next;
        }

        if (twinFound == FALSE) {
            appError = TWIN_LOOP;
            return(retVal=RTN_ERROR);
        }

    }

    loop = loop->next;
  }

  if (twinCount > 0) {
      sprintf(msg, ", %2d loop twins", twinCount);
      MESSAGE.Show(msg, CONCAT);
  }

  return(retVal=RTN_OK);
}


void CS::keyFeaturePoints ()
// ----------------------------------------------------------------------------
// marks KF points in CS. Valid key feature points are E (EndPoint),
// B (BrachPoint) and C (CrossPoint).
// In order to find KF points, the number of FOREGRND-to-BACKGRND transitions
// around (8-neighbours) each FOREGRND pixel is counted. 
//
//                count        KF
//                  1          E
//                  3          B
//                  4          C
// ----------------------------------------------------------------------------
{ int k, onF, onB, F_to_B;
  long j;
  long matrixLen = (long)Width * Depth;
  unsigned char huge *csPtr = Cs;
  int neighOffset[9] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1, -Width
  };

  j = -1;
  csPtr--;

  while (++j < matrixLen) {

    csPtr++;

    if ((*csPtr | foreGrnd) == foreGrnd) {

        k = NORTH;
        onF = onB = FALSE;
        F_to_B = 0;

        if (( *(csPtr+neighOffset[k]) | foreGrnd) == foreGrnd) 
                onF = TRUE;
        else 
                onB = TRUE;

        while (++k < 9) 

            if (( *(csPtr+neighOffset[k]) | foreGrnd) == foreGrnd) {

                if (onB) {
                        onB = FALSE;
                        onF = TRUE;
                }
            }

            else if (onF) {

                F_to_B++;
                onB = TRUE;
                onF = FALSE;
            }


        if (F_to_B == 2)    continue;

        else if (F_to_B == 1)   *csPtr |= EndPoint;
        else if (F_to_B == 3)   *csPtr |= BranchPoint;
        else if (F_to_B == 4)   *csPtr |= CrossPoint;

    }

  }

}

