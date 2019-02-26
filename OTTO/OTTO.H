/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#define FALSE   0
#define TRUE    1

// general function return values
#define RTN_ERROR       0
#define RTN_OK          1
#define MIN_FOUND       2
#define MAX_FOUND       3
#define LOOP_FOUND      4
#define TERMINATE       5
#define END_P           6
#define LOOP_P          7
#define MIN_IGNORE      8
#define MAX_IGNORE      9
#define REGULAR_MIN     10
#define BRN_P           11
#define END_CUT         12


#define NOHANDLE        -2

// image file formats currently supported
#define PCX     10


#define GRAPH_2         0
#define GRAPH_16        1
#define TEXT_8025       2
#define TEXT_8050       3


// index constant mnemonics for application error messages
#define INIT_MOUSE      1
#define GRAPH_ERROR     2
#define BAD_SCREENMODE  3
#define SCREEN_ALLOC    4
#define IMGF_OPEN       5
#define BAD_IMGFORMAT   6
#define NOT_BINARYPCX   7
#define TOO_BIGPCX      8
#define TMP_CREATE      9
#define BAD_PCXREAD     10
#define TMP_WRITE       11
#define BAD_PCXLINE     12
#define BAD_MENUSCR     13
#define BAD_IMGSCR      14
#define MENU_ALLOC1     15
#define MENU_ALLOC2     16
#define END_PROGRAM     17
#define NO_MENU         18
#define CUTFRAME_ALLOC  19
#define NEXT_CONTOUR1   20
#define NEXT_CONTOUR2   21
#define LPCNT_ALLOC     22
#define BAD_PIXEL       23
#define NEXT_CONTOUR3   24
#define COMPO_ALLOC     25
#define NEXT_CONTOUR4   26
#define NEXT_LPIXEL     27
#define TWIN_LOOP       28
#define NO_COMPO        29
#define HEAP_ERROR      30
#define BUFFER_ALLOC    31
#define MIN_ALLOC       32
#define BAD_MIN2        33
#define BAD_MAX2        34
#define START_POINT     35
#define BAD_MIN1        36
#define BAD_MAX1        37
#define NEXT_POINT1     38
#define NEXT_POINT2     39
#define NEXT_POINT3     40
#define NEXT_POINT4     41
#define NEXT_POINT5     42
#define NEXT_POINT6     43
#define NEXT_POINT7     44
#define SEG_ALLOC       45
#define DANG_LOOP       46
#define NEXT_POINT8     47
#define NEXT_POINT9     48
#define NEXT_POINT10    49
#define NEXT_POINT11    50
#define NEXT_POINT12    51
#define NEXT_POINT13    52
#define NEXT_POINT14    53
#define NEXT_POINT15    54
#define NEXT_MFSGPIX    55
#define NEXT_POINT16    56
#define BAD_GRIDSP      57
#define MFSG_ALLOC      58
#define NO_TRNDIR       59
#define MFSGF_OPEN      60
#define HMM_ALLOC1      61
#define HMM_ALLOC2      62
#define HMM_ALLOC3      63
#define HMM_ALLOC4      64
#define HMM_ALLOC5      65
#define FVAR_ALLOC1     66
#define FVAR_ALLOC2     67
#define NO_MODEL        68
#define MDLF_OPEN       69

#define _o_     if (appError)   return(appError)
#define _f_     if (retVal == RTN_ERROR)        return(appError)
#define _r_     if (retVal == RTN_ERROR)        return(retVal);

#define BUTTON_RELEASED 0
#define BUTTON_PRESSED  1

#define ON_MENU         1
#define ON_IMAGE        2

#define CLOCKWISE           99
#define COUNTER_CLOCKWISE   101

#define REFRESH         11
#define CONCAT          12

/*  
        +---+---+---+---+---+---+---+---+
        | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
        +---+---+---+---+---+---+---+---+
          |   |   |   |   |   |   |   +--- 0: foreground
          |   |   |   |   |   |   |        1: background
          |   |   |   |   |   |   |
          |   |   |   |   |   |   |  00: M
          |   |   |   |   |   |   +- 01: B
          |   |   |   |   |   +----- 10: E
          |   |   |   |   |          11: C  or  contour pixel
          |   |   |   |   |
          |   |   |   |   +----- 1: loop pixel
          |   |   |   |
          |   |   |   +----- 1: flooded
          |   |   |
          |   |   +----- 1: loop center
          |   |             MFSg pixel visited
          |   |
          |   +----- 1: twin loop pixel
          |
          +----- 1: joint(C or B) visited
                    MFSg pixel
*/

// a pixel in CS frame can be one of the following:
#define FOREGRND        0x0         
#define BACKGRND        0x01
#define CONTOUR         0x06
#define FLOODed         0x11
#define LPCENTER        0x21
#define LPCONTOUR       0x08
#define LOOPPIX         0x08
#define TWINLPPIX       0x48


// a pixel in CS frame can have the following attributes. (of course, some
// are mutually exclusive)
                                   // check existence     assign
#define BranchPoint         0x02   //        &               |
#define EndPoint            0x04   //        &               |
#define MidPoint            0xF9   //        |               &
#define CrossPoint          0x06   //        &               |
#define loopPixel           0x08   //        &               |
#define twinLoopPixel       0x40   //        &               |
#define foreGrnd            0xFE   //        |               &
#define jointVisited        0x80   //        &               |
#define MFSgPixel           0x80   //        &               |
#define visitedMFSgPixel    0x20   //        &               |


// clockwise definitions of directions
#define NORTH   0
#define N_EAST  1
#define EAST    2
#define S_EAST  3
#define SOUTH   4
#define S_WEST  5
#define WEST    6 
#define N_WEST  7