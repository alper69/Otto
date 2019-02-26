/* 
 *    Copyright (C) 1992-94  A. Alper Atici
 *        Dept. of Computer Engineering, METU
 */

#define MAX_MSGLen      80

struct menuInfo {
        int charHeight;
        int charWidth;
        int fillColor;
        int textColor;
        int borderColor;
        int left, right;
};

struct imageDispInfo {
        unsigned char far *displayBuf;
        int width;
        int depth;
        int panOff;
        int scrollOff;
};
        

// 128-byte standard PCX file header
struct PCXHead {
        char manufacturer;
        char version;
        char encoding;
        char bits;
        int xmin, ymin;
        int xmax, ymax;
        int hres;
        int vres;
        char palette[48];
        char RESERVED;
        char color_planes;
        int bytes_per_line;
        int palette_type;
        char FILLER[58];
};

class FileName {

        int fileExist (char *);

    public:

        FileName (char *, char * = "Enter filename", 
                          char * = "( [!]Dos command, [=]Exit ) : ");
};

class Mouse {

        int initMouse();

    public :

        void showMouseCursor();
        void hideMouseCursor();
        void setGraphMouseYRange(int, int);
        void waitButtonPushGraphMouse(int&, int&);
        int pressedButtonGraphMouse(int&, int&);
        Mouse();
};

class Screen : public Mouse {

        char BgiPath[50];
        int State;
        int msgLineHeight;
        int menuHeight;

        void Init();

    public:

        int mode (int);
        void set_menuHeight (int a) { menuHeight = a; }
        void preFrameCut();
        void postFrameCut();
        void preBaselineGen();
        void postBaselineGen();
        int imageDispParam (struct imageDispInfo *);
        int menuParam (struct menuInfo *);
        int controlScr (int&, int&);
        Screen (char *);
        ~Screen();
};

class CutOutFrame {

    protected:

        int FrameExist;
        int FrLeft, FrTop, FrRight, FrBottom;
        int BaseTop, BaseBottom;

    public :

        CutOutFrame();
        void frameGen (int, int, Screen *);
        void baselineGen (Screen *);
};

class Image : public CutOutFrame {

        int FhImage;
        int Width, Depth;
        int MinDispX, MinDispY, MaxDispX, MaxDispY;
        int ImgRowOff;
        struct imageDispInfo iInfo;

        int decompressPCX (int);
        int Use ();

    public :

        void Show();
        void rightPan();
        void leftPan();
        void scrollUp();
        void scrollDown();
        int frameCutOut (int&, int&, unsigned char huge *&, int&, int&, int);
        void showImgSize (int);
        Image (char *, Screen *);
        ~Image();
};

class Menu {

    protected :

        int MenuHeight;
//      int *CmdXBorders;
//      unsigned char *CmdBoxImgBuf;
        int CmdXBorders[30];
        unsigned char CmdBoxImgBuf[1024];
        int CmdNum;

    public :

        int getMenuHeight() { return (MenuHeight); }
        int menuCommandSelect (int, int, Screen *);
        int Show (char far * far *, Screen *);
        Menu (int, char far * far *, Screen *);
//      ~Menu();
};

class DumbMenu : public Menu {

        int lastSelected;

    public :

        int menuCommandSelect (int, int, Screen *, int&);
        DumbMenu (int, char far * far *, Screen *);
};

class Message {

        int MsgLen;
        int MsgExist;
        char MsgString[MAX_MSGLen];

    public :
        
        Message();
        void Show (char *, int);
        void Clear();
        int height();
        void getInp (char *, char *);
};

class Heap {

    public :

        void far *alloc (long);
        int free (void far *);
        long avail ();
};
