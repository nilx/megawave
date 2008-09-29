/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   W_X11R4.h    Window Device for X11 Release 4,5,6 - Include file -
   
   Vers. 3.4
   (C) 1991-2002 Jacques Froment
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
 v3.4: mouse button 4 and 5 added (JF)
----------------------------------------------------------------------*/

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
#define W_MS_UP  13
#define W_MS_DOWN  14
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

/*
 * GLOBAL VARIABLES
 */

/* defined in mw-wdevice_var.c */
extern int             _W_XErrorOccured;
extern Display         *_W_Display;
extern int             _W_Screen;
extern int             _W_Depth;
extern unsigned long   _W_BlackPixel;
extern unsigned long   _W_WhitePixel;
extern Colormap        _W_Colormap;
extern GC              _W_GC;
extern Visual          *_W_Visual;
extern Cursor          _W_Cursor;
extern XFontStruct     *_W_Font; 
extern int             _W_nfcols;
extern unsigned long   _W_freecols[256];
extern int             _W_NumCols;
extern unsigned char   _W_special_color;
extern unsigned char   _W_Red[256],_W_Green[256],_W_Blue[256];
extern unsigned long   _W_cols[256];
extern XColor          _W_RGB[256];
extern int             _W_KeyBuffer; 
