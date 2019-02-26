/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#include <stdio.h>
#include <conio.h>
#include "otto.h"
#include "classdef.h"
#include "cs.h"

extern int appError;
extern int retVal;
extern Message MESSAGE;
extern Heap HEAP;

extern void beep (void);

int CS::findMFSg ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  determineSegments();

  if (retVal != RTN_OK)      return(retVal);

  markMFSgPixels();

  if (retVal != RTN_OK)      return(retVal);
  else                       return(retVal=RTN_OK);

}


int CS::determineSegments ()
// ----------------------------------------------------------------------------
// this fcn. creates the list of segments in CS, and does some house-keeping
// operations with the loops.
// The term 'segment' in this fcn. and elsewhere corresponds to 'Character
// Segment'.
// ----------------------------------------------------------------------------
{ int direction, found, loopCount=0, segCount=0, twinCount;
  int i, k, endX, staX, lpX;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long jntPos, j;
  struct CutMark *tmpCut=MinList;
  struct lpCenter *lpTmp, *firstFoundLoop;
  struct Segment *segTmp;

  // Remember that 'MinList', the head of cutMarks, points to the last cutMark 
  // on CS. The linked list of segments that will be created in this function 
  // will have the opposite order, i.e. the head of the list (lSegs) 
  // will point to the first letter segment in CS.

  lSegs = NULL;

  do {

    if ((segTmp = (struct Segment *) HEAP.alloc(sizeof(struct Segment))) == NULL) {
      appError = SEG_ALLOC;
      return(retVal=RTN_ERROR);
    }

    segCount++;  // number of segments in CS.

    if (tmpCut == MinList)
        segTmp->endCutMark = MainCompo->startPixel;
    else
        segTmp->endCutMark = jntPos;

    if (tmpCut == NULL) {
        // there is no cut-mark, that is, single segment.
        segTmp->stCutMark = rightmostEP;
        direction = segTmp->CFdirection = SOUTH;
    }
    else {
        segTmp->stCutMark = jntPos = tmpCut->position;
        direction = segTmp->CFdirection = tmpCut->CFdirection;
    }


    j = segTmp->stCutMark;

    do {
        
        if ( (*(Cs+j) & loopPixel) == loopPixel) {
            segTmp->loopExist = TRUE;
            loopCount++;
            break;
        }
        else if (j == segTmp->endCutMark) {
            segTmp->loopExist = FALSE;
            break;
        }

        direction = (8 + direction - 2) % 8;  // initially add +90 deg.
        found = FALSE;

        for (k=0; found == FALSE  &&  k<8; k++)
            if ( (*(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
                found = TRUE;
            else
                direction = (8 + direction + 1) % 8;  // add -45 deg.

        if (found)
            j += neighOffset[direction];
        else {
            appError = NEXT_POINT8;
            return(retVal=RTN_ERROR);
        }

    } while (TRUE);

    segTmp->mfsg = NULL;

    if (lSegs == NULL) {
        segTmp->next = NULL;
        lSegs = segTmp;
    }
    else {
        segTmp->next = lSegs;
        lSegs = segTmp;
    }

    if (tmpCut == NULL)
        break;
    else
        tmpCut = tmpCut->next;

  } while (TRUE);

  // When the letters 'vav, fe, kaf' appear at the end of a CS, the above
  // procedure will not be able to detect their loops. We have to check
  // this condition now.
  //
  // Sometimes letter 'he' (middle) does not produce loop twins, and appear
  // as what we call 'loop pairs'. This is the right time to perform this 
  // check. Then follows the operation we have mentioned in the previous paragraph.

  if (segCount > 2) {      // there is no sense in looking for loop pairs 
                           // in a CS of less than 3 segments.

    segTmp = lSegs->next;
    while (segTmp->next != NULL) {    // as long as the last segment is not reached

        if (segTmp->loopExist == TRUE) {

            lpTmp = LoopCenter;
            found = FALSE;
#pragma warn -sig
            endX = segTmp->endCutMark % Width;
            staX = segTmp->stCutMark  % Width;

            while(lpTmp != NULL) {
                
                lpX = lpTmp->position % Width;
#pragma warn +sig
                if (lpX < staX  &&  lpX > endX)

                    if (found == FALSE)
                        if (lpTmp->twin == NULL) {
                            found = TRUE;
                            firstFoundLoop = lpTmp;
                        }
                        else 
                            // this loop already has a twin.
                            break;
                    else {
                        // loop pair found !
                        lpTmp->twin = firstFoundLoop;
                        firstFoundLoop->twin = lpTmp;
                        break;
                    }

                lpTmp = lpTmp->next;
            }
        }

        segTmp = segTmp->next;
    }

  }

  // check for letters 'vav, kaf, fe' at end (the reason is explained above).
  // The existence can be detected by dangling loops, as follows:
  lpTmp = LoopCenter;
  i = twinCount = 0;
  while (lpTmp != NULL) {

    i++;
    if (lpTmp->twin != NULL)
        twinCount++;
    lpTmp = lpTmp->next;
  }

  i -= twinCount / 2;

  // variable 'loopCount' does not reflect the actual number of loops in CS.
  // It is equal to the number of loops visited during segment processing (implemented 
  // above in this function). Therefore variable 'i' has been calculated accordingly.

  if (loopCount < i) {
    // dangling loop found.

    // Reach to the last segment, since this dangling loop is assumed to belong to it.
    segTmp = lSegs;
    while (segTmp->next != NULL)    segTmp = segTmp->next;

    if (segTmp->loopExist == FALSE) 
        segTmp->loopExist = TRUE;
    else {
        appError = DANG_LOOP;
        return(retVal=RTN_ERROR);
    }

  }

  return(retVal=RTN_OK);
}


int CS::markMFSgPixels ()
// ----------------------------------------------------------------------------
// This is the main function that performs feature extraction according to 
// what is called as 'Main Feature Segment'. The appropriate function depending
// on the position (beginning, middle, etc) of character is called, because
// there are several differences in details of the algorithm for every position. 
// ----------------------------------------------------------------------------
{
  struct Segment *segTmp;

  if (lSegs->next == NULL)
    // there is only one segment
    return(retVal = extractSINGLEletterMFSg());

  else {

    // there are several segments
    if ((retVal=extractBEGINNINGletterMFSg(lSegs))  != RTN_OK)
        return(retVal);
//    Show(16); (void) getch();
    segTmp = lSegs->next;
    while (segTmp->next != NULL) {   // as long as the last segment is not reached

        if ((retVal=extractMIDDLEletterMFSg(segTmp)) != RTN_OK)
            return(retVal);
//      Show(16); (void) getch();
        segTmp = segTmp->next;
    }

    return(retVal = extractENDletterMFSg(segTmp));
  }

}


int CS::extractENDletterMFSg (struct Segment *sgm)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int k, direction, found, jntFound, jntDir, c_wiseDir, Cc_wiseDir;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long j, jntIndex, prevPos;
  unsigned char jntCandid, stPixelSave;


  if (sgm->loopExist == TRUE)
    return(retVal=RTN_OK);

  direction = sgm->CFdirection;
  j = sgm->stCutMark;

  do {

    direction = (8 + direction - 2) % 8;  // initially add +90 deg.
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction + 1) % 8;  // add -45 deg.

    if (found) {
      
      prevPos = j;

      j += neighOffset[direction];

      if (( *(Cs+j) & EndPoint) == EndPoint) 
          return(retVal = extract_all_pixel_MFSg(sgm));
      
      jntFound = FALSE;

      if (direction % 2  ==  1) {

          jntDir = (8 + direction + 1) % 8;
          jntCandid = *(Cs+prevPos+neighOffset[jntDir]);

          if ((jntCandid & BranchPoint) == BranchPoint  ||
              (jntCandid & CrossPoint ) == CrossPoint) {

              jntFound = TRUE;
              jntIndex = prevPos + neighOffset[jntDir];
          }
      }

      if ((jntFound == FALSE) &&
          (( *(Cs+j) & BranchPoint) == BranchPoint  ||
           ( *(Cs+j) & CrossPoint)  == CrossPoint ) ) {

          jntFound = TRUE;
          jntIndex = j;
      }
                  
      if (jntFound) {
          // Remove the reaching branch to this B (or C).
          // Then convert it into an M (MidPoint).
          stPixelSave = *(Cs+sgm->stCutMark);
          if ((retVal=removeReachingBranch(sgm, prevPos)) != RTN_OK)
                return(retVal);

          *(Cs+jntIndex) &= MidPoint;

          // obtain necessary initial directions for both clockwise and
          // counter-clockwise skeleton tracing in CS::END_SINGLE_MFSgPixels().

          if (jntIndex == j) 
              Cc_wiseDir = c_wiseDir = (direction + 4) % 8;  // add +180 deg.
          else 
              Cc_wiseDir = c_wiseDir = (jntDir + 4) % 8;  // add +180 deg.

          c_wiseDir  = (8 + c_wiseDir + 2) % 8;     // add -90 deg.
          Cc_wiseDir = (8 + Cc_wiseDir - 2) % 8;    // add +90 deg.

          retVal = END_SINGLE_MFSgPixels(jntIndex, c_wiseDir, Cc_wiseDir, sgm);

          if (retVal != RTN_OK)    return(retVal);

          // restore the value of pixel at 'stCutMark', since it was destroyed
          // at CS::removeReachingBranch();
          *(Cs+sgm->stCutMark) = stPixelSave;

          return(retVal=RTN_OK);
      }

    }

    else {
        appError = NEXT_POINT15;
        return(retVal=RTN_ERROR);
    }

  } while (TRUE);

}

int CS::extract_all_pixel_MFSg (struct Segment *sgm)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int k, direction, found;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long j;


  direction = sgm->CFdirection;
  j = sgm->stCutMark;

  *(Cs+j) |= MFSgPixel;
  sgm->stFeature = j;

  do {

    direction = (8 + direction - 2) % 8;  // initially add +90 deg.
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction + 1) % 8;  // add -45 deg.


    if (found) {
      
      j += neighOffset[direction];
      *(Cs+j) |= MFSgPixel;

      if (( *(Cs+j) & EndPoint) == EndPoint) {
          sgm->endFeature = j;
          return(retVal=RTN_OK);
      }

    }

    else {
        appError = NEXT_POINT10;
        return(retVal=RTN_ERROR);
    }

  } while (TRUE);

}


int CS::removeReachingBranch (struct Segment *sgm, long i)
// ----------------------------------------------------------------------------
// Removes the reaching branch of segment 'sgm' until (and including) point
// at position 'i' is reached. Remove operation is accomplished by assigning
// background value. Background is currently in FLOODed form.
// ----------------------------------------------------------------------------
{ int k, found, direction;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long j;

  direction = sgm->CFdirection;
  j = sgm->stCutMark;
  *(Cs+j) = FLOODed;

  do {

    if (j == i)     return(retVal=RTN_OK);

    direction = (8 + direction - 2) % 8;  // initially add +90 deg.
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction + 1) % 8;  // add -45 deg.


    if (found) {
      j += neighOffset[direction];
      *(Cs+j) = FLOODed;
    }

    else {
        appError = NEXT_POINT11;
        return(retVal=RTN_ERROR);
    }

  } while (TRUE);

}

int CS::END_SINGLE_MFSgPixels (long i, int c_wiseDir, int Cc_wiseDir, struct Segment *sgm)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int k, direction, found, jntFound, Cc_wiseComplete, initAngle, stepAngle;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  int end1, end2, len1, len2, jntDir, prevDir;
  long j, jntIndex=0, prevPos, idxCandid;
  unsigned char jntCandid;

  j = i;

//  if (( *(Cs+j) | foreGrnd) == foreGrnd)
      *(Cs+j) |= MFSgPixel;

  Cc_wiseComplete = FALSE;
  initAngle = 2;      // initially perform counter-clockwise skeleton following
  stepAngle = -1;
  direction = Cc_wiseDir;

    
  do {
    
    if (( *(Cs+j) & EndPoint) == EndPoint)

        if (Cc_wiseComplete == FALSE) {
            sgm->stFeature = j;
            Cc_wiseComplete = TRUE;
            initAngle = -2;
            stepAngle = 1;
            direction = c_wiseDir;
            j = i;
        }

        else {
            sgm->endFeature = j;
            return(retVal=RTN_OK);
        }

    prevDir = direction;
    direction = (8 + direction + initAngle) % 8;  
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if (( *(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction + stepAngle) % 8; 

    if (found) {
      prevPos = j;

      j += neighOffset[direction];


      jntFound = FALSE;

      if (direction % 2  ==  1) {

          jntDir = (8 + direction + stepAngle) % 8;
          idxCandid = prevPos + neighOffset[jntDir];
          jntCandid = *(Cs+idxCandid);

          if ((idxCandid != jntIndex) && 
              ((jntCandid & BranchPoint) == BranchPoint  ||
               (jntCandid & CrossPoint ) == CrossPoint) ) {

              jntFound = TRUE;
              jntIndex = idxCandid;
              Pivot = prevPos;
          }
      }

      if ((j != jntIndex) && (jntFound == FALSE) &&
          (( *(Cs+j) & BranchPoint) == BranchPoint  ||
           ( *(Cs+j) & CrossPoint)  == CrossPoint) ) {

          jntFound = TRUE;
          jntIndex = j;
          Pivot = prevPos;
      }



      if (jntFound) {
        // look ahead from the two branches, and select one with the following
        // priority :
        //  - branch ends with a B.
        //  - branch ends with an E.
        // In case a tie occurs, select the longest branch.

        end1 = branchLookAhead(initAngle, stepAngle, prevDir, jntIndex, len1);
        if (retVal != RTN_OK)
            return(retVal);

        Pivot = prevPos;
        end2 = branchLookAhead(-initAngle, -stepAngle, prevDir, jntIndex, len2);
        if (retVal != RTN_OK)
            return(retVal);

        if (end1 == BRN_P) {

            if (end2 == BRN_P) {
                if (len1 < len2) {
                    initAngle = -initAngle;
                    stepAngle = -stepAngle;
                    j = prevPos;
                    direction = prevDir;
                    continue;
                }
            }
        }

        else if (end2 == BRN_P) {

            initAngle = -initAngle;
            stepAngle = -stepAngle;
            j = prevPos;
            direction = prevDir;
            continue;
        }

        else  // both are END_P
            if (len1 < len2) {
                initAngle = -initAngle;
                stepAngle = -stepAngle;
                j = prevPos;
                direction = prevDir;
                continue;
            }

        *(Cs+j) |= MFSgPixel;
      }

      else
          *(Cs+j) |= MFSgPixel;

    }

    else {
        appError = NEXT_POINT12;
        return(retVal=RTN_ERROR);
    }

  } while (TRUE);

}


int CS::branchLookAhead(int initAngle, int stepAngle, int direction, long jIdx,
                        int& len) 
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int k, found, jntFound, jntDir;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long j, prevPos, idxCandid;
  unsigned char jntCandid;

  j = Pivot;
  len = 0;

  do {

    direction = (8 + direction + initAngle) % 8;  
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if (( *(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction + stepAngle) % 8; 

    if (found) {
      prevPos = j;

      j += neighOffset[direction];
      len++;

      if (( *(Cs+j) & EndPoint) == EndPoint)
          return(END_P);

      jntFound = FALSE;

      if (direction % 2  ==  1) {

          jntDir = (8 + direction + stepAngle) % 8;
          idxCandid = prevPos + neighOffset[jntDir];
          jntCandid = *(Cs+idxCandid);

          if ((idxCandid != jIdx) && 
              ((jntCandid & BranchPoint) == BranchPoint  ||
               (jntCandid & CrossPoint ) == CrossPoint) ) 

              jntFound = TRUE;

      }

      if ((j != jIdx) && (jntFound == FALSE) &&
          (( *(Cs+j) & BranchPoint) == BranchPoint  ||
           ( *(Cs+j) & CrossPoint)  == CrossPoint) ) 

          jntFound = TRUE;


      if (jntFound) 
          return(BRN_P);

    }

    else {
        appError = NEXT_POINT13;
        return(retVal=RTN_ERROR);
    }

  } while (TRUE);

}

int CS::branchLookAhead(int initAngle, int stepAngle, int direction, long jIdx,
                        int& len, long endCutMark) 
// ----------------------------------------------------------------------------
// overloaded fcn. of 'branchLookAhead'. This one is called from 
// CS::extractBEGINNINGletterMFSg().
// ----------------------------------------------------------------------------
{ int k, found, jntFound, jntDir;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long j, prevPos, idxCandid;
  unsigned char jntCandid;

  j = Pivot;
  len = 0;

  do {
    
    direction = (8 + direction + initAngle) % 8;  
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if (( *(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction + stepAngle) % 8; 

    if (found) {
      prevPos = j;

      j += neighOffset[direction];
      len++;

      if (( *(Cs+j) & EndPoint) == EndPoint)
          return(END_P);

      if (j == endCutMark)  // these two statements are the only different part
          return(END_CUT);  // from the other overloaded fcn.

      jntFound = FALSE;

      if (direction % 2  ==  1) {

          jntDir = (8 + direction + stepAngle) % 8;
          idxCandid = prevPos + neighOffset[jntDir];
          jntCandid = *(Cs+idxCandid);

          if ((idxCandid != jIdx) && 
              ((jntCandid & BranchPoint) == BranchPoint  ||
               (jntCandid & CrossPoint ) == CrossPoint) ) 

              jntFound = TRUE;

      }

      if ((j != jIdx) && (jntFound == FALSE) &&
          (( *(Cs+j) & BranchPoint) == BranchPoint  ||
           ( *(Cs+j) & CrossPoint)  == CrossPoint) ) 

          jntFound = TRUE;


      if (jntFound) 
          return(BRN_P);

    }

    else {
        appError = NEXT_POINT13;
        return(retVal=RTN_ERROR);
    }

  } while (TRUE);

}


int CS::extractSINGLEletterMFSg ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  return( extractENDletterMFSg(lSegs) );
}



int CS::extractMIDDLEletterMFSg (struct Segment *sgm)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int k, direction, found, jntFound, end, len, prevDir, jntDir;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long j, jntIndex=0, prevPos, idxCandid, oldstCutMark;
  unsigned char jntCandid, stPixelSave;


  if (sgm->loopExist == TRUE)
    return(retVal=RTN_OK);

  direction = sgm->CFdirection;
  j = sgm->stCutMark;

  do {

    prevDir = direction;
    direction = (8 + direction - 2) % 8;  // initially add +90 deg.
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction + 1) % 8;  // add -45 deg.


    if (found) {
      prevPos = j;

      j += neighOffset[direction];

      if (j == sgm->endCutMark)
          return(retVal = extractBEGINNINGletterMFSg(sgm));

      jntFound = FALSE;

      if (direction % 2  ==  1) {

          jntDir = (8 + direction + 1) % 8;
          idxCandid = prevPos + neighOffset[jntDir];
          jntCandid = *(Cs+idxCandid);

          if ((idxCandid != jntIndex) && 
              ((jntCandid & BranchPoint) == BranchPoint  ||
               (jntCandid & CrossPoint ) == CrossPoint) ) {

              jntFound = TRUE;
              jntIndex = idxCandid;
              Pivot = prevPos;
          }
      }

      if ((j != jntIndex) && (jntFound == FALSE) &&
          (( *(Cs+j) & BranchPoint) == BranchPoint  ||
           ( *(Cs+j) & CrossPoint)  == CrossPoint) ) {

          jntFound = TRUE;
          jntIndex = j;
          Pivot = prevPos;
      }


      if (jntFound) {
            // Look ahead with clockwise skeleton following.

            end = branchLookAhead(-2, 1, prevDir, jntIndex, len, sgm->endCutMark);
            if (retVal != RTN_OK)
                return(retVal);

            if (end == END_P) {

                stPixelSave = *(Cs+sgm->stCutMark);
                if ((retVal=removeReachingBranch(sgm, prevPos)) != RTN_OK)
                    return(retVal);

                *(Cs+jntIndex)  &= MidPoint;

                // save 'stCutMark' and assign former(!) joint 'jntIndex' to it.
                // Then extract as in beginning letter.
                oldstCutMark = sgm->stCutMark;
                sgm->stCutMark = jntIndex;
                sgm->CFdirection = prevDir;
                retVal = extractBEGINNINGletterMFSg(sgm);

                if (retVal != RTN_OK)    return(retVal);

                // restore the value of pixel at 'oldstCutMark', since it was destroyed
                // at CS::removeReachingBranch();
                *(Cs+oldstCutMark) = stPixelSave;

                return(retVal=RTN_OK);
            }

            else
                return(retVal = extractBEGINNINGletterMFSg(sgm));

      }

    }

    else {
        appError = NEXT_POINT9;
        return(retVal=RTN_ERROR);
    }

  } while (TRUE);

}


int CS::extractBEGINNINGletterMFSg (struct Segment *sgm)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int k, direction, found, jntFound, initAngle, stepAngle;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  int end1, end2, len1, len2, prevDir, jntDir;
  long j, jntIndex=0, prevPos, idxCandid;
  unsigned char jntCandid;


  if (sgm->loopExist == TRUE)
    return(retVal=RTN_OK);

  direction = sgm->CFdirection;
  j = sgm->stCutMark;
  initAngle = 2;    // initially perform counter-clockwise skeleton following
  stepAngle = -1;

  *(Cs+j) |= MFSgPixel;
  sgm->stFeature = j;

  do {
  
    if (j == sgm->endCutMark) {
        sgm->endFeature = j;
        return(retVal=RTN_OK);
    }

    prevDir = direction;
    direction = (8 + direction + initAngle) % 8;
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction + stepAngle) % 8; 

    if (found) {
      prevPos = j;

      j += neighOffset[direction];

      if ( (*(Cs+j) & EndPoint) == EndPoint) {
          *(Cs+j) |= MFSgPixel;
          sgm->endFeature = j;
          return(retVal=RTN_OK);
      }


      jntFound = FALSE;

      if (direction % 2  ==  1) {

          jntDir = (8 + direction + stepAngle) % 8;
          idxCandid = prevPos + neighOffset[jntDir];
          jntCandid = *(Cs+idxCandid);

          if ((idxCandid != jntIndex) && 
              ((jntCandid & BranchPoint) == BranchPoint  ||
               (jntCandid & CrossPoint ) == CrossPoint) ) {

              jntFound = TRUE;
              jntIndex = idxCandid;
              Pivot = prevPos;
          }
      }

      if ((j != jntIndex) && (jntFound == FALSE) &&
          (( *(Cs+j) & BranchPoint) == BranchPoint  ||
           ( *(Cs+j) & CrossPoint)  == CrossPoint) ) {

          jntFound = TRUE;
          jntIndex = j;
          Pivot = prevPos;
      }



      if (jntFound) {
        // look ahead from the two branches, and select one with the following
        // priority :
        //  - branch ends with a B.
        //  - branch ends with an E.
        //  - branch ends with 'endCutMark'.
        // In case a tie occurs, select the longest branch.

        end1 = branchLookAhead(initAngle, stepAngle, prevDir, jntIndex, len1, sgm->endCutMark);
        if (retVal != RTN_OK)
            return(retVal);

        Pivot = prevPos;
        end2 = branchLookAhead(-initAngle, -stepAngle, prevDir, jntIndex, len2, sgm->endCutMark);
        if (retVal != RTN_OK)
            return(retVal);

        if (end1 == BRN_P) {

            if (end2 == BRN_P) {
                if (len1 < len2) {
                    initAngle = -initAngle;
                    stepAngle = -stepAngle;
                    j = prevPos;
                    direction = prevDir;
                    continue;
                }
            }
        }

        else if (end2 == BRN_P) {

            initAngle = -initAngle;
            stepAngle = -stepAngle;
            j = prevPos;
            direction = prevDir;
            continue;
        }

        else if (end1 == END_P) {

            if (end2 == END_P) {
                if (len1 < len2) {
                    initAngle = -initAngle;
                    stepAngle = -stepAngle;
                    j = prevPos;
                    direction = prevDir;
                    continue;
                }
            }
        }

        else if (end2 == END_P) {

            initAngle = -initAngle;
            stepAngle = -stepAngle;
            j = prevPos;
            direction = prevDir;
            continue;
        }

        else if (len1 < len2) {
            initAngle = -initAngle;
            stepAngle = -stepAngle;
            j = prevPos;
            direction = prevDir;
            continue;
        }


        *(Cs+j) |= MFSgPixel;
      }

      else 
        *(Cs+j) |= MFSgPixel;


    }

    else {
        appError = NEXT_POINT14;
        return(retVal=RTN_ERROR);
    }

  } while (TRUE);

}
