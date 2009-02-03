/**
 * @file definitions.h
 *
 * structures and declarations for the megawave wpanel library
 *
 * @author John Bradley for XV <xv@trilon.com> (1989 - 1994),
 *         Jacques Froment <jacques.froment@univ-ubs.fr> (1991 - 2006),
 *         Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 *
 * @todo non-free origin (see http://www.trilon.com/xv/pricing.html),
 *       replace by a free alternative 
 */

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <X11/Xlib.h>
#define  XK_MISCELLANY
#include <X11/keysymdef.h>

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


#define Wp_max_buttons 100  /* maximum number of buttons */
#define WP_STRSIZE 1000  /* maximum string size */

/* colors 64 grey levels + 5x5x5 */

#define WP_BLACK    0
#define WP_GREY    40
#define WP_WHITE   63
#define WP_RED    164
#define WP_BLUE    68
#define WP_GREEN   84


/* wp types */

#define WP_NULL    0
#define WP_TOGGLE  1
#define WP_INT     2
#define WP_FLOAT   3


typedef struct wp_toggle {
  char *text;          /* text to display */
  int color;           /* active color */
  short nbuttons;      /* number of buttons */
  short button;        /* current active button */
  char **button_text;  /* text for each button */
  int x,y ;            /* position on window (upleft corner) */
  int (*proc)(struct wp_toggle *, int); 
                       /* function to call when value changes */
                       /* (may be NULL) */
} *Wp_toggle;

typedef struct wp_int {
  char *text;          /* text to display */
  char *format;        /* format for int display (eg "%d") */
  int value;           /* value */
  int strsize;         /* internal use (initialize to 0) */
  int scale;           /* length of scale bar (0 means no bar) */
  int firstscale;      /* value of bar left border */
  int lastscale;       /* value of bar right border */
  int divscale;        /* number of bar scale divisions */
  int color;           /* text color */
  short nbuttons;      /* number of buttons */
  char **button_text;  /* text for each button */
  int *button_inc;     /* increment for each button */
  int x,y ;            /* position on window (upleft corner) */
  int (*proc)(struct wp_int *, int); 
                       /* function to call when value changes */
                       /* (may be NULL) */
} *Wp_int;

typedef struct wp_float {
  char *text;          /* text to display */
  char *format;        /* format for int display (eg "%d") */
  float value;         /* value */
  int strsize;         /* internal use (initialize to 0) */
  int color;           /* text color */
  short nbuttons;      /* number of buttons */
  char **button_text;  /* text for each button */
  float *button_inc;   /* increment for each button */
  int x,y ;            /* position on window (upleft corner) */
  int (*proc)(struct wp_float *, int); 
                       /* function to call when value changes */
                       /* (may be NULL) */
} *Wp_float;

typedef struct wpanel {
  Wframe *window;        /* attached window */
  char state;            /* -1 means that window should be closed */
  int nx,ny;             /* size of bitmaps (initial window size) */
  char *type;            /* bitmap (associated wp type) */
  void **action;         /* bitmap (pointer to wp structure) */
  short *button;         /* bitmap (associated button number) */
} *Wpanel;

/* TODO: drop? */
extern int mwwindelay;
extern int mwrunmode;

#endif /* !_DEFINITIONS_H_ */

