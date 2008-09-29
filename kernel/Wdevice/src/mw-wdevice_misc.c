/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  W_X11R4_misc.c    Window Device for X11 Release 4,5,6
  This file regroups all miscellaneous WX* commands     
  (That is, which should not be used as this by the     
  main functions, but which are called in W_X11R4.c)    
   
  Vers. 3.3
  (C) 1991-2001 Jacques Froment, 
  Simon Masnou     (16 bits plane added)
  Parts of this code inspired from XV: Copyright 1989, 1994 by John Bradley.
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~  This file is part of the MegaWave2 Wdevice library ~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include "libmw-wdevice.h"
#include "mw-wdevice.h"
#include "mw-wdevice_var.h"

#include "mw-wdevice_misc.h"

/*===== For W_X11R4_misc  (internal use) =====*/

typedef struct WWWWW
{ unsigned char r,g,b;
     int oldindex;
     int use; } W_CMAPENT;

#define W_NOPIX 0xffffffff

/*----- General Functions -----*/


int WX_ErrorHandler(Display *display, XErrorEvent *error)
{
     /* FIXME: dummy instruction, unused parameter */
     display = display;
     error = error;

     WDEBUG(WX_ErrorHandler);
     _W_XErrorOccured = 1;
     return 0;
}


/*     Sets up the connection to the X server */
/*     Return -1 if cannot connect */

int WX_Init(char *theDisplayName)
{	
     WDEBUG(WX_Init); 

     if (_W_Display == NULL)
	  _W_Display = XOpenDisplay( theDisplayName );

     if ( _W_Display == NULL )
     {
	  WLIB_ERROR;
	  fprintf( stderr, 
		   "Cannot establish a connection to the X Server %s\n",
		   XDisplayName( theDisplayName ) );
	  return(-1);
     }

#ifdef W_DEBUG_ON
     XSynchronize(_W_Display,True);
     WLIB_ERROR;
     fprintf(stderr,"WX_Init: Debug ON causes synchronized X errors display\n");
  
#endif
  

     /* Default Parameters */
     _W_Screen = DefaultScreen( _W_Display );  
     _W_Depth = DefaultDepth( _W_Display, _W_Screen );
     _W_BlackPixel = BlackPixel( _W_Display, _W_Screen );
     _W_WhitePixel = WhitePixel( _W_Display, _W_Screen );
     _W_Colormap = DefaultColormap( _W_Display, _W_Screen );
     _W_GC = DefaultGC(_W_Display, _W_Screen);
     _W_Visual = DefaultVisual(_W_Display, _W_Screen);

     /* Create a cursor for all the program's windows */
     _W_Cursor     = XCreateFontCursor( _W_Display, XC_crosshair );

     /* Load a font for all the program's windows */

     if ( (_W_Font = XLoadQueryFont(_W_Display,WFONT1))==NULL &&
	  (_W_Font = XLoadQueryFont(_W_Display,WFONT2))==NULL &&
	  (_W_Font = XLoadQueryFont(_W_Display,WFONT3))==NULL )

     {
	  WLIB_ERROR;
	  fprintf( stderr,"Cannot load the following fonts:\n");
	  fprintf( stderr,"\t %s \n\t %s \n\t %s \n",WFONT1,WFONT2,WFONT3);
	  return(-1);
     }

     XSetFont(_W_Display,_W_GC,_W_Font->fid);
  
     return(0);
}

/*                Remove a colormap allocation done using WX_AllocColors  */

void  WX_FreeColors(void)
{
     int i;

     WDEBUG(WX_FreeColors); 

     if (_W_nfcols > 0)
     {
	  for (i=0; i<_W_nfcols; i++)
	       XFreeColors(_W_Display, _W_Colormap, &(_W_freecols[i]), 1, 0L);
	  XFlush(_W_Display);
     }
     _W_NumCols = -1;
     _W_nfcols = 0;
     _W_special_color = 255;
     memset(&(_W_Red[0]),0,sizeof(unsigned char)*256);
     memset(&(_W_Green[0]),0,sizeof(unsigned char)*256);
     memset(&(_W_Blue[0]),0,sizeof(unsigned char)*256);
     memset(&(_W_cols[0]),0,sizeof(unsigned long)*256);
}

/*          Put the desired colormap regarding to _W_Red,_W_Green,_W_Blue */

/*      Code modified from an original code of John Bradley               */
/*      (C) Copyright University of Pennsylvania, USA                     */

void  WX_AllocColors(void)
{
     int      i,j,unique,p2alloc,p3alloc;
     Colormap cmap;
     Status   ret;
     XColor   ctab[256];
     unsigned long pixel, *fcptr;
     int      dc;
     unsigned int  ncells;
     int           fc2pcol[256];  /* maps _W_freecols into pic pixel values */
     int noglob=0; /* force to only use colors it alloced */

     WDEBUG(WX_AllocColors); 

     if ((_W_NumCols <= 1) && (_W_NumCols > 256))
     {
	  WLIB_ERROR;
	  fprintf(stderr,"[WX_AllocColors] Invalid _W_NumCols value = %d\n",
		  _W_NumCols);
	  return;
     }
     if (_W_nfcols > 0) WX_FreeColors(); /* Free previously allocation */

     /* Maximum possible number of colormap cells for this screen */
     ncells    = DisplayCells(_W_Display, _W_Screen);
  
     _W_nfcols = unique = p2alloc = p3alloc = 0;
     for (i=0; i<_W_NumCols; i++) _W_cols[i] = W_NOPIX;

     /* FIRST PASS */

     cmap = _W_Colormap;
     for (i=0; (i<_W_NumCols) && (unique < _W_NumCols) ; i++)
     {
	  _W_RGB[i].red   = (_W_Red[i]<<8);
	  _W_RGB[i].green = (_W_Green[i]<<8);
	  _W_RGB[i].blue  = (_W_Blue[i]<<8);

	  _W_RGB[i].flags = DoRed | DoGreen | DoBlue;
      
	  /* Try to get the right color */
	  ret=XAllocColor(_W_Display, cmap, &(_W_RGB[i]));
	  if (ret) 	
	  {
	       pixel = _W_cols[i] = _W_RGB[i].pixel;
	  
	       /* see if the newly allocated color is new and different */
	       for (j=0, fcptr=_W_freecols; j<_W_nfcols && *fcptr!=pixel; j++,fcptr++);
	       if (j==_W_nfcols) unique++;
	  
	       fc2pcol[_W_nfcols] = i;
	       _W_freecols[_W_nfcols++] = pixel;
	  }
	  /*
	    else
	    If not possible, Wdevice V2 created our own colormap 
	    No longer allowed: what about the index i ??
	  */
     }

     /* SECOND PASS */

     /* read entire colormap (or first 256 entries) into 'ctab' */
     dc = (ncells<256) ? ncells : 256;
     for (i=0; i<dc; i++) ctab[i].pixel = (unsigned long) i;

     XQueryColors(_W_Display, cmap, ctab, dc);
  
     for (i=0; i<_W_NumCols && unique<_W_NumCols; i++)
	  if (_W_cols[i]==W_NOPIX) {  /* an unallocated pixel */
	       int           d, mdist, close;
	       unsigned long ri,gi,bi;

	       mdist = 100000;   close = -1;
	       ri = _W_Red[i];  gi = _W_Green[i];  bi = _W_Blue[i];
      
	       for (j=0; j<dc; j++) 
	       {
		    d = abs(ri - (ctab[j].red>>8)) +
			 abs(gi - (ctab[j].green>>8)) +
			 abs(bi - (ctab[j].blue>>8));
		    if (d<mdist) { mdist=d; close=j; }
	       }
      
	       if (XAllocColor(_W_Display, cmap, &ctab[close])) {
		    memcpy(&(_W_RGB[i]), &ctab[close], sizeof(XColor));
		    _W_cols[i] = ctab[close].pixel;
		    fc2pcol[_W_nfcols] = i;
		    _W_freecols[_W_nfcols++] = _W_cols[i];
		    p2alloc++;
		    unique++;
	       }
	  }
        

     /* THIRD PASS */

     for (i=0; i<_W_NumCols; i++) {
	  if (_W_cols[i] == W_NOPIX) {  /* an unallocated pixel */
	       int           d, k, mdist, close;
	       unsigned long ri,gi,bi;
      
	       mdist = 100000;   close = -1;
	       ri = _W_Red[i];  gi = _W_Green[i];  bi = _W_Blue[i];

	       if (!noglob) {   /* search the entire X colormap */
		    for (j=0; j<dc; j++) 
		    {
			 d = abs(ri - (ctab[j].red>>8)) +
			      abs(gi - (ctab[j].green>>8)) +
			      abs(bi - (ctab[j].blue>>8));
			 if (d<mdist) { mdist=d; close=j; }
		    }
		    memcpy(&(_W_RGB[i]), &ctab[close], sizeof(XColor));
		    _W_cols[i] = _W_RGB[i].pixel;
		    p3alloc++;
	       }
	       else {                     /* only search the alloc'd colors */
		    for (j=0; j<_W_nfcols; j++) {
			 k = fc2pcol[j];
			 d = abs(ri - (_W_RGB[k].red>>8)) +
			      abs(gi - (_W_RGB[k].green>>8)) +
			      abs(bi - (_W_RGB[k].blue>>8));
			 if (d<mdist) { mdist=d;  close=k; }
		    }
		    memcpy(&(_W_RGB[i]), &(_W_RGB[close]), sizeof(XColor));
		    _W_cols[i] = _W_RGB[i].pixel;
	       }
	  }
     }
  
}

/*                  This function called by WX_AllocXImage ONLY */
/*                  Create the right XImage regarding to the screen used */

void WX_CreateXImage(Wframe *window, int dx, int dy)
{
     int sizepix;

     WDEBUG(WX_CreateXImage); 

     switch (_W_Depth)
     {
     case 8:  /* The screen is a 8-bits plane */

	  window->ximage = XCreateImage(_W_Display,_W_Visual,_W_Depth,ZPixmap,0,
					NULL,dx,dy,8,0);
	  sizepix = dx*dy;
	  break;

     case 1:  /* The screen is monochrome */

	  window->ximage = XCreateImage(_W_Display,_W_Visual,_W_Depth,XYPixmap,0,
					NULL,dx,dy,8,0);
	  if (window->ximage != NULL) 
	       sizepix = window->ximage->bytes_per_line * dy;
	  break;

     case 4: case 6:  /* The screen has 16 or 64 levels */

	  window->ximage = XCreateImage(_W_Display,_W_Visual,_W_Depth,ZPixmap,0,
					NULL,dx,dy,8,0);
	  if (window->ximage != NULL) 
	       sizepix = window->ximage->bytes_per_line * dy;
	  break;

     case 16:
	  window->ximage = XCreateImage(_W_Display,_W_Visual,_W_Depth,ZPixmap,0,
					NULL,dx,dy,16,0);
	  if (window->ximage != NULL) sizepix = 2 * dx * dy;
	  break;
      
     case 24: case 32: /* What a nice screen ! */

	  window->ximage = XCreateImage(_W_Display,_W_Visual,_W_Depth,ZPixmap,0,
					NULL,dx,dy,32,0);
	  if (window->ximage != NULL) sizepix = 4 * dx * dy;
	  break;


     default:  /* No code supported */
	  WLIB_ERROR;
	  fprintf(stderr,
		  "[WX_CreateXImage] No code to handle this display of %d bits deep\n",
		  _W_Depth);
	  window->ix = window->iy = sizepix = 0;
	  window->pic = window->pix = NULL;
	  return;
	  break;
     }

     if (window->ximage == NULL)
     {
	  WLIB_ERROR;
	  fprintf(stderr,"[WX_CreateXImage] Cannot create the XImage !\n");
	  window->ix = window->iy = sizepix = 0;
	  window->pic = window->pix = NULL;
	  return;
     }

     /* Allocate *pix */
     window->pix = (unsigned char *) malloc(sizepix);

     if (window->pix == NULL)
     {
	  WLIB_ERROR;
	  fprintf(stderr,
		  "[WX_CreateXImage] Not enough memory to bitmap the window !\n");
	  window->ix = window->iy = sizepix = 0;
	  XDestroyImage(window->ximage);
	  window->pic = NULL;
	  window->ximage = NULL;
	  return;
     }
     memset((char *) window->pix, (int) 0, (int) sizepix);
     window->ximage->data = (char *) window->pix;
     window->ix = dx; window->iy = dy; 

     /* Allocate *pic in case of not 8 bpp screens or set pic to pix
	Note du 15/3/99 : pourquoi seulement dans le cas 8 bpp ? Rajoute 24 et 32
     */
     if ((_W_Depth == 8)||(_W_Depth == 16)||(_W_Depth == 24)||(_W_Depth == 32))
	  window->pic = window->pix;
     else
     {
	  window->pic = (unsigned char *) malloc(dx*dy);

	  if (window->pic == NULL)
	  {
	       WLIB_ERROR;
	       fprintf(stderr,
		       "[WX_CreateXImage] Not enough memory to create *pic !\n");
	       return;
	  }
	  memset((char *) window->pic, (int) 0, (int) dx*dy);
     }
}


/*            - Alloc/Realloc the *ximage and the *pix bitmap of a window */
/*              This is used as a bridge between a bitmap buffer (*pix)   */
/*              and a window or pixmap buffer.                            */

void WX_AllocXImage(Wframe *window, int dx, int dy)
{
     WDEBUG(WX_AllocXImage); 

     if ((window->ximage == NULL) || (window->pix == NULL))
	  window->ix = window->iy = 0; /* For Security */

     if ( (dx != window->ix) || (dy != window->iy) )
	  /* Has to be allocated */
     {
	  if (window->ximage != NULL)
	  { 
	       /* Done by XDestroyImage() 
		  free(window->pix);
		  window->pix = NULL;
	       */
	       if ((_W_Depth != 8)&&(_W_Depth != 16)&&(_W_Depth != 24)&&
		   (_W_Depth != 32))
	       {
		    free(window->pic);
		    window->pic = NULL;
	       }
	       XDestroyImage(window->ximage);
	  }
	  WX_CreateXImage(window, dx, dy);
     }
}


/*             - Alloc/Realloc the pixmap field of a window                  */
/*             This is used as a graphics buffer : indeed the graphic        */
/*             function don't write directly into the window but rather into */
/*             this pixmap. A flush copy the pixmap in the window.           */
/*             This allows to keep in memory the exact representation of the */
/*             window, and then to restore the content after an expose event.*/

void WX_AllocXPixmap(Wframe *window, int dx, int dy)
{
     WDEBUG(WX_AllocXPixmap); 

     if (window->pixmap == (Pixmap) NULL) window->px = window->py = 0; /* For Security */

     if ((dx*dy) > (window->px * window->py)) 
/*  if ( (dx != window->px) || (dy != window->py) ) */
	  /* Has to be allocated */
     {

#ifdef W_DEBUG_ON
	  fprintf(stderr,"\t Try to allocate size (%d,%d)\n",dx,dy);
#endif

	  if (window->pixmap != (Pixmap) NULL) XFreePixmap(_W_Display,window->pixmap);
	  window->pixmap = XCreatePixmap(_W_Display,window->win,dx,dy,_W_Depth);
	  if (window->pixmap == (Pixmap) NULL)
	  {
	       fprintf(stderr,
		       "WX_AllocXPixmap : Not enough memory to pixmap the window !\n");
	       window->px = 0; window->py = 0;
	       return;
	  }

	  window->px = dx; window->py = dy; 
	  XCopyArea(_W_Display,window->win,window->pixmap,_W_GC,0,0,dx,dy,0,0);
     }
}


/* Ditherizing function */
/* This allow to simulate 256 gray levels on a monochrome display */

/* Code modified from an original code of John Bradley */
/* (C) Copyright University of Pennsylvania, USA                     */

void WX_Ditherize(Wframe *window, int dx, int dy)
{

     register short *dp;
     register unsigned char   pix8, bit;
     int white = 255;
     int black = 0;
     short          *dithpic;
     int             i, j, err, bperln, order;
     unsigned char           *pp, *image, w, b, w8, b8;

     WDEBUG(WX_Ditherize);

     if ((window == NULL) || (window->ximage == NULL)) return;

     image  = (unsigned char *) window->ximage->data;
     bperln = window->ximage->bytes_per_line;
     order  = window->ximage->bitmap_bit_order;

     dithpic = (short *) malloc(dx * dy * sizeof(short));
     if (dithpic == NULL) 
     {
	  WLIB_ERROR;
	  fprintf(stderr,"[WX_Ditherize] not enough memory to ditherize");
	  return;
     }

     w = white&0x1;  b=black&0x1;
     w8 = w<<7;  b8 = b<<7;        /* b/w bit in high bit */

     /* Copy bitmap into dithpic */
     pp = window->pic;  dp = dithpic;
     for (i=dx*dy; i>0; i--) *dp++ = 255-_W_Red[*pp++];

     dp = dithpic;
     pp = image;

     for (i=0; i<dy; i++) {
	  pp = image + i*bperln;

	  if (order==LSBFirst) {
	       bit = pix8 = 0;
	       for (j=0; j<dx; j++,dp++) {
		    if (*dp<128) { err = *dp;     pix8 |= b8; }
		    else { err = *dp-255; pix8 |= w8; }

		    if (bit==7) {
			 *pp++ = pix8;  bit=pix8=0;
		    }
		    else { pix8 >>= 1;  bit++; }

		    if (j<dx-1) dp[1] += ((err*7)/16);

		    if (i<dy-1) {
			 dp[dx] += ((err*5)/16);
			 if (j>0)       dp[dx-1] += ((err*3)/16);
			 if (j<dx-1) dp[dx+1] += (err/16);
		    }
	       }
	       if (bit) *pp++ = pix8>>(7-bit);  /* write partial byte at end of line */
	  }

	  else {   /* order==MSBFirst */
	       bit = pix8 = 0;
	       for (j=0; j<dx; j++,dp++) {
		    if (*dp<128) { err = *dp;     pix8 |= b; }
		    else { err = *dp-255; pix8 |= w; }

		    if (bit==7) {
			 *pp++ = pix8;  bit=pix8=0;
		    }
		    else { pix8 <<= 1; bit++; }

		    if (j<dx-1) dp[1] += ((err*7)/16);

		    if (i<dy-1) {
			 dp[dx] += ((err*5)/16);
			 if (j>0)       dp[dx-1] += ((err*3)/16);
			 if (j<dx-1) dp[dx+1] += (err/16);
		    }
	       }
	       if (bit) *pp++ = pix8<<(7-bit);  /* write partial byte at end of line */
	  }
     }

     free(dithpic);
}


/*   Called by WUserEvent: set in _W_KeyBuffer the ascii value of */
/*   the key pressed, or the key symbol in case of non-printable key. */

int WX_KeyPress(XKeyEvent *event)
{
     int             length;
     KeySym          theKeySym;
     XComposeStatus  theComposeStatus;
     char            key;

     /* This test to avoid a memory fault into the function XLookupString() */
     /* (on HP-UX A.09.03 with X11R5), when sometimes a XKeyEvent is received */
     /* by WUserEvent and it does not correspond to a key pressed. In that case,*/
     /* The key ID given by keycode as an enormous (and probably inconsistant) */
     /* value. JF 8/8/94 */

     if (((*event).keycode <= 0) || ((*event).keycode >= 0xFF00)) return(-1);

     length = XLookupString( event,
			     &key,
			     1,
			     &theKeySym,
			     &theComposeStatus );

     if ( ( theKeySym < ' ' )  || /* -- ASCII 32 " "  */
	  ( theKeySym > '~' ) ||  /* -- ASCII 126 "~" */
	  (length <=0) )
	  /* Not a printable character : set the Ascii code */
	  _W_KeyBuffer=theKeySym;
     else
	  _W_KeyBuffer=key;  /* Map Ascii value */
     return(0);
}
