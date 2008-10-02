/**
 * @file wdevice-defs.h
 *
 * structures and declarations for the megawave wdevice library
 *
 * @author John Bradley for XV <xv@trilon.com> (1989 - 1994),		\
 *         Jacques Froment <jacques.froment@univ-ubs.fr> (1991 - 2006), \
 *         Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/*
 * FIXME: non-free origin (see http://www.trilon.com/xv/pricing.html)
 *        replace by a free alternative
 */

#ifndef _WDEVICE_DEFS_H_
#define _WDEVICE_DEFS_H_

#include <X11/Xlib.h>

#include "wdevice-config.h"

/**
 * @brief Wframe : main structure for a window
 *
 * This structure is device-dependent and the fields must NOT
 * be accessed by any program running the Wgraphics library.
 */
typedef struct {
     Window win;               /*< X Window ID                             */
     int x,y;                  /*< Current Location of the Window          */
     int dx,dy;                /*< Current Size of the Window              */
     unsigned char *pix;       /*< BitMap for the Window (client side)     */
                               /*  Format of pixels is screen-dependent    */
     unsigned char *pic;       /*< BitMap - Format of pixels in 8 bits pp  */
                               /*  Used only in case of not 8 bpp screens  */
                               /*  (else pic = pix)                        */
     XImage *ximage;           /*< XImage structure of the Window          */
                               /*  (client side)                           */
     int ix,iy;                /*  Size of the window alloc.               */
                               /*  for ximage and pix                      */
     Pixmap pixmap;            /*< Pixmap structure                        */
                               /*  for buffering the graphics              */
                               /*  (server side)                           */
     int px,py;                /*< Size of the memory allocated for Pixmap */
     unsigned long event_mask; /*< Event Mask for this Window              */
} Wframe;


/*
 * MACROS
 */

/* #define W_DEBUG_ON */
#ifdef W_DEBUG_ON
#define WDEBUG(Function) (fprintf(stderr,"\n>>> Function <<<\n"))
#else
#define WDEBUG(Function)
#endif

#define WLIB_ERROR (fprintf(stderr,"Wdevice Library error: "))

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
extern unsigned long   _W_cols[256];


#endif /* !_WDEVICE_DEFS_H_ */
