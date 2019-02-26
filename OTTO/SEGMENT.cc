/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#include <stdio.h>
#include <math.h>
#include "otto.h"
#include "classdef.h"
#include "cs.h"

enum COLORS {
    BLACK,                  /* dark colors */
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHTGRAY,
    DARKGRAY,               /* light colors */
    LIGHTBLUE,
    LIGHTGREEN,
    LIGHTCYAN,
    LIGHTRED,
    LIGHTMAGENTA,
    YELLOW,
    WHITE
};

extern int appError;
extern int retVal;
extern Message MESSAGE;
extern Heap HEAP;

int CS::findFarthestX (long& FarthestX)
// ----------------------------------------------------------------------------
// this fcn. finds the position of point in CS farthest on the X-axis.
// 'maxX' cannot be used, because CS is currently thinned and this value
// with the others(maxY, minY, minX) are not valid anymore.
// *** 30/11/93 ***
// This point will be used as right-to-left segmentation starting point.
// ----------------------------------------------------------------------------
{ int direction=SOUTH, found, k, maxX, xVal, pass=0;
  int neighOffset[8] = {
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1
  };
  long i, j;

  i = j = MainCompo->startPixel;
  maxX = 0;

  while (pass < 2) {

    do {

        direction = (8 + direction + 2) % 8;  // initially add -90 deg.
        found = FALSE;

        for (k=0; k<8  &&  found == FALSE; k++) 
            if ( (*(Cs+j+neighOffset[direction]) | foreGrnd) == foreGrnd)
                found = TRUE;
            else
                direction = (8 + direction - 1) % 8;  // add +45 deg.


        if (found) {

            j = j + neighOffset[direction];

#pragma warn -sig
            xVal = j % Width;
#pragma warn +sig

            if (j > MaxBaseLine  ||  j < MinBaseLine)
                // the pixel is not within the baseline interval
                continue;

            if (xVal > maxX) {
                maxX = xVal;
                FarthestX = j;
            }
        }

        else {

            appError = NEXT_POINT5;
            return(retVal=RTN_ERROR);
        }

    } while (j != i);

    pass++;
  }

  return(retVal=RTN_OK);
}


int CS::Min_Max_Partition ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int reason, minSkipped, singleYE=FALSE, kefANDye=FALSE;
  int lastMinX, lastMinY, endMaxX, endMaxY, minDir;
  float lastRatio, YEratio;
  long j, lastMin, matrixLen=(long)Width * Depth;
  char msg[30], prompt1[]=", lastRatio = %.2f",
                prompt2[]=", 'elif' found at end.",
                prompt3[]=", 'he' found at end.",
                prompt4[]=", this is letter 'HE'",
                prompt5[]=", 'ye' may exist at end",
                prompt6[]=", this letter may be 'ye'",
                prompt7[]=", YEratio = %.2f",
                prompt8[]=", this is 'KEF + YE'",
                prompt9[]=", this is 'LAMELIF'";
  struct CutMark *tmpMin;


  reason = findFirstMin(j, minDir);  _f_;

  if (reason == TERMINATE)

      // terminated before fixing a minimum, so there cannot be any 
      // 'jointVisited' attributes to clear out. Simply returns.
      return(retVal=RTN_OK);

  else if (reason == MIN_FOUND  &&  ( *(Cs+j) & loopPixel) != loopPixel) {

        if ((MinList = (struct CutMark *) HEAP.alloc (sizeof(struct CutMark)) ) == NULL) {
            appError = MIN_ALLOC;
            return(retVal=RTN_ERROR);
        }

        MinList->position = j;
        MinList->CFdirection = minDir;
        MinList->next = NULL;

        displayStep(j, LIGHTGREEN);

        minSkipped = FALSE;

        if (CheckYE)

            if (KEFandYE)
                kefANDye = TRUE;
//              genKEF_YEcutMark();
            else
                singleYE = TRUE;

  }

  else if (reason == MIN_IGNORE)

      minSkipped = TRUE;

  else if (reason == REGULAR_MIN) {

      minSkipped = FALSE;
      goto fixMin;
  }


  while (TRUE) {

      reason = findMax(j); _f_;

      if (reason == MAX_FOUND)
          displayStep(j, LIGHTMAGENTA);

      else if (reason == TERMINATE) {

          if (( *(Cs+j) & loopPixel) == loopPixel) {

              if (MinList == NULL)
                  MESSAGE.Show(prompt4, CONCAT);
              else
                  MESSAGE.Show(prompt3, CONCAT);

              break;
          }


          if (MinList != NULL) {

              // access last minimum
              lastMin = MinList->position;

#pragma warn -sig
              lastMinX = lastMin % Width;
              lastMinY = lastMin / Width;
              endMaxX = MainCompo->startPixel % Width;
              endMaxY = MainCompo->startPixel / Width;
#pragma warn +sig

              if (endMaxX == lastMinX)
                  lastRatio = lastMinY - endMaxY;
              else
                  lastRatio = (float)(lastMinY - endMaxY) / (lastMinX - endMaxX);

              sprintf(msg, prompt1, lastRatio);
              MESSAGE.Show(msg, CONCAT);

              if (lastRatio > 2.0) {

                  MESSAGE.Show(prompt2, CONCAT);

                  // 'kef' at beginning sets CheckYE to TRUE. If the last
                  // min. is not deleted, then CheckYE has to be cleared
                  // to prevent false alarm of 'ye' at end.

                  CheckYE = FALSE;
              }

              else {
                  // delete last min. from linked list

                  if (MinList != NULL  &&  minSkipped == FALSE) {
                      tmpMin = MinList->next;
                      HEAP.free(MinList);
                      MinList = tmpMin;
                  }
              }

          }


          if (CheckYE) {

              YEratio = (float)( (BF_STARTpoint % Width) - (MainCompo->startPixel % Width) )
                        / ((BF_STARTpoint % Width) - (FB_STARTpoint % Width));

              sprintf(msg, prompt7, YEratio);
              MESSAGE.Show(msg, CONCAT);

              if (YEratio < 4.0) 

                  if (singleYE)
                      MESSAGE.Show(prompt6, CONCAT);

                  else if (kefANDye)
                      MESSAGE.Show(prompt8, CONCAT);

                  else {
                      MESSAGE.Show(prompt5, CONCAT);
                      separate_YE();
                  }

          }


          break;
      }

      singleYE = kefANDye = FALSE;

fixMin:
      reason = findMin(j, minDir);  _f_;

      if (reason == TERMINATE) {

          if (( *(Cs+j) & loopPixel) == loopPixel)

              if (MinList == NULL)
                  MESSAGE.Show(prompt4, CONCAT);
              else
                  MESSAGE.Show(prompt3, CONCAT);


          break;
      }

      else if (reason == MIN_FOUND  &&  ( *(Cs+j) & loopPixel) != loopPixel) {

          // check if this is 'lamelif'. 
          tmpMin = MinList;

          while (tmpMin != NULL) {

              if (tmpMin->position == j) {
                  MESSAGE.Show(prompt9, CONCAT);
                  goto quit;
              }
                  
              tmpMin = tmpMin->next;
          }


          if ((tmpMin = (struct CutMark *) HEAP.alloc (sizeof(struct CutMark)) ) == NULL) {
            appError = MIN_ALLOC;
            return(retVal=RTN_ERROR);
          }

          tmpMin->position = j;
          tmpMin->CFdirection = minDir;

          if (MinList == NULL) {
              MinList = tmpMin;
              MinList->next = NULL;
          }
          else {
              tmpMin->next = MinList;
              MinList = tmpMin;
          }

          displayStep(j, LIGHTGREEN);

          minSkipped = FALSE;
      }

      else if (reason == MIN_IGNORE)
          minSkipped = TRUE;

  }

quit:
  // clear 'jointVisited' attributes.

  j=-1;
  while (++j < matrixLen)
    if (( *(Cs+j) & jointVisited) == jointVisited)
       *(Cs+j) &= ~jointVisited;


  return(retVal=RTN_OK);
}


int CS::findFirstMin (long& i, int& minCFdir)
// ----------------------------------------------------------------------------
// In some cases, a cutMark should not be placed on first minimum.
// This fcn. takes a ratio, and decides on placing a cutMark on the min.
// that is found.
// ----------------------------------------------------------------------------
{ int k, curY, pvY, pvX, new_pvX, kf, firstX, firstY, XofMin, YofMin;
  int backward, forward, fwdTObwd, bwdTOfwd, reason, takeOff,
      found, minEP;
  int fbCOUNT, bfCOUNT, jntDir;
  int neighOffset[8] = {
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1
  };
  long j, StCandid, minStart, prevPos;
  float firstRatio;
  unsigned char jntCandid;
  char msg[30], prompt1[]="firstRatio = %.2f";


  findFarthestX( j );    _f_;
  displayStep(j, YELLOW);

  rightmostEP = j;

  pvDirection = SOUTH;
  IgnoreMin = FALSE;
  CheckYE = KEFandYE = HAfound = FALSE;


  if (( *(Cs+j) & EndPoint) == EndPoint)

    Pivot = j;

  else if (( *(Cs+j) | MidPoint) == MidPoint)

          if (( *(Cs+j) & loopPixel) == loopPixel) {
              Pivot = i = j;
              retVal = RTN_OK;
              return(REGULAR_MIN);
          }

          else {

              StCandid = j;

              kf = findStP(StCandid);  _f_;

              if (kf == LOOP_P) {

                  rightmostEP = Pivot = i = StCandid;
                  retVal = RTN_OK;
                  return(REGULAR_MIN);
              }

              else if (kf == END_P)

                  rightmostEP = Pivot = StCandid;

              else {

                  appError = START_POINT;
                  return(retVal=RTN_ERROR);
              }

          }

  else {

    appError = START_POINT;
    return(retVal=RTN_ERROR);
  }

  // now, we have a starting point of type E (EndPoint).


  minStart = i = Pivot;
  fbCOUNT = bfCOUNT = 0;

#pragma warn -sig
  firstY = curY = pvY = Pivot / Width;
  firstX = pvX  = Pivot % Width;
#pragma warn +sig

  minEP = takeOff = JointFound = FALSE;
  backward = fwdTObwd = bwdTOfwd = FALSE;
  forward = TRUE;
  minCFdir = pvDirection;

  do {

    pvDirection = (8 + pvDirection - 2) % 8;  // initially add +90 deg.
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+Pivot+neighOffset[pvDirection]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            pvDirection = (8 + pvDirection + 1) % 8;  // add -45 deg.



    if (found) {
        prevPos = Pivot;

        Pivot += neighOffset[pvDirection];

        if (Pivot == MainCompo->startPixel)
            return(TERMINATE);


        if (( *(Cs+Pivot) & BranchPoint) == BranchPoint  ||
            ( *(Cs+Pivot) & CrossPoint)  == CrossPoint ) {

            JointFound = TRUE;
            JointIndex = Pivot;
        }
        else if (pvDirection % 2  ==  1) {

            jntDir = (8 + pvDirection + 1) % 8;
            jntCandid = *(Cs+prevPos+neighOffset[jntDir]);

            if ((jntCandid & BranchPoint) == BranchPoint  ||
                (jntCandid & CrossPoint ) == CrossPoint) {

                JointFound = TRUE;
                JointIndex = prevPos + neighOffset[jntDir];
            }
        }

        if (JointFound) 
            break;


#pragma warn -sig
        pvY     = Pivot / Width;
        new_pvX = Pivot % Width;
#pragma warn +sig

        if (new_pvX > pvX) {

            if (forward) {
                // forward to backward transition starts.

                fwdTObwd = TRUE;
                forward  = FALSE;
                FB_STARTpoint = Pivot - neighOffset[pvDirection];
                fbCOUNT++;

            }

            else if (fwdTObwd) {

                if (++fbCOUNT  >  3) {
                    // forward to backward transition successfully completed.

                    backward = TRUE;
                    fwdTObwd = FALSE;
                }
            }

            else if (bwdTOfwd) {
                // backward to forward transition failed.

                bwdTOfwd = FALSE;
                backward = TRUE;
                bfCOUNT = 0;
            }

        }

        else if (new_pvX < pvX) {

            if (backward) {
                // backward to forward transition starts.

                bwdTOfwd = TRUE;
                backward = FALSE;
                BF_STARTpoint = Pivot - neighOffset[pvDirection];
                bfCOUNT++;
            }

            else if (bwdTOfwd) {

                if (++bfCOUNT  >  3) {
                    // backward to forward transition successfully completed.

                    bwdTOfwd = FALSE;
                    forward  = TRUE;

                    if (CheckYE)
                        // CheckYE would going to be set TRUE, but it is already
                        // TRUE. We are still to find first min. and CheckYE
                        // occurs twice. This is the sign of 'kef + ye'.
                        KEFandYE = TRUE;

                    else

                        CheckYE = TRUE;
                }

            }

            else if (fwdTObwd) {
                // forward to backward transition failed.

                fwdTObwd = FALSE;
                forward = TRUE;
                fbCOUNT = 0;
            }

        }

        pvX = new_pvX;

        if (( *(Cs+Pivot) & EndPoint) == EndPoint) {
            // this is an important case. We are on an endpoint, but
            // TERMINATE is not satisfied. From now on, the min. that
            // will be found must be ignored.
                minEP = TRUE;
                minStart = Pivot;

                // the following is to detect 'cim, ha, hI, chim' at end, or
                // single.
//              if (backward) {
//                  HAfound = TRUE;
//                  break;
//              }
        }

        if ((curY-pvY) == -1) {
            // new Pivot is lower, so make it the min. candidate
            minStart = i = Pivot;
            minCFdir = pvDirection;
            curY = pvY;
        }

        else if (curY == pvY)
            // new Pivot is same height, center min. candidate if necessary
            if (takeOff) {
                minStart = i = Pivot;
                minCFdir = pvDirection;
                takeOff = FALSE;
            }
            else {
                i = minStart  -  (minStart - Pivot) / 2;
                minCFdir = pvDirection;
            }

        else if ((curY-pvY) == 2)
            // O.K. we got it.

            break;

        else  // new Pivot is 1 higher
            takeOff = TRUE;

    }

    else {

        appError = NEXT_POINT3;
        return(retVal=RTN_ERROR);
    }

  } while (TRUE);


  if (IgnoreMin) {
      IgnoreMin = FALSE;
      return(MIN_IGNORE);
  }

  else if (minEP) {
      IgnoreMin = TRUE;
      return(MIN_IGNORE);
  }

  else if (CheckYE) 
      // the case when 'kef' is at the beginning, or single 'ye'.
      // First ratio may be small when this case occurs and cause the min.
      // to be ignored, so just return MIN_FOUND without calculating firstRatio.
      return(MIN_FOUND);


#pragma warn -sig
  XofMin = i % Width;
  YofMin = i / Width;
#pragma warn +sig

  if (XofMin == firstX)
      firstRatio = YofMin - firstY;
  else
      // 'abs' function is applied to denominator to prevent negative value.
      firstRatio = (float)(YofMin - firstY) / abs(firstX - XofMin);

  sprintf(msg, prompt1, firstRatio);
  MESSAGE.Show(msg, REFRESH);

  // Decide to ignore first min. or not.
  // Difference b/w Y-coordinates is checked, so that the segment is tall 
  // enough. This can be compared against the height of baseline, rather than
  // a constant (which is '2' here).
  if ( (firstRatio > 1.0) && (YofMin - firstY  >  2) )
      return(MIN_FOUND);
  else 
      return(MIN_IGNORE);

}


int CS::findMin (long& i, int& minCFdir)
// ----------------------------------------------------------------------------
// *** 30/11/93 ***
// ----------------------------------------------------------------------------
{ int k, curY, pvY, pvX, new_pvX;
  int backward, forward, fwdTObwd, bwdTOfwd, takeOff, jntDir,
      found, minEP;
  int fbCOUNT, bfCOUNT;
  int neighOffset[8] = {
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1
  };
  long minStart, prevPos;
  unsigned char jntCandid;
  unsigned char far *lastJntVisited;

  minStart = i = Pivot;
  fbCOUNT = bfCOUNT = 0;

#pragma warn -sig
  curY = pvY = Pivot / Width;
  pvX  = Pivot % Width;
#pragma warn +sig

  minEP = takeOff = JointFound = FALSE;
  backward = fwdTObwd = bwdTOfwd = FALSE;
  forward = TRUE;
  minCFdir = pvDirection;

  // the following check is to handle neighbor joints, i.e. when two 
  // B or C touch each other.

  if (( *(Cs+Pivot) & BranchPoint) == BranchPoint  ||
      ( *(Cs+Pivot) & CrossPoint) == CrossPoint  )   {

      JointFound = TRUE;
      JointIndex = Pivot;
      return(MIN_IGNORE);
  }



  do {

    pvDirection = (8 + pvDirection - 2) % 8;  // initially add +90 deg.
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+Pivot+neighOffset[pvDirection]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            pvDirection = (8 + pvDirection + 1) % 8;  // add -45 deg.


    if (found) {
        prevPos = Pivot;

        Pivot += neighOffset[pvDirection];

        if (Pivot == MainCompo->startPixel)
            return(TERMINATE);


        if (( *(Cs+Pivot) & BranchPoint) == BranchPoint  ||
            ( *(Cs+Pivot) & CrossPoint)  == CrossPoint ) {

            JointFound = TRUE;
            JointIndex = Pivot;
        }
        else if (pvDirection % 2  ==  1) {

            jntDir = (8 + pvDirection + 1) % 8;
            jntCandid = *(Cs+prevPos+neighOffset[jntDir]);

            if ((jntCandid & BranchPoint) == BranchPoint  ||
                (jntCandid & CrossPoint ) == CrossPoint) {

                JointFound = TRUE;
                JointIndex = prevPos + neighOffset[jntDir];
            }
        }

        if (JointFound)
            break;

#pragma warn -sig
        pvY     = Pivot / Width;
        new_pvX = Pivot % Width;
#pragma warn +sig

        if (new_pvX > pvX) {

            if (forward) {
                // forward to backward transition starts.

                fwdTObwd = TRUE;
                forward  = FALSE;
                FB_STARTpoint = Pivot - neighOffset[pvDirection];
                fbCOUNT++;

            }

            else if (fwdTObwd) {

                if (++fbCOUNT  >  3) {
                    // forward to backward transition successfully completed.

                    backward = TRUE;
                    fwdTObwd = FALSE;
                }
            }

            else if (bwdTOfwd) {
                // backward to forward transition failed.

                bwdTOfwd = FALSE;
                backward = TRUE;
                bfCOUNT = 0;
            }

        }

        else if (new_pvX < pvX) {

            if (backward) {
                // backward to forward transition starts.

                bwdTOfwd = TRUE;
                backward = FALSE;
                BF_STARTpoint = Pivot - neighOffset[pvDirection];
                bfCOUNT++;
            }

            else if (bwdTOfwd) {

                if (++bfCOUNT  >  3) {
                    // backward to forward transition successfully completed.

                    bwdTOfwd = FALSE;
                    CheckYE = TRUE;
                }

            }

            else if (fwdTObwd) {
                // forward to backward transition failed.

                fwdTObwd = FALSE;
                forward = TRUE;
                fbCOUNT = 0;
            }

        }

        pvX = new_pvX;

        if (( *(Cs+Pivot) & EndPoint) == EndPoint) {
            // this is an important case. We are on an endpoint, but
            // TERMINATE is not satisfied. From now on, the min. that
            // will be found must be ignored.
                minEP = TRUE;
                minStart = Pivot;

                // the following is to detect 'cim, ha, hI, chim' at end, or
                // single.
//              if (backward) {
//                  HAfound = TRUE;
//                  break;
//              }
        }

        if ((curY-pvY) == -1) {
            // new Pivot is lower, so make it the min. candidate
            minStart = i = Pivot;
            minCFdir = pvDirection;
            curY = pvY;
        }

        else if (curY == pvY) 
            // new Pivot is same height, center min. candidate if necessary
            if (takeOff) {
                minStart = i = Pivot;
                minCFdir = pvDirection;
                takeOff = FALSE;
            }
            else {
                i = minStart  -  (minStart - Pivot) / 2;
                minCFdir = pvDirection;
            }

        else if ((curY-pvY) == 2) 
            // O.K. we got it.

            break;

        else  // new Pivot is 1 higher
            takeOff = TRUE;

    }

    else {

            appError = NEXT_POINT3;
            return(retVal=RTN_ERROR);
    }

  } while (TRUE);


  if (backward  &&  !minEP ) {
      // 'ha, cim, etc.' found
      HAfound = TRUE;
      IgnoreMin = TRUE;
      return(MIN_IGNORE);
  }
  
  else if (IgnoreMin) {

      if (HAfound == TRUE) 
          HAfound = FALSE;
      else
          IgnoreMin = FALSE;

      return(MIN_IGNORE);
  }

  else if (minEP) {
      IgnoreMin = TRUE;
      return(MIN_IGNORE);
  }

  return(MIN_FOUND);
}


int CS::findMax (long& i)
// ----------------------------------------------------------------------------
// *** 30/11/93 ***
// ----------------------------------------------------------------------------
{ int k, curY, pvY, jntDir;
  int found;
  int neighOffset[8] = {
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1
  };
  unsigned char jntCandid;
  long prevPos;

  i = Pivot;

  if (JointFound) {

      *(Cs+JointIndex) |= jointVisited;

      if (Pivot != JointIndex) {

          i = JointIndex;
          CheckYE = FALSE;
          return(MAX_FOUND);
      }
  }

#pragma warn -sig
  curY = pvY = Pivot / Width;
#pragma warn +sig


  do {
/*
    for (k=0; JointFound == FALSE  &&  k<8; k++) {

            jntCandid = (unsigned char far *) (Cs+Pivot+neighOffset[k]);

            if (( *jntCandid & BranchPoint) == BranchPoint  ||
                ( *jntCandid & CrossPoint)  == CrossPoint) 

                   if ( (*jntCandid & jointVisited) != jointVisited) {
                       JointFound = TRUE;
                       JointIndex = Pivot + neighOffset[k];
                   }

    }
*/
    pvDirection = (8 + pvDirection - 2) % 8;  // initially add +90 deg.
    found = FALSE;

    for (k=0; found == FALSE  &&  k<8; k++)
        if ( (*(Cs+Pivot+neighOffset[pvDirection]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            pvDirection = (8 + pvDirection + 1) % 8;  // add -45 deg.


    if (found) {
        prevPos = Pivot;

        Pivot += neighOffset[pvDirection];

        if (Pivot == MainCompo->startPixel)
            return(TERMINATE);

        if (( *(Cs+Pivot) & BranchPoint) == BranchPoint  ||
            ( *(Cs+Pivot) & CrossPoint)  == CrossPoint ) {

//          if (( *(Cs+Pivot) & jointVisited) != jointVisited) {
                JointFound = TRUE;
                JointIndex = Pivot;
//          }
        }

        else if (pvDirection % 2  ==  1) {

            jntDir = (8 + pvDirection + 1) % 8;
            jntCandid = *(Cs+prevPos+neighOffset[jntDir]);

            if ((jntCandid & BranchPoint) == BranchPoint  ||
                (jntCandid & CrossPoint ) == CrossPoint) {

 //             if (( *(Cs+Pivot) & jointVisited) != jointVisited) {
                    JointFound = TRUE;
                    JointIndex = prevPos + neighOffset[jntDir];
//              }
            }
        }

        if (JointFound) {

            *(Cs+JointIndex) |= jointVisited;

            if (Pivot != JointIndex) {

                i = JointIndex;
                CheckYE = FALSE;
                return(MAX_FOUND);
            }
        }

#pragma warn -sig
        pvY  = Pivot / Width;
#pragma warn +sig

        if ((curY-pvY) == 1) {
            // new Pivot is higher, so make it the max. candidate
            i = Pivot;
            curY = pvY;
        }

        else if ((curY-pvY) == -2) {
            // O.K. we got it.
            CheckYE = FALSE;
            return(MAX_FOUND);
        }

    }

    else {
            appError = NEXT_POINT3;
            return(retVal=RTN_ERROR);
    }


  } while (TRUE);

}


int CS::findStP (long& candidate)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int direction, found, k, onLoop;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };


  direction = NORTH;

  onLoop = 0;

  do {

    direction = (8 + direction + 2) % 8;  
    found = FALSE;

    for (k=0; k<8  &&  found == FALSE; k++) 
        if ( (*(Cs+candidate+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction - 1) % 8;  


    if (found) {

        candidate = candidate + neighOffset[direction];

        displayStep(candidate, CYAN);

        if (( *(Cs+candidate) & EndPoint) == EndPoint)
                return(END_P);

        else if (( *(Cs+candidate) & loopPixel) == loopPixel)
                if (++onLoop > 2)
                    return(LOOP_P);

    }

    else {

        appError = NEXT_POINT4;
        return(retVal=RTN_ERROR);
    }


  } while (TRUE);

}


int CS::separate_YE ()
// ----------------------------------------------------------------------------
// this function generates the cutMark to separate letter 'ye' at end, through
// counter-clockwise contour(skeleton) following, i.e. goes left-to-right.
// The procedure starts from the point 'FB_STARTpoint' fixed previously.
// It tries to select a position on the pixel string of maximum horizontal 
// length.
// ----------------------------------------------------------------------------
{ int direction, found, k, jntFound=FALSE;
  int neighOffset[8] = { 
        -Width, -Width+1, 1, Width+1, Width, Width-1, -1, -Width-1 
  };
  long minCandid=FB_STARTpoint, minCndPivot;
  unsigned char far *jntCandid;
  int minCndLen, minCndPivotLen, minCndY, minCndDir;
  struct CutMark *tmpMin;


  direction = NORTH;
  minCndDir = SOUTH; // opposite of current direction (see explanation below).
  minCndLen = minCndPivotLen = 1;
#pragma warn -sig
  minCndY = FB_STARTpoint / Width;
#pragma warn +sig
  minCndPivot = minCandid;

  do {

    for (k=0; jntFound == FALSE  &&  k<8; k++) {

            jntCandid = (unsigned char far *) (Cs+minCndPivot+neighOffset[k]);

            if (( *jntCandid & BranchPoint) == BranchPoint  ||
                ( *jntCandid & CrossPoint)  == CrossPoint)

                       jntFound = TRUE;

    }

    if (jntFound)
        break;


    direction = (8 + direction + 2) % 8;  
    found = FALSE;

    for (k=0; k<8  &&  found == FALSE; k++) 
        if ( (*(Cs+minCndPivot+neighOffset[direction]) | foreGrnd) == foreGrnd)
            found = TRUE;
        else
            direction = (8 + direction - 1) % 8;  


    if (found) {

        minCndPivot += neighOffset[direction];

        displayStep(minCndPivot, CYAN);

        if (minCndPivot == MinList->position)
            // we have reached the last minimum. Therefore, do not place
            // a cutMark.
            return(retVal=RTN_OK);


        if (minCndPivot / Width  !=  minCndY) {

            if (minCndPivotLen >= minCndLen) {
                minCndLen = minCndPivotLen;
                minCandid = minCndPivot - neighOffset[direction];
                minCndDir = direction;
            }
#pragma warn -sig
            minCndY = minCndPivot / Width;
#pragma warn +sig
            minCndPivotLen = 1;
        }

        else 
            minCndPivotLen++;
        
    }

    else {

        appError = NEXT_POINT7;
        return(retVal=RTN_ERROR);
    }


  } while (TRUE);

  // add new min. to minList.

  if ((tmpMin = (struct CutMark *) HEAP.alloc (sizeof(struct CutMark)) ) == NULL) {
      appError = MIN_ALLOC;
      return(retVal=RTN_ERROR);
  }

  tmpMin->position = minCandid;
  tmpMin->CFdirection = (minCndDir + 4) % 8;   // opposite direction (add 180 deg.)
                                               // This has to be done to perform
                                               // clockwise skeleton following
                                               // during CS::findMFSg().
  if (MinList == NULL) {
      MinList = tmpMin;
      MinList->next = NULL;
  }
  else {
      tmpMin->next = MinList;
      MinList = tmpMin;
  }


  return(retVal=RTN_OK);
}