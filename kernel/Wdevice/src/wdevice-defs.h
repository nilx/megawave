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
