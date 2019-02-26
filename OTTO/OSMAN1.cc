/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <graphics.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include "otto.h"
#include "classdef.h"

extern unsigned _stklen = 16384U;

extern int appError;
extern int retVal;
extern char *appErrList[];
extern Message MESSAGE; 
extern Heap HEAP;

void beep (void)
// ----------------------------------------------------------------------------   
{ 
  sound(200);
  delay(100);
  nosound();
}
        

FileName::FileName (char *fName, char *reqMsg, char *pasteMsg)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ char dosCommand[60], prompt1[]="\nDOS Command: ",
                       prompt2[]="\nFile does not exist!\a\n",
                       prompt3[]="use";

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

        default    : if (stricmp(fName, prompt3) == 0)
                         return;
                                
                     if ( fileExist(fName) )
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

Mouse::Mouse()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  if (initMouse() == FALSE) 
    appError = INIT_MOUSE;
}

int Mouse::initMouse()
// ----------------------------------------------------------------------------
//  Initializes the mouse driver.
//  If the mouse driver is not installed, AX is returned FALSE (0).
// ----------------------------------------------------------------------------
{ union REGS regs;

  regs.x.ax = 0;
  int86(0x33, &regs, &regs);
  return(regs.x.ax);
}

void Mouse::showMouseCursor()
// ----------------------------------------------------------------------------
// make mouse cursor visible on screen
// ----------------------------------------------------------------------------
{ union REGS regs;
 
  regs.x.ax = 1;
  int86(0x33, &regs, &regs);
}

void Mouse::hideMouseCursor()
// ----------------------------------------------------------------------------
// make mouse cursor invisible on screen
// ----------------------------------------------------------------------------
{ union REGS regs;
 
  regs.x.ax = 2;
  int86(0x33, &regs, &regs);
}

void Mouse::setGraphMouseYRange (int minY, int maxY)
// ----------------------------------------------------------------------------
// set Y-coordinate range mouse cursor is allowed to move on.
// Restriction: Valid only for screen in graphics mode.
// ----------------------------------------------------------------------------
{ union REGS regs;

  regs.x.ax = 8;
  regs.x.cx = minY;
  regs.x.dx = maxY;
  int86(0x33, &regs, &regs);
}

int Mouse::pressedButtonGraphMouse (int& x, int& y)
// ----------------------------------------------------------------------------
// loops as button is kept pressed and mouse is not moved.
// Coordinates of the cursor in graphics mode are returned in arguments.
// Restriction: Valid only for screen in graphics mode.
// ----------------------------------------------------------------------------
{ union REGS inRegs, outRegs;

  inRegs.x.ax = 3;
  int86(0x33, &inRegs, &outRegs);

  // as long as the button is kept pressed
  while(outRegs.x.bx == BUTTON_PRESSED) {
    
    if (outRegs.x.cx != x  ||  outRegs.x.dx != y) {
        // mouse has been moved.
        x = outRegs.x.cx;
        y = outRegs.x.dx;
        return(BUTTON_PRESSED);
    }

    int86(0x33, &inRegs, &outRegs);
  }
  // button has been released
  return(BUTTON_RELEASED);
}

void Mouse::waitButtonPushGraphMouse (int& x, int& y)
// ----------------------------------------------------------------------------
// loops until mouse button is pressed. 
// Coordinates of the cursor in graphics mode are returned in arguments.
// Restriction: Valid only for screen in graphics mode.
// ----------------------------------------------------------------------------
{ union REGS inRegs, outRegs;

  inRegs.x.ax = 3;

  do 
    int86(0x33, &inRegs, &outRegs);

  while (outRegs.x.bx != BUTTON_PRESSED);

  x = outRegs.x.cx;
  y = outRegs.x.dx;
}

Screen::Screen (char *bgiDir)
// ----------------------------------------------------------------------------
{
  State = -1;     // state of screen is undefined initially 

  if (appError)   // an error occured during mouse constructor
    return;

  strcpy(BgiPath, bgiDir);
  Init();
}

void Screen::Init()
// ----------------------------------------------------------------------------
{ int gd=VGA, gm=VGALO, graphError;

  initgraph(&gd, &gm, BgiPath);
  graphError = graphresult();

  if (graphError != grOk) {

        appError = GRAPH_ERROR;
        sprintf(appErrList[appError], "%s", grapherrormsg(graphError));
        return;
  }

  State = GRAPH_16;
  menuHeight = 0;
}

int Screen::mode (int newMode)
// ----------------------------------------------------------------------------
// sets screen to a specific mode
// ----------------------------------------------------------------------------
{ int curMode, gd, gm;

  if (newMode == State)
    return(retVal=RTN_OK);

  curMode = getgraphmode();

  switch(newMode) {

    case GRAPH_16 : if (State == GRAPH_2) {
                        closegraph();
                        gd = VGA;
                        gm = VGALO;
                        initgraph(&gd, &gm, BgiPath);
                    }
                    else
                        setgraphmode(curMode);

                    msgLineHeight = MESSAGE.height();
                    setGraphMouseYRange(0, getmaxy() - msgLineHeight);
                    MESSAGE.Clear();

                    break;

    case GRAPH_2  : if (State == TEXT_8025  ||  State == TEXT_8050)
                        setgraphmode(curMode);

                    closegraph();
                    gd = MCGA;
                    gm = MCGAHI;
                    initgraph(&gd, &gm, BgiPath);

                    msgLineHeight = MESSAGE.height();
                    setGraphMouseYRange(0, getmaxy() - msgLineHeight);
                    MESSAGE.Clear();

                    break;

    case TEXT_8025: if (State == GRAPH_2) {
                        closegraph();
                        gd = VGA;
                        gm = VGALO;
                        initgraph(&gd, &gm, BgiPath);
                        restorecrtmode();
                    }

                    else if (State == GRAPH_16) {
                        restorecrtmode();
                        textmode(C80);
                    }

                    else
                        textmode(C80);

                    break;

    case TEXT_8050: if (State == GRAPH_2) {
                        closegraph();
                        gd = VGA;
                        gm = VGALO;
                        initgraph(&gd, &gm, BgiPath);
                        restorecrtmode();
                        textmode(C4350);
                    }

                    else if (State == GRAPH_16) {
                        restorecrtmode();
                        textmode(C4350);
                    }

                    else
                        textmode(C4350);

                    break;

    default       : appError = BAD_SCREENMODE;
                    return(retVal=RTN_ERROR);

  }

  State = newMode;
  return(retVal=RTN_OK);
}

void Screen::preFrameCut()
// ----------------------------------------------------------------------------
// establishes necessary settings before frame generation on displayed image
// ----------------------------------------------------------------------------
{
  setlinestyle(DASHED_LINE, 0, NORM_WIDTH);
  setwritemode(XOR_PUT);
  setcolor(1);
  setGraphMouseYRange(menuHeight, getmaxy() - msgLineHeight);
}

void Screen::postFrameCut()
// ----------------------------------------------------------------------------
// re-establishes previous conditions, after frame has been generated
// ----------------------------------------------------------------------------
{
  setGraphMouseYRange(0, getmaxy() - msgLineHeight);
}

void Screen::preBaselineGen()
// ----------------------------------------------------------------------------
// establishes necessary settings before baseline interval generation on frame
// ----------------------------------------------------------------------------
{
  setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
  setwritemode(XOR_PUT);
  setcolor(1);
  setGraphMouseYRange(menuHeight, getmaxy() - msgLineHeight);
}

void Screen::postBaselineGen()
// ----------------------------------------------------------------------------
// re-establishes previous conditions, after baseline interval has been generated
// ----------------------------------------------------------------------------
{
  setGraphMouseYRange(0, getmaxy() - msgLineHeight);
}

int Screen::menuParam (struct menuInfo *mnu)
// ----------------------------------------------------------------------------
// returns the required info. to display a menu on top of screen in
// a specific screen mode.
// ----------------------------------------------------------------------------
{ char prompt1[]="A";

  if (State == GRAPH_2) {
    mnu->charHeight = textheight(prompt1);
    mnu->charWidth  = textwidth(prompt1);
    mnu->fillColor  = 1;  // WHITE
    mnu->textColor  = 0;  // BLACK
    mnu->borderColor= 0;  
    mnu->left       = 0;
    mnu->right      = getmaxx();
  }
  else if (State == GRAPH_16) {
    mnu->charHeight = textheight(prompt1);
    mnu->charWidth  = textwidth(prompt1);
    mnu->fillColor  = LIGHTGRAY;  
    mnu->textColor  = 0;  // BLACK
    mnu->borderColor= 0;  
    mnu->left       = 0;
    mnu->right      = getmaxx();
  }
  else {
    appError = BAD_MENUSCR;
    return(retVal=RTN_ERROR);
  }

  return(retVal=RTN_OK);
}

int Screen::imageDispParam (struct imageDispInfo *imInfo)
// ----------------------------------------------------------------------------
// returns the required info. to display image in a specific screen mode.
// Image is displayed below the menu, and above the message line.
// ----------------------------------------------------------------------------
{ unsigned imgOffset;

  if (menuHeight == 0) {
  // there is not a menu on screen yet
  appError = NO_MENU;
  return(retVal=RTN_ERROR);
  }

  // memory offset of displayed menu in bytes 
  imgOffset = (getmaxx() + 1) * menuHeight / 8;

  if (State == GRAPH_2) {
    imInfo->displayBuf = (unsigned char far *) MK_FP(0xA000, imgOffset);
    imInfo->width      = getmaxx() + 1;
    imInfo->depth      = getmaxy() + 1 - menuHeight - msgLineHeight;
    imInfo->panOff     = 48; // this value has to be a power of 8.
    imInfo->scrollOff  = 35;
  }
  else {
    appError = BAD_IMGSCR;
    return(retVal=RTN_ERROR);
  }

  return(retVal=RTN_OK);
}

int Screen::controlScr (int& x, int& y)
// ----------------------------------------------------------------------------
// this function takes control while waiting for an action from user.
// It waits for mouse button push, and then returns region on which
// it has been pressed.
// Coordinates of the cursor in graphics mode are returned in arguments.
// Restriction: Valid only for screen in graphics mode.
// ----------------------------------------------------------------------------
{
  waitButtonPushGraphMouse(x, y);

  return(y < menuHeight  ?  ON_MENU  :  ON_IMAGE);
}

Screen::~Screen()
// ----------------------------------------------------------------------------
{
  if (State != -1)    
    closegraph();
}

CutOutFrame::CutOutFrame()
// ----------------------------------------------------------------------------
{
  FrameExist = FALSE;
}

void CutOutFrame::frameGen (int startX, int startY, Screen *scr)
// ----------------------------------------------------------------------------
// generates a rectangular frame on displayed image. 
// Frame is re-sizeable as mouse button is kept pressed.
// Terminates when button is released.
// ----------------------------------------------------------------------------
{ int endX = startX--, 
      endY = startY--;
  int x, y;

  scr->preFrameCut();

  scr->hideMouseCursor();
  if ( FrameExist ) 
    // there is already a frame, so erase it first
    rectangle(FrLeft, FrTop, FrRight, FrBottom);

  rectangle(startX, startY, endX, endY);

  scr->showMouseCursor();
  while (scr->pressedButtonGraphMouse(x, y) == BUTTON_PRESSED) {

    scr->hideMouseCursor();
    rectangle(startX, startY, endX, endY);
    endX = x;
    endY = y;
    rectangle(startX, startY, endX, endY);
    scr->showMouseCursor();
  }

  FrameExist = TRUE;

  if (startX < endX) {
    FrLeft  = startX;
    FrRight = endX;
  }
  else {
    FrLeft  = endX;
    FrRight = startX;
  }

  if (startY < endY) {
    FrTop    = startY;
    FrBottom = endY;
  }
  else {
    FrTop    = endY;
    FrBottom = startY;
  }

  // initialize the top and bottom of baseline interval
  BaseTop = FrTop + 1;
  BaseBottom = FrBottom - 1;

  scr->hideMouseCursor();  
  rectangle(startX, startY, endX, endY);        // erase the frame and  
  rectangle(FrLeft, FrTop, FrRight, FrBottom);  // redraw with changed values
  scr->showMouseCursor();

  scr->postFrameCut();
}

void CutOutFrame::baselineGen (Screen *scr)
// ----------------------------------------------------------------------------
// generates a rectangular frame on displayed image. 
// Frame is re-sizeable as mouse button is kept pressed.
// Terminates when button is released.
// ----------------------------------------------------------------------------
{ int x, y, startY, newVal;

  if ((FrRight - FrLeft) < 8  ||  (FrBottom - FrTop) < 8  // frame too small.
                              ||  ( ! FrameExist )        // no frame generated.
     ) {   
    beep();
    return;
  }

  scr->preBaselineGen();

  scr->hideMouseCursor();
  line(FrLeft+1, BaseTop, FrRight-1, BaseTop);
  line(FrLeft+1, BaseBottom, FrRight-1, BaseBottom);
  scr->showMouseCursor();

  scr->controlScr(x, y);
  startY = y;

  // adjust the top of baseline interval
  while (scr->pressedButtonGraphMouse(x, y) == BUTTON_PRESSED) 

    if (y != startY) {
        newVal = BaseTop + (y - startY);

        if (newVal > FrTop  &&  newVal < BaseBottom) {

            scr->hideMouseCursor();
            line(FrLeft+1, BaseTop, FrRight-1, BaseTop);
            BaseTop = newVal;
            line(FrLeft+1, BaseTop, FrRight-1, BaseTop);
            scr->showMouseCursor();

            startY = y;
        }
    }

  scr->controlScr(x, y);
  startY = y;

  // adjust the bottom of baseline interval
  while (scr->pressedButtonGraphMouse(x, y) == BUTTON_PRESSED) 

    if (y != startY) {
        newVal = BaseBottom + (y - startY);

        if (newVal < FrBottom  &&  newVal > BaseTop) {

            scr->hideMouseCursor();
            line(FrLeft+1, BaseBottom, FrRight-1, BaseBottom);
            BaseBottom = newVal;
            line(FrLeft+1, BaseBottom, FrRight-1, BaseBottom);
            scr->showMouseCursor();

            startY = y;
        }
    }

  // erase the lines from frame
  scr->hideMouseCursor();
  line(FrLeft+1, BaseTop, FrRight-1, BaseTop);
  line(FrLeft+1, BaseBottom, FrRight-1, BaseBottom);
  scr->showMouseCursor();

  scr->postBaselineGen();
}

Image::Image (char *imgFname, Screen *scr)
// ----------------------------------------------------------------------------
{ char prompt1[]="use",
       prompt2[]="Please wait...";

  int fhImg; 
  char fType;

  FhImage = NOHANDLE;

  if (stricmp(imgFname, prompt1) == 0) {
      Use();
      if (appError)  return;
      goto next;
  }

  MESSAGE.Show(prompt2, CONCAT);

  if ((fhImg = open(imgFname, O_BINARY | O_RDONLY)) == -1) {
    appError = IMGF_OPEN;
    return;
  }

  // first byte represents the type of image file (PCX, BMP, etc.)
  read(fhImg, &fType, 1);
  lseek(fhImg, 0L, SEEK_SET);

  switch(fType) {

        case PCX  : decompressPCX(fhImg);
                    break;

        default   : appError = BAD_IMGFORMAT;
                    break;
  }

  close(fhImg);

  if (appError)    return;

next:
  scr->imageDispParam(&iInfo);
  if (appError)   return;

  MinDispX = MinDispY = 1;

  if (iInfo.width < Width) {
    MaxDispX = iInfo.width;
    ImgRowOff = (Width - MaxDispX) / 8;
  }
  else {
    MaxDispX = Width;
    ImgRowOff = 0;
  }

  MaxDispY = (iInfo.depth < Depth  ?  iInfo.depth : Depth);

  Show();
}


int Image::Use ()
// ----------------------------------------------------------------------------
// use previously decompressed image on disk.
// ---------------------------------------------------------------------------- 
{ char tmpFname[]="TEMP.IMG", tmpFDim[]="TEMP.DIM";
  int fhImg; 

  if ((fhImg = open(tmpFname, O_BINARY | O_RDONLY)) == -1) {
    appError = IMGF_OPEN;
    return(retVal=RTN_ERROR);
  }

  FhImage = fhImg;

  if ((fhImg = open(tmpFDim, O_BINARY | O_RDONLY)) == -1) {
    appError = IMGF_OPEN;
    return(retVal=RTN_ERROR);
  }

  read(fhImg, &Width, sizeof(Width));
  read(fhImg, &Depth, sizeof(Depth));
  close(fhImg);

  return(retVal=RTN_OK);
}


int Image::decompressPCX (int fh)
// ----------------------------------------------------------------------------
// converts PCX-formatted image file into raw bitmap matrix.
// Bitmap is temporarily saved onto disk.
// ----------------------------------------------------------------------------
{ unsigned char *bitmapBuf;
  struct PCXHead header;
  unsigned char ch;
  int line, width, depth, bytes_per_row, bufLines, bufLineCount;
  int repeat, imgFh;
  unsigned i=0, j, FBUFSIZE=65535U;
  long heapSize=HEAP.avail();
  char tmpFname[]="TEMP.IMG";
  char tmpFDim[]="TEMP.DIM";

  if (heapSize < FBUFSIZE) 
    FBUFSIZE = (unsigned) heapSize;

  if ((bitmapBuf = (unsigned char *) HEAP.alloc (FBUFSIZE)) == NULL) {
    appError = BUFFER_ALLOC;
    return(retVal=RTN_ERROR);
  }

  read(fh, &header, sizeof(struct PCXHead));

  if (header.color_planes != 1) {
    appError = NOT_BINARYPCX;
    HEAP.free(bitmapBuf);
    return(retVal=RTN_ERROR);
  }

  width = header.xmax - header.xmin + 1;

  if (width > 8000) {
    appError = TOO_BIGPCX;
    HEAP.free(bitmapBuf);
    return(retVal=RTN_ERROR);
  }
  // round width to nearest 8's power
  width = (width / 8) * 8;

  bytes_per_row = width / 8;

  // calculate the number of decompressed image lines that can be accumulated 
  // in buffer 'bitmapBuf'.
  bufLines = FBUFSIZE / header.bytes_per_line;

  depth = header.ymax - header.ymin + 1;

  if ((imgFh = open(tmpFname, O_CREAT | O_TRUNC | O_BINARY | O_RDWR,
                              S_IWRITE | S_IREAD)) == -1) {
    appError = TMP_CREATE;
    HEAP.free(bitmapBuf);
    return(retVal=RTN_ERROR);
  }
  else
    FhImage = imgFh;

  for (line=1, bufLineCount=1; line<=depth; line++, bufLineCount++) {

    j = 0;

    do {
        if ( read(fh, &ch, 1) != 1) {
            appError = BAD_PCXREAD;
            HEAP.free(bitmapBuf);
            return(retVal=RTN_ERROR);
        }
        if ((ch & 0xC0) == 0xC0) {    // two highest bits are 11
            repeat = ch & 0x3F;

            if ( read(fh, &ch, 1) != 1) {
                appError = BAD_PCXREAD;
                HEAP.free(bitmapBuf);
                return(retVal=RTN_ERROR);
            }

            while (repeat--) {
                bitmapBuf[i++] = ch;
                j++;
            }
        }

        else {
            bitmapBuf[i++] = ch;
            j++;
        }

    } while (j < header.bytes_per_line);

    if (j == header.bytes_per_line) {
        if (j > bytes_per_row)
            i = i - (j - bytes_per_row);

        if (bufLineCount == bufLines) {

            if ( write(FhImage, bitmapBuf, i) != i) {
                appError = TMP_WRITE;
                HEAP.free(bitmapBuf);
                return(retVal=RTN_ERROR);
            }
            bufLineCount = 0;
            i = 0;
        }
    }
    else {
        appError = BAD_PCXLINE;
        HEAP.free(bitmapBuf);
        return(retVal=RTN_ERROR);
    }

  } // for 

  if (i > 0)
    if ( write(FhImage, bitmapBuf, i) != i) {
        appError = TMP_WRITE;
        HEAP.free(bitmapBuf);
        return(retVal=RTN_ERROR);
    }

//  imShow(FhImage, width, depth);

  Width = width;
  Depth = depth;

  HEAP.free(bitmapBuf);

  if ((imgFh = open(tmpFDim, O_CREAT | O_TRUNC | O_BINARY | O_RDWR,
                             S_IWRITE | S_IREAD)) == -1) {
    appError = TMP_CREATE;
    return(retVal=RTN_ERROR);
  }
  write(imgFh, &Width, sizeof(Width));
  write(imgFh, &Depth, sizeof(Depth));
  close(imgFh);

  return(retVal=RTN_OK);
}


void Image::Show()
// ----------------------------------------------------------------------------
// displays raw image on disk(in temporary file) onto screen.
// Constraint: 640x480 2-color graphics mode (MCGAHI) assumed.
// ---------------------------------------------------------------------------- 
{ unsigned char far *dispMem;
  int dispRowOff, dispRowLen, i;

  dispMem = iInfo.displayBuf;
  dispRowOff = iInfo.width / 8;

  dispRowLen = (MaxDispX - MinDispX + 1) / 8;

  lseek(FhImage, ((long)(MinDispY - 1) * (Width / 8)) + (MinDispX / 8), SEEK_SET);

  if (ImgRowOff > 0)

    for (i=MinDispY; i<=MaxDispY; i++) {
        read(FhImage, dispMem, dispRowLen);
        lseek(FhImage, ImgRowOff, SEEK_CUR);
        dispMem += dispRowOff;
    }

  else

    for (i=MinDispY; i<=MaxDispY; i++) {
        read(FhImage, dispMem, dispRowLen);
        dispMem += dispRowOff;
    }

}


void Image::rightPan()
// ----------------------------------------------------------------------------
// shifts image to right 
// ----------------------------------------------------------------------------
{ int panOff, dispRowLen, dispRowOff, i;
  unsigned char far *dispMem;

  if (MaxDispX == Width) {
    beep();
    return;
  }

  panOff = (MaxDispX + iInfo.panOff  >  Width) ? (Width - MaxDispX) : iInfo.panOff;

  dispRowLen = dispRowOff = iInfo.width / 8;

  dispMem = iInfo.displayBuf;

  MinDispX += panOff;
  MaxDispX += panOff;

  lseek(FhImage, ((long)(MinDispY - 1) * (Width / 8)) + (MinDispX / 8), SEEK_SET);

  for (i=MinDispY; i<=MaxDispY; i++) {
    
    read(FhImage, dispMem, dispRowLen);
    lseek(FhImage, ImgRowOff, SEEK_CUR);
    dispMem += dispRowOff;
  }

  FrameExist = FALSE;
}

void Image::leftPan()
// ----------------------------------------------------------------------------
// shifts image to left
// ----------------------------------------------------------------------------
{ int panOff, dispRowLen, dispRowOff, i;
  unsigned char far *dispMem;

  if (MinDispX == 1) {
    beep();
    return;
  }

  panOff = (MinDispX - iInfo.panOff < 1) ? (MinDispX - 1) : iInfo.panOff;

  dispRowLen = dispRowOff = iInfo.width / 8;

  dispMem = iInfo.displayBuf;

  MinDispX -= panOff;
  MaxDispX -= panOff;

  lseek(FhImage, ((long)(MinDispY - 1) * (Width / 8)) + (MinDispX / 8), SEEK_SET);

  for (i=MinDispY; i<=MaxDispY; i++) {
    
    read(FhImage, dispMem, dispRowLen);
    lseek(FhImage, ImgRowOff, SEEK_CUR);
    dispMem += dispRowOff;
  }

  FrameExist = FALSE;
}

void Image::scrollDown ()
// ----------------------------------------------------------------------------
// scrolls down the image
// ----------------------------------------------------------------------------
{ int scrollOff, dispRowLen, dispRowOff, i;
  unsigned char far *dispMem;

  if (MaxDispY == Depth) {
    beep();
    return;
  }

  scrollOff = (MaxDispY + iInfo.scrollOff > Depth) ? (Depth - MaxDispY) : iInfo.scrollOff;

  dispRowLen = dispRowOff = iInfo.width / 8;

  dispMem = iInfo.displayBuf;

  MinDispY += scrollOff;
  MaxDispY += scrollOff;

  lseek(FhImage, ((long)(MinDispY - 1) * (Width / 8)) + (MinDispX / 8), SEEK_SET);

  for (i=MinDispY; i<=MaxDispY; i++) {
    
    read(FhImage, dispMem, dispRowLen);
    lseek(FhImage, ImgRowOff, SEEK_CUR);
    dispMem += dispRowOff;
  }

  FrameExist = FALSE;
}

void Image::scrollUp ()
// ----------------------------------------------------------------------------
// scrolls up the image
// ----------------------------------------------------------------------------
{ int scrollOff, dispRowLen, dispRowOff, i;
  unsigned char far *dispMem;

  if (MinDispY == 1) {
    beep();
    return;
  }

  scrollOff = (MinDispY - iInfo.scrollOff < 1) ? (MinDispY - 1) : iInfo.scrollOff;

  dispRowLen = dispRowOff = iInfo.width / 8;

  dispMem = iInfo.displayBuf;

  MinDispY -= scrollOff;
  MaxDispY -= scrollOff;

  lseek(FhImage, ((long)(MinDispY - 1) * (Width / 8)) + (MinDispX / 8), SEEK_SET);

  for (i=MinDispY; i<=MaxDispY; i++) {
    
    read(FhImage, dispMem, dispRowLen);
    lseek(FhImage, ImgRowOff, SEEK_CUR);
    dispMem += dispRowOff;
  }

  FrameExist = FALSE;
}


int Image::frameCutOut (int& width, int& depth, unsigned char huge *& frCut, 
                        int& baseTop, int& baseBottom, int menuHeight)
// ----------------------------------------------------------------------------
// cuts out the frame generated on screen into an array of char, in which  
// each pixel is stored into a byte sequentially row by row, i.e. 
// a 2-D object (matrix) is stored into 1-D object.
// If the frame does not exist on screen or it is not sufficiently large,
// the fcn. rejects to cut out.
// ----------------------------------------------------------------------------
{ unsigned char mask, frRowBuf[80];
  int minCutX, minCutY, maxCutX, maxCutY, cutRowLen, seekOffset, x_offSet;
  int byte_offSet, stBitNum, i, j, m, w;
  long k;

  if ((FrRight - FrLeft) < 8  ||  (FrBottom - FrTop) < 8  // frame too small.
                              ||  ( ! FrameExist )        // no frame generated.
     ) {   
    frCut = (unsigned char *) 0;
    beep();
    return(retVal=RTN_OK);
  }

  baseTop = BaseTop - FrTop - 1;
  baseBottom = BaseBottom - FrTop + 1;

  minCutX = MinDispX + FrLeft;
  minCutY = MinDispY + (FrTop - menuHeight);
  maxCutX = MinDispX + FrRight;
  maxCutY = MinDispY + (FrBottom - menuHeight);

  width = maxCutX - minCutX + 1;
  depth = maxCutY - minCutY + 1;

  // The smallest value that both of 'MinDispX' and 'MinDispY' variables 
  // can take is 1 (one), whereas this is 0 (zero) for 'FrLeft'.
  // Therefore 'minCutX' takes 1 (one) as the smallest value. A byte
  // boundary for this variable is 1, 9, 17, 25, etc. That's why we 
  // subtract 1 below.

  x_offSet = (minCutX - 1) / 8;

  // number of bits to skip in the first byte of frame row. If 'minCutX' is
  // at byte boundary, then we do not have to skip any bit, which will make
  // 'byte_offSet' zero (0) below.

  byte_offSet = (minCutX - 1) % 8;

  // number of bytes in a frame row (on image). We need to add 2, to account
  // for partial bytes that may happen both at the start and the end of row.

  cutRowLen = width / 8 + 2;



  frCut = (unsigned char huge *) HEAP.alloc((long)width * depth);

  if (frCut == NULL) {
    appError = CUTFRAME_ALLOC;
    return(retVal=RTN_ERROR);
  }

  lseek(FhImage, ((long)(minCutY - 1) * (Width / 8)) + x_offSet, SEEK_SET);

  seekOffset = (Width / 8) - cutRowLen;
  k=0;

  for (i=minCutY; i<=maxCutY; i++) {

    w = 0;
    read(FhImage, frRowBuf, cutRowLen);

    mask = 0x80;
    mask >>= byte_offSet;
    stBitNum = byte_offSet;

    for (j=0; j<cutRowLen; j++) {

        for (m=stBitNum; m<8  &&  w<width; m++, w++, k++) {
            // BACKGRND(WHITE) = 1, FOREGRND(BLACK) = 0
            *(frCut+k) = (frRowBuf[j] & mask)  ?  BACKGRND : FOREGRND;
            mask >>= 1;
       }

       mask = 0x80;
       stBitNum = 0;
    }

    lseek(FhImage, seekOffset, SEEK_CUR);
  }

  FrameExist = FALSE;

  return(retVal=RTN_OK);
}

void Image::showImgSize (int op)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ char msg[30];

  sprintf(msg, "%4dx%4d", Width, Depth);
  MESSAGE.Show(msg, op);
}

Image::~Image()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ 
  if (FhImage != NOHANDLE) {
    close(FhImage);
//    remove(tmpFname);
  }
}

Menu::Menu (int nCommand, char *strCommand[], Screen *scr)
// ----------------------------------------------------------------------------
// creates a menu. 'nCommand' is the number of menu commands, 
// 'strCommand' holds menu command strings.
// ----------------------------------------------------------------------------
{ struct menuInfo mInfo;
  int right;
  int maxCmdLen, cmdLen, i;

  unsigned bufSize;

//  CmdXBorders  = (int *) 0;
//  CmdBoxImgBuf = (unsigned char far *) 0;

  if (scr->menuParam(&mInfo) != RTN_OK)    return;

  MenuHeight = 2 * mInfo.charHeight;
  scr->set_menuHeight(MenuHeight);

  right = mInfo.left;

  maxCmdLen = 0;

//  if ((CmdXBorders = (int *) HEAP.alloc(sizeof(int) * (nCommand+1)) ) == NULL) {
//      appError = MENU_ALLOC1;
//      return;
//  }

  CmdXBorders[0] = right;

  for (i=1; i<=nCommand; i++) {

    cmdLen = strlen(strCommand[i-1]);
    if (cmdLen > maxCmdLen)
        maxCmdLen = cmdLen;

    right += (cmdLen + 2) * mInfo.charWidth; 
    CmdXBorders[i] = ++right;     

  }
/*
  bufSize = imagesize(0, 0, (maxCmdLen + 2) * mInfo.charWidth, MenuHeight - 1);
        
  if ((CmdBoxImgBuf = (unsigned char far *) HEAP.alloc (bufSize) ) == NULL) {
      appError = MENU_ALLOC2;
      return;
  }
*/
  CmdNum = nCommand;

  Show(strCommand, scr);
}


int Menu::Show (char *strCommand[], Screen *scr)
// ----------------------------------------------------------------------------
// displays menu horizontally on top of the screen.
// ----------------------------------------------------------------------------
{ struct menuInfo mInfo;
  int top, bottom, i;

  top = 1;
  bottom = MenuHeight - 2;

  if (scr->menuParam(&mInfo) != RTN_OK)
      return(retVal=RTN_ERROR);

  setfillstyle(SOLID_FILL, mInfo.fillColor);
  settextjustify(CENTER_TEXT, CENTER_TEXT);
  setcolor(mInfo.borderColor);
  rectangle(mInfo.left, 0, mInfo.right, bottom + 1);

  setcolor(mInfo.textColor);

  for (i=1; i<=CmdNum; i++) {
    bar(CmdXBorders[i-1] + 1, top, CmdXBorders[i] - 1, bottom);
    outtextxy((CmdXBorders[i-1] + CmdXBorders[i]) / 2, (top + bottom) / 2, strCommand[i-1]);
  }

  return(retVal=RTN_OK);
}

int Menu::menuCommandSelect (int x, int y, Screen *scr)
// ----------------------------------------------------------------------------
// this func. is called when mouse button has been pressed on (selected)  
// a menu command. It returns 0, if the command is deselected. 
// If button is released on the selected command, it returns the command
// number.
// ----------------------------------------------------------------------------
{ int i;

  // find command number on which mouse button pressed
  for (i=1; i<=CmdNum && x>=CmdXBorders[i]; i++) ;

  if (i > CmdNum)    return(0);

  // respond by reversing the menu command image
  scr->hideMouseCursor();
  getimage(CmdXBorders[i-1], 0, CmdXBorders[i], MenuHeight - 1, CmdBoxImgBuf);
  putimage(CmdXBorders[i-1], 0, CmdBoxImgBuf, NOT_PUT);
  scr->showMouseCursor();

  // as long as button kept pressed
  while(scr->pressedButtonGraphMouse(x, y) == BUTTON_PRESSED)

    if (y > MenuHeight  ||  x < CmdXBorders[i-1]  ||  x > CmdXBorders[i]) {
        // button is not released, and mouse cursor is not on menu command
        scr->hideMouseCursor();
        putimage(CmdXBorders[i-1], 0, CmdBoxImgBuf, COPY_PUT);
        scr->showMouseCursor();
        return(0);
    }

  // button released on menu command, i.e. command is selected
  scr->hideMouseCursor();
  putimage(CmdXBorders[i-1], 0, CmdBoxImgBuf, COPY_PUT);
  scr->showMouseCursor();
  return(i);
}

/*
Menu::~Menu()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  if (CmdXBorders != NULL)
    HEAP.free(CmdXBorders);

  if (CmdBoxImgBuf != NULL)
    HEAP.free(CmdBoxImgBuf);
}
*/

DumbMenu::DumbMenu (int nCommand, char *strCommand[], Screen *scr) : 
              Menu (nCommand, strCommand, scr)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  lastSelected = 0;
}

int DumbMenu::menuCommandSelect (int x, int y, Screen *scr, int& choice)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ int mnuCmd, newSelect=FALSE, curOp=lastSelected+1;

  mnuCmd = Menu::menuCommandSelect(x, y, scr);

  if (mnuCmd == CmdNum) {
    // RETURN (last command) is selected
    choice = mnuCmd;
    return(mnuCmd);
  }

  setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
  setwritemode(COPY_PUT);
  setcolor(LIGHTRED);
  scr->hideMouseCursor();

  while (lastSelected < mnuCmd) {
    lastSelected++;
    rectangle(CmdXBorders[lastSelected-1], 0, CmdXBorders[lastSelected], MenuHeight - 1);
    newSelect = TRUE;
  }

  scr->showMouseCursor();

  if (newSelect) {
      choice = mnuCmd;
      return(curOp);
  }
  else
      return(0);
  
}    

Message::Message ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  MsgLen = MAX_MSGLen;
  MsgString[MsgLen-1] = '\x0';
  Clear();
}

void Message::Show (char *mStr, int op)
// ----------------------------------------------------------------------------
// When 'op' is REFRESH, existing message is overwritten. With 'op' == CONCAT,
// 'mStr' is concatenated to the existing message.
// ----------------------------------------------------------------------------
{
  settextjustify(LEFT_TEXT, BOTTOM_TEXT);

  if (MsgExist) 

    if (op == CONCAT) 

        strncat(MsgString, mStr, MsgLen-strlen(MsgString)-2);

    else {
        setcolor(BLACK);
        outtextxy(0, getmaxy(), MsgString);
        strncpy(MsgString, mStr, MsgLen-2);
    }

  else
    strncpy(MsgString, mStr, MsgLen-2);

  setcolor(WHITE);
  outtextxy(0, getmaxy(), MsgString);

  MsgExist = TRUE;
}

void Message::Clear ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  MsgExist = FALSE;
}

int Message::height ()
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  return( textheight("A") + 2);
}

void Message::getInp (char *prompt, char *iStr)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{ char cursor[]="_", ch, minChar=32, maxChar=126;
  int ptr=0, charWidth=textwidth("A"), curX, curY, maxInpLen;

  settextjustify(LEFT_TEXT, BOTTOM_TEXT);

  maxInpLen = (MsgLen-1) - strlen(prompt);

  if (MsgExist) {
    setcolor(BLACK);
    outtextxy(0, getmaxy(), MsgString);
  }

  strncpy(MsgString, prompt, MsgLen-2);
  MsgExist = TRUE;
  iStr[ptr] = '\x0';

  setcolor(WHITE);
  moveto(0, getmaxy());
  outtext(MsgString);
  curX = getx();
  curY = gety();

  setcolor(WHITE);
  outtextxy(curX, curY, cursor);

  while ((ch=getch()) != '\r') {   // as long as ENTER is not pressed

    if (ch == '\b'  &&  ptr > 0) {
        // BACKSPACE pressed

        // erase cursor
        setcolor(BLACK);
        outtextxy(curX, curY, cursor);
        setcolor(WHITE);

        ptr--;
        setcolor(BLACK);
        moverel(-charWidth, 0);
        curX = getx();
        curY = gety();
        outtextxy(curX, curY, iStr+ptr);
        setcolor(WHITE);
        outtextxy(curX, curY, cursor);
        iStr[ptr] = '\x0';
    }

    else if (ptr < maxInpLen  &&  ch >= minChar  &&  ch <= maxChar) {
        // a valid character entered

        // erase cursor
        setcolor(BLACK);
        outtextxy(curX, curY, cursor);
        setcolor(WHITE);

        iStr[ptr] = ch;
        iStr[ptr+1] = '\x0';
        outtext(iStr+ptr);
        curX = getx();
        curY = gety();
        outtextxy(curX, curY, cursor);
        ptr++;
    }

  }

  // erase cursor
  setcolor(BLACK);
  outtextxy(curX, curY, cursor);
  setcolor(WHITE);

  // copy input string into buffer so that it can be cleared
  strcat(MsgString, iStr);
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


void far * far _graphgetmem (unsigned size)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  return( HEAP.alloc(size) );
}

#pragma argsused
void far _graphfreemem (void far *ptr, unsigned size)
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
{
  HEAP.free(ptr);
}
