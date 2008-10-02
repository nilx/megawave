/*
 * structures and definitions for the megawave wdevice library
 */

/**
 * @brief Wframe : main structure for a window
 *
 * This structure is device-dependent and the fields must NOT
 *          be accessed by any program running the Wgraphics library.
 */

#include <X11/Xlib.h>
#define  XK_MISCELLANY
#include <X11/keysymdef.h>

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


/* #define W_DEBUG_ON */

#ifdef W_DEBUG_ON
#define WDEBUG(Function) (fprintf(stderr,"\n>>> Function <<<\n"))
#else
#define WDEBUG(Function)
#endif

#define WLIB_ERROR (fprintf(stderr,"Wdevice Library error: "))

/*
 * FONTS
 */

#define WFONT1 "-misc-fixed-medium-r-normal-*-13-*"
#define WFONT2 "8x13"
#define WFONT3 "-*-courier-medium-r-*-*-12-*"

/*
 * WINDOW PARAMETERS
 */

#define BORDER_WIDTH  2  /* width of the window border */

/* plot window attributes */
#define PLOT_RES_NAME   "Plot" /* name of the plot window icon           */
#define PLOT_RES_CLASS  "Plot" /* resource class of the plot window icon */
#define PLOT_MIN_WIDTH  50     /* minimum useful size of the plot window */
#define PLOT_MIN_HEIGHT 50
#define PLOT_MAX_WIDTH  2000   /* maximum useful size of the plot Window */
#define PLOT_MAX_HEIGHT 2000

/* image window qttributes */
#define IMAGE_RES_NAME   "View" /* name of the image window icon           */
#define IMAGE_RES_CLASS  "View" /* resource class of the image window icon */
#define IMAGE_MIN_WIDTH  50     /* minimum useful size of the image window */
#define IMAGE_MIN_HEIGHT 50
#define IMAGE_MAX_WIDTH  2000   /* maximum useful size of the image window */
#define IMAGE_MAX_HEIGHT 2000

/*
 * EVENTS
 */

/*
 * Define which system events would be sent to the window.
 * The whole mask is created by a OR with this mask and the user
 * mask
 */ 
#define SYSTEM_EVENT_MASK (ExposureMask			\
			   | EnterWindowMask		\
			   | LeaveWindowMask		\
			   | StructureNotifyMask)

/*
 * This is a list of user events as they can be set by WSetUserEvent()
 * and read with WUserEvent(). This list is a selection of most useful
 * events that are simultaneously defined in all of the various window
 * systems supported by Wdevice.
 */

/* mouse */
#define W_MS_LEFT    10 /* mouse buttons (not a mask) */
#define W_MS_RIGHT   11 
#define W_MS_MIDDLE  12
#define W_MS_UP      13
#define W_MS_DOWN    14
#define W_MS_BUTTON  ButtonPressMask /* mask for button scanning */

/*
 * For keyboard, non-printable characters: see X11 include file
 * keysymdef.h
 */ 

/* window */
#define W_REPAINT  ExposureMask        /* have to repaint the window    */
#define W_RESIZE   ResizeRedirectMask  /* have to resize the window     */
#define W_ENTER    EnterWindowMask     /* mouse enters the window       */
#define W_LEAVE    LeaveWindowMask     /* mouse leaves the window       */
#define W_KEYPRESS KeyPressMask        /* a key has been pressed        */
#define W_DESTROY  StructureNotifyMask /* the window has been destroyed */

/*
 * PENCIL
 */

#define W_COPY GXcopy
#define W_XOR  GXequiv

