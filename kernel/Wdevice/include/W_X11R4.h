/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   W_X11R4.h    Window Device for X11 Release 4,5,6 - Include file -
   
   Vers. 3.3
   (C) 1991-2001 Jacques Froment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~  This file is part of the MegaWave2 Wdevice library ~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*----------------------------------------------------------------------
 v3.3: increased  PLOT/IMAGE_MAX_WIDTH/HEIGHT (L.Moisan)
----------------------------------------------------------------------*/

/* X11 Include Files */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

/* To get key symb (non-printable keys) */
#define  XK_MISCELLANY
#include <X11/keysymdef.h>

/* Wframe : main structure for a window */
/*          This structure is device-dependent and the fields must NOT */
/*          be accessed by any program running the Wgraphics library.  */

typedef struct {

  Window win;               /* X Window ID */
  int x,y;                  /* Current Location of the Window */
  int dx,dy;                /* Current Size of the Window */
  unsigned char *pix;       /* BitMap for the Window (client side) */
                            /* Format of pixels is screen-dependent */
  unsigned char *pic;       /* BitMap - Format of pixels in 8 bits pp */
                            /* Used only in case of not 8 bpp screens */
                            /* (else pic = pix) */
  XImage *ximage;           /* XImage structure of the Window (client side) */
  int ix,iy;                /* Size of the window alloc. for ximage and pix */
  Pixmap pixmap;            /* Pixmap structure for buffering the graphics */
                            /* (server side) */
  int px,py;                /* Size of the memory allocated for Pixmap */
  unsigned long event_mask; /* Event Mask for this Window */

} Wframe;



/* Define W_DEBUG_ON in order to get a debug */

/*
 #define W_DEBUG_ON
*/

#ifdef W_DEBUG_ON
#define WDEBUG(Function) (fprintf(stderr,"\n>>> Function <<<\n"))
#else
#define WDEBUG(Function)
#endif

#define WLIB_ERROR (fprintf(stderr,"Wdevice Library error: "))


/*===== The Fonts =====*/

/* 
  #define WFONT1 "-*-lucida-medium-r-*-*-12-*"
  #define WFONT2 "-*-helvetica-medium-r-*-*-12-*"
  #define WFONT3 "6x13"
*/

#define WFONT1 "-misc-fixed-medium-r-normal-*-13-*"
#define WFONT2 "8x13"
#define WFONT3 "-*-courier-medium-r-*-*-12-*"


/*===== Window parameters =====*/

#define BORDER_WIDTH  2  /* Width of the window's border */

/* Plot Window Attributes */

#define PLOT_RES_NAME  "Plot" /* Name of the Plot window's Icon */
#define PLOT_RES_CLASS "Plot" /* Resource class of the Plot window's Icon */

#define PLOT_MIN_WIDTH 50     /* Minimum useful size of the Plot Window */
#define PLOT_MIN_HEIGHT 50

#define PLOT_MAX_WIDTH 2000    /* Maximum useful size of the Plot Window */
#define PLOT_MAX_HEIGHT 2000

/* Image Window Attributes */

#define IMAGE_RES_NAME  "View" /* Name of the Image window's Icon */
#define IMAGE_RES_CLASS "View" /* Resource class of the Image window's Icon */

#define IMAGE_MIN_WIDTH 50     /* Minimum useful size of the Image Window */
#define IMAGE_MIN_HEIGHT 50

#define IMAGE_MAX_WIDTH 2000    /* Maximum useful size of the Image Window */
#define IMAGE_MAX_HEIGHT 2000

/*===== Events =====*/

/* Define which System events would be sent to the Window */
/* The whole mask is created by a OR with this mask and the user's mask */

#define SYSTEM_EVENT_MASK (ExposureMask | EnterWindowMask | LeaveWindowMask | \
			   StructureNotifyMask)

/* This is a list of users events as they can be set by WSetUserEvent() and  */
/* read with WUserEvent(). This list is a selection of most useful events    */
/* that are simultaneously defined in all of the various window systems      */
/* supported by Wdevice.                                                     */

/* Mouse */

#define W_MS_LEFT    10 /* Mouse buttons (not a mask) */
#define W_MS_RIGHT   11 
#define W_MS_MIDDLE  12
#define W_MS_BUTTON  ButtonPressMask /* Mask for button scanning */

/* For keyboard, non-printable characters: see X11 include file keysymdef.h */

/* Window */

#define W_REPAINT  ExposureMask /* Have to repaint the window (Expose...) */
#define W_RESIZE   ResizeRedirectMask  /* Have to resize the window */
#define W_ENTER    EnterWindowMask     /* Mouse enters the window */
#define W_LEAVE    LeaveWindowMask     /* Mouse leaves the window */
#define W_KEYPRESS KeyPressMask        /* A key has been pressed */
#define W_DESTROY  StructureNotifyMask /* The window has been destroyed */

/*===== Types of Pencil  =====*/

#define W_COPY GXcopy
#define W_XOR  GXequiv  /* GXxor seems wrong... Why ? */


/*===== Function definitions  =====*/

#ifdef __STDC__

int WIsAnActiveWindow(Wframe *);
void WSetColorMap();
void WFlushWindow(Wframe *);
void WFlushAreaWindow(Wframe *,int,int,int,int);
int WColorsAvailable();
void WSetColorPencil(Wframe *,int);
void WSetForegroundColorPencil(Wframe *);
void WSetBackgroundColorPencil(Wframe *);
void WSetSpecialColorPencil(Wframe *);
void WSetTypePencil(int);
void WDrawPoint(Wframe *,int,int);
void WDrawLine(Wframe *,int,int,int,int);
void WDrawString(Wframe *,int,int,char *);
void WDrawRectangle(Wframe *,int,int,int,int);
void WFillRectangle(Wframe *,int,int,int,int);
void WClearWindow(Wframe *);
void WDestroyWdeviceWindow(Wframe *);
void WDestroyWindow(Wframe *);
void WMoveWindow(Wframe *,int,int);
void WPutTitleWindow(Wframe *,char *);
void WSaveImageWindow(Wframe *,int,int,int,int);
void WRestoreImageWindow(Wframe *,int,int,int,int);
int WLoadBitMapImage(Wframe *,unsigned char *,int,int);
int WLoadBitMapColorImage(Wframe *,unsigned char *,unsigned char *,
			  unsigned char *,int,int);
void WSystemEvent(Wframe *);
void WSetUserEvent(Wframe *,unsigned long);
int WUserEvent(Wframe *);
int WGetStateMouse(Wframe *,int *,int *,int *);
int WGetKeyboard(void);
Wframe *WNewImageWindow();
Wframe *WOpenImageWindow(int,int,int,int,char *);
void WReOpenImageWindow(Wframe *,int,int,int,int,char *);

#else

int WIsAnActiveWindow();
void WSetColorMap();
void WFlushWindow();
void WFlushAreaWindow();
int WColorsAvailable();
void WSetColorPencil();
void WSetForegroundColorPencil();
void WSetBackgroundColorPencil();
void WSetSpecialColorPencil();
void WSetTypePencil();
void WDrawPoint();
void WDrawLine();
void WDrawString();
void WDrawRectangle();
void WFillRectangle();
void WClearWindow();
void WDestroyWdeviceWindow();
void WDestroyWindow();
void WMoveWindow();
void WPutTitleWindow();
void WSaveImageWindow();
void WRestoreImageWindow();
int WLoadBitMapImage();
int WLoadBitMapColorImage();
void WSystemEvent();
void WSetUserEvent();
int WUserEvent();
int WGetStateMouse();
int WGetKeyboard();
Wframe *WNewImageWindow();
Wframe *WOpenImageWindow();
void WReOpenImageWindow();

#endif


