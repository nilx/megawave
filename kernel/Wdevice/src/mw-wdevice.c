/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  W_X11R4.c    Window Device for X11 Release 4,5,6
   
  Vers. 2.9
  Initial release from Jacques Froment
  Parts of this code inspired from XV: Copyright 1989, 1994 by John Bradley.
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~  This file is part of the MegaWave2 Wdevice library ~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*----------------------------------------------------------------------
  History
  v???: 16 bits plane added (Simon Masnou)
  v2.7: corrections WFlushAreaWindow and WRestoreImageWindow (L.Moisan)
  v2.8: mouse button 4 and 5 added (JF)
  v2.9: added include <string.h> (for Linux 2.6.12 & gcc 4.0.2) (JF)
  ----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include <string.h>

/* global header */
#include "libmw-wdevice.h"
/* variables */
#include "mw-wdevice_var.h"
/* miscellaneous functions */
#include "mw-wdevice_misc.h"
/* icon */
#include "mw-wdevice_icon.h"

/* self */
#include "mw-wdevice.h"

/*           Return 1 if the window is active, 0 elsewhere */

int WIsAnActiveWindow(Wframe *window)
{
     XTextProperty text_prop;

     WDEBUG(WIsAnActiveWindow);

     _W_XErrorOccured = 0;

     XSetErrorHandler(WX_ErrorHandler);

     /* Call a X function which generates BadWindow error in case of non-window */
     XGetWMClientMachine(_W_Display,window->win,&text_prop);

     XSetErrorHandler(NULL);

     return(1-_W_XErrorOccured);
}
			   

/*              Initialize the ColorMap */

void WSetColorMap(void)
{
     int i,r,g,b;
     int NumColors=190; /* Total number of colors (64 + 5x5x5 + 1) */
     int NCGray=64;  /* Number of different gray levels */

     WDEBUG(WSetColorMap);

  
     if (_W_NumCols > 0 )
     {
	  WLIB_ERROR;
	  fprintf(stderr,
		  "[WSetColorMap] ColorMap already allocated ! (Num.Cols=%d)\n",
		  _W_NumCols);
	  return;
     }

     _W_NumCols = NumColors;

     /* Set the gray levels */
     for (i=0;i<NCGray;i++) 
	  _W_Red[i]=_W_Green[i]=_W_Blue[i] = (256*i)/NCGray; 

     /* Set the color levels */
     for (r=0, i=NCGray; r<5; r++)
	  for (g=0; g<5; g++)
	       for (b=0; b<5; b++,i++) 
	       {
		    _W_Red[i] = (r*255)/4;
		    _W_Green[i] = (g*255)/4;
		    _W_Blue[i] = (b*255)/4;
	       }

     /* Set the special color */
     _W_Red[i] = 255;           /* a kind of red color which is different */
     _W_Green[i] = 28;          /* to the ones you can get in the previous */
     _W_Blue[i] = 84;           /* colormap. */
     _W_special_color = i;

     WX_AllocColors(); /* ColorMap allocation */

}


/*           Flush the Window (Show the window has it is) */

void WFlushWindow(Wframe *window)
{
     WDEBUG(WFlushWindow); 

     XCopyArea(_W_Display,window->pixmap,window->win,_W_GC,0,0,window->px,
	       window->py,0,0);
     XFlush(_W_Display); 
}

/*           Flush an area of the Window */

void WFlushAreaWindow(Wframe *window, int x0, int y0, int x1, int y1)

               
                  /* Rectangle to be flushed */

{ int dx,dy;

     WDEBUG(WFlushAreaWindow); 

     if (x0 < 0) x0 = 0;
     if (y0 < 0) y0 = 0;
     if (x1 >= window->px) x1 = window->px-1;
     if (y1 >= window->py) y1 = window->py-1;  

     dx = x1-x0+1; dy = y1-y0+1;
     if ((dx <= 0) || (dy <= 0))
     {
	  WLIB_ERROR;
	  fprintf(stderr,"[WFlushAreaWindow] Bad area size\n");
	  return;
     }
     if (dx > window->px)  dx = window->px;
     if (dy > window->py)  dy = window->py;

     /* modification Lionel Moisan (jan 2001) */
     /*XCopyArea(_W_Display,window->pixmap,window->win,_W_GC,0,0,dx,dy,x0,y0);*/
     XCopyArea(_W_Display,window->pixmap,window->win,_W_GC,x0,y0,dx,dy,x0,y0);
     XFlush(_W_Display);
}


/*                  Return the number of colors available on the display */
int WColorsAvailable(void)
{
     WDEBUG(WColorsAvailable); 

     return(1 << _W_Depth);
}


/*        Set the color of the Pencil (foreground color)             */
/*        color means the index of the table _W_Red[], _W_Green[], _W_Blue[] */

void WSetColorPencil(Wframe *window, int color)
{
     WDEBUG(WSetColorPencil); 

     if (_W_NumCols <=0) WSetColorMap();
     if ((color < 0) || (color >= _W_NumCols)) 
     {
	  WLIB_ERROR;
	  fprintf(stderr,
		  "[WSetColorPencil] color #%d is not in the allowed range #0...#%d\n",color,_W_NumCols-1);
	  return;
     }

     XSetForeground(_W_Display,_W_GC,_W_RGB[color].pixel);
}


/*        Set the color of the Pencil to be the foreground color       */
/*        which is the first allocated color 0.                        */

void WSetForegroundColorPencil(Wframe *window)
{
     WDEBUG(WSetForegroundColorPencil); 

     if (_W_NumCols <=0) WSetColorMap();
     XSetForeground(_W_Display,_W_GC,_W_RGB[0].pixel);
}

/*        Set the color of the Pencil to be the background color       */
/*        which is the last allocated color _W_NumCols-1.       */

void WSetBackgroundColorPencil(Wframe *window)
{
     WDEBUG(WSetBackgroundColorPencil); 

     if (_W_NumCols <=0) WSetColorMap();
     XSetForeground(_W_Display,_W_GC,_W_RGB[_W_NumCols-1].pixel);
}

/*        Set the special color for the Pencil                         */
/*        The special color is a color outside the range 0..._W_NumCols */
/*        and it is used as a special mark on drawing (text,...)       */

void WSetSpecialColorPencil(Wframe *window)
{
     WDEBUG(WSetSpecialColorPencil); 

     if (_W_NumCols <=0) WSetColorMap();  
     XSetForeground(_W_Display,_W_GC,_W_RGB[_W_special_color].pixel);
}


/*        Set the bitwise logical operation for graphic functions */

void WSetTypePencil(int opt)
{
     WDEBUG(WSetTypePencil);
  
     XSetFunction(_W_Display,_W_GC,opt);
}


/*        Draw a point (x,y) in the current Pencil color */

void WDrawPoint(Wframe *window, int x, int y)
{
     WDEBUG(WDrawPoint); 

     if (_W_NumCols <=0) WSetColorMap();
     XDrawPoint(_W_Display,window->pixmap,_W_GC,x,y);
}


/*        Draw a line (x0,y0)-(x1,y1) in the current Pencil color */

void WDrawLine(Wframe *window, int x0, int y0, int x1, int y1)
{
     WDEBUG(WDrawLine); 

     if (_W_NumCols <=0) WSetColorMap();
     XDrawLine(_W_Display,window->pixmap,_W_GC,x0,y0,x1,y1);
}


/*         Draw a string into the window and at the location (x,y) */

void WDrawString(Wframe *window, int x, int y, char *text)
{
     WDEBUG(WDrawString); 

     if (_W_NumCols <=0) WSetColorMap();
     XDrawImageString(_W_Display,window->pixmap,_W_GC,x,y,text,strlen(text));
}


/*        Draw a rectangle (x0,y0)-(x1,y1) in the current Pencil color */

void WDrawRectangle(Wframe *window, int x0, int y0, int x1, int y1)
{
     WDEBUG(WDrawRectangle); 

     if (_W_NumCols <=0) WSetColorMap();
     XDrawRectangle(_W_Display,window->pixmap,_W_GC,x0,y0,x1-x0,y1-y0);
}

/*        Fill a rectangle (x0,y0)-(x1,y1) in the current Pencil color */

void WFillRectangle(Wframe *window, int x0, int y0, int x1, int y1)
{
     WDEBUG(WFillRectangle); 

     if (_W_NumCols <=0) WSetColorMap();
     XFillRectangle(_W_Display,window->pixmap,_W_GC,x0,y0,x1-x0+1,y1-y0+1);
}


/*          Clear the entire window */
void WClearWindow(Wframe *window)
{
     WDEBUG(WClearWindow); 
     XClearWindow(_W_Display,window->win);
     XCopyArea(_W_Display,window->win,window->pixmap,_W_GC,0,0,window->px,
	       window->py,0,0);
}

/*          Destroy the Wdevice window but not the X window ID */
void WDestroyWdeviceWindow(Wframe *window)
{
     WDEBUG(WDestroyWdeviceWindow); 

     /* if (_W_nfcols > 0) WX_FreeColors(); */ /* Not in multi-windows mode ! */
     if (window->ximage != NULL) XDestroyImage(window->ximage);
     if (window->pixmap != (Pixmap) NULL) XFreePixmap(_W_Display,window->pixmap);
     free(window);
     window = NULL;

}

/*          Destroy the window */
void WDestroyWindow(Wframe *window)
{
     WDEBUG(WDestroyWindow); 
     /* if (_W_nfcols > 0) WX_FreeColors(); */ /* Not in multi-windows mode ! */
     /*if (window->ximage != NULL) XDestroyImage(window->ximage);
       Seems to be destroyed by XDestroyWindow() */
     if (window->pixmap != (Pixmap) NULL) XFreePixmap(_W_Display,window->pixmap);
     if (window->win != 0) XDestroyWindow(_W_Display,window->win);
     window->pixmap = 0;
     window->win = 0;
     /* 
	free(window);
	Do not free window so that we can test if it has been deleted or not. 
     */
}


/*                 Move a window to position (x,y) */

void WMoveWindow(Wframe *window, int x, int y)
{
     WDEBUG(WMoveWindow); 
   
     XMoveWindow(_W_Display,window->win,x,y);
}


/*                 Put a title in a window */

void WPutTitleWindow(Wframe *window, char *title)
{
     WDEBUG(WPutTitleWindow); 
     XStoreName( _W_Display, window->win, title );
}


/*----- Image Functions -----*/


/*                   Save the bitmap of the  window in internal memory */
/*                   (x,y) is the upper-left corner of the rectangle to be   */
/*                   saved and (width,height) is this dimension.             */

void WSaveImageWindow(Wframe *window, int x, int y, int width, int height)
{
     WDEBUG(WSaveImageWindow); 
  
     if ( (x+width) > window->dx ) width = window->dx-x;
     if ( (y+height) > window->dy ) height = window->dy-y;
     if ((width <=0 ) || (height <= 0))
     {
	  WLIB_ERROR;
	  fprintf(stderr,"[WSaveImageWindow] Illegal Dimensions (out of frame)\n");
	  return;
     }

     if ((width*height) > (window->ix * window->iy))  
	  /* ximage has to be allocated */
	  WX_AllocXImage(window,width,height);

     if ( ((width*height) <= (window->ix * window->iy)) &&
	  (window->ximage != NULL) )
	  window->ximage = XGetImage(_W_Display,window->pixmap,
				     x,y,width,height,AllPlanes,ZPixmap);
     else window->ximage = NULL;
}

/*                   Restore the bitmap of the window from internal memory  */
/*                   if not possible, clear it.                             */
/*                   (x,y) is the upper-left corner where the rectangle has */
/*                   to be restored and (width,height) is the dimension.    */

void WRestoreImageWindow(Wframe *window, int x, int y, int width, int height)
{
     XImage *xi;

     WDEBUG(WRestoreImageWindow); 

     if ( (x+width) > window->px ) width = window->px-x;
     if ( (y+height) > window->py ) height = window->py-y;

     if ((width <=0 ) || (height <= 0))
     {
	  WLIB_ERROR;
	  fprintf(stderr,"[WRestoreImageWindow] Illegal Dimensions (out of frame)\n");
	  return;
     }
    
     if (window->ximage != NULL) 
	  /* modification Lionel Moisan (fev 2001) */
	  /*XPutImage(_W_Display,window->pixmap,_W_GC,window->ximage,0,0,x,y,width,height);*/
	  XPutImage(_W_Display,window->pixmap,_W_GC,window->ximage,x,y,x,y,width,height);
     else 
	  WClearWindow(window);
}

/*                   Load a BitMap buffer representing an image into  */
/*                   the internal memory.                             */
/*                   MUST BE CALLED after a WInit???ColorMap          */
/*                   (It makes a new colormap)                        */
/*                   Return -1 if something wrong, 0 if OK            */

int WLoadBitMapImage(Wframe *window, unsigned char *bitmap, 
		     int width, int height)
{
     register unsigned char *p,*q;
     register int i;
     register unsigned short t;

     WDEBUG(WLoadBitMapImage); 

     if (_W_NumCols <=0) WSetColorMap();

     /* ximage may have to be allocated */
     WX_AllocXImage(window,width,height);

     if (window->pic == NULL)
     {
	  WLIB_ERROR;
	  fprintf(stderr,"[WLoadBitMapImage] cannot load bitmap (pic is NULL)\n");
	  return(-1);
     }

     switch (_W_Depth)
     {
     case 8: /* Good case : bitmap has the same structure than pix */
	  /* Copy bitmap into window->pic with Quantization of the picture */
	  /* >> 2 because 64 gray levels set in the colormap */
	  for (i=width*height, p=window->pic, q=bitmap; i; i--, p++, q++) 
	       *p =  (unsigned char) _W_cols[(*q) >> 2];
	  break;

     case 1: /* Black & White Screen : have to ditherize */
	  for (i=width*height, p=window->pic, q=bitmap; i; i--, p++, q++) 
	       *p = (*q) >> 2;  /* It is necessary here ? Should be checked ! */
	  WX_Ditherize(window,width,height);
	  break;

     case 16: /* thresholding */
	  for (i=width*height, p=window->pic, q=bitmap; i; i--, q++)
	  {
	       t=(((*q)<<8) & 0xf800) | (((*q)<<3) & 0x07e0) | (((*q)>>3)&0x001f);
	       *p++=t-((t>>8)<<8);
	       *p++=t>>8;
	  }
	  break;

     case 24: case 32: /* True colors */
	  for (i=width*height, p=window->pic, q=bitmap; i; i--, q++)
	  {
	       *(p++) = *q; *(p++) = *q; *(p++) = *q; 
	       /* if (_W_Depth==32) */ *(p++) = *q;
	  }
	  break;

     default:
	  WLIB_ERROR;
	  fprintf(stderr,
		  "[WLoadBitMapImage] No code implemented for this screen\n");
	  return(-1);
	  break;
     }
     return(0);
}

static int highbit(long unsigned int ul)
{
     /* returns position of highest set bit in 'ul' as an integer (0-31),
	or -1 if none */

     int i;  unsigned long hb;
     hb = 0x8000;  hb = (hb<<16);  /* hb = 0x80000000UL */
     for (i=31; ((ul & hb) == 0) && i>=0;  i--, ul<<=1);
     return i;
}




/*                   Load a BitMap buffer representing a color image  */
/*                   (3 planes of 8 bits) into the internal memory.   */
/*                   Make the conversion 24 bits -> 8 bits            */
/*                   MUST BE CALLED after a WInit???ColorMap          */
/*                   (It makes a new colormap)                        */
/*                   Return -1 if something wrong, 0 if OK            */

int WLoadBitMapColorImage(Wframe *window, 
			  unsigned char *Red, 
			  unsigned char *Green, 
			  unsigned char *Blue, int width, int height)
{
     unsigned char *R,*G,*B,*p;
     int i;
     unsigned char *bitmap;

     /* For TrueColor */
     unsigned long rmask, gmask, bmask, xcol;
     int           rshift, gshift, bshift, border, cshift;
     int           maplen;
     unsigned long r,g,b; 

     if (_W_NumCols <=0) WSetColorMap();

     /* ximage may have to be allocated */
     WX_AllocXImage(window,width,height);

     if (window->pic == NULL)
     {
	  WLIB_ERROR;
	  fprintf(stderr,"[WLoadBitMapColorImage] cannot load bitmap (pic is NULL)\n");
	  return(-1);
     }

     switch (_W_Depth)
     {
     case 8: /* Good case : bitmap has the same structure than pix */
	  /* because For 5x5x5 colormap */
	  for (i=0, R=Red, G=Green, B=Blue, p=window->pic;
	       i < width*height; i++, R++, G++, B++, p++) 
	       *p = (*B)/56 + 5*((*G)/56) + 25*((*R)/56) + 64;
	  for (i=width*height, p=window->pic; i>0; i--,p++) 
	       *p = (unsigned char) _W_cols[*p];
      
	  break;

     case 1: /* Black & White Screen : have to ditherize */
	  WX_Ditherize(window,width,height);
	  break;

     case 16:
	  rmask = _W_Visual->red_mask;
	  gmask = _W_Visual->green_mask;
	  bmask = _W_Visual->blue_mask;
	  border   = window->ximage->byte_order;
	  rshift = 7 - highbit(rmask);
	  gshift = 7 - highbit(gmask);
	  bshift = 7 - highbit(bmask);
	  maplen = _W_Visual->map_entries;
	  if (maplen>256) maplen=256;
	  cshift = 7 - highbit((unsigned long) (maplen-1));
	  for (i=0, R=Red, G=Green, B=Blue, p=window->pic;
	       i < width*height; i++, R++, G++, B++)
	  {
	       r=*R; g=*G; b=*B;
	       if (rshift<0) r = r << (-rshift);
	       else r = r >> rshift;
	       if (gshift<0) g = g << (-gshift);
	       else g = g >> gshift;
	       if (bshift<0) b = b << (-bshift);
	       else b = b >> bshift;
	  
	       r = r & rmask;
	       g = g & gmask;
	       b = b & bmask;
	  
	       xcol = r | g | b;
	  
	       if (border == MSBFirst) {
		    *p++ = (xcol>>8)  & 0xff;
		    *p++ =  xcol      & 0xff;
	       }
	       else {  /* LSBFirst */
		    *p++ =  xcol      & 0xff;
		    *p++ = (xcol>>8)  & 0xff;
	       }
	  }
	  break;
      
     case 24: case 32: /* True colors */ 
	  rmask = _W_Visual->red_mask;
	  gmask = _W_Visual->green_mask;
	  bmask = _W_Visual->blue_mask;
	  border   = window->ximage->byte_order;
	  rshift = 7 - highbit(rmask);
	  gshift = 7 - highbit(gmask);
	  bshift = 7 - highbit(bmask);
	  maplen = _W_Visual->map_entries;
	  if (maplen>256) maplen=256;
	  cshift = 7 - highbit((unsigned long) (maplen-1));
	  for (i=0, R=Red, G=Green, B=Blue, p=window->pic;
	       i < width*height; i++, R++, G++, B++) 
	  {
	       r=*R; g=*G; b=*B;
	       if (rshift<0) r = r << (-rshift);
	       else r = r >> rshift;
	       if (gshift<0) g = g << (-gshift);
	       else g = g >> gshift;
	       if (bshift<0) b = b << (-bshift);
	       else b = b >> bshift;

	       r = r & rmask;
	       g = g & gmask;
	       b = b & bmask;

	       xcol = r | g | b;

	       if (border == MSBFirst)
	       {
		    /* if (_W_Depth==32) */
		    *p++ = (xcol>>24) & 0xff;
		    *p++ = (xcol>>16) & 0xff;
		    *p++ = (xcol>>8)  & 0xff;
		    *p++ =  xcol      & 0xff;
	       }
	       else 
		    /* LSBFirst */
	       {  
		    *p++ =  xcol      & 0xff;
		    *p++ = (xcol>>8)  & 0xff;
		    *p++ = (xcol>>16) & 0xff;
		    /* if (_W_Depth==32) */
		    *p++ = (xcol>>24) & 0xff; 
	       }
	  }
	  break;

     default:
	  WLIB_ERROR;
	  fprintf(stderr,
		  "[WLoadBitMapColorImage] No code implemented for this screen\n");
	  return(-1);
	  break;
     }
     return(0);
}

/*----- Event Functions -----*/


/*          Manage the system event of a window. This function should be */
/*          called as often as the user wants a "good" looking of them.  */
/*          For example, it manages the automatic redraw of the window   */
/*          if this one has been altered by an event (as Expose...).     */

void WSystemEvent(Wframe *window)
{
     XEvent event;
     XExposeEvent *exp_event;
     XConfigureEvent *conf_event;

     if (XPending(_W_Display) == 0) return; /* No event in queue */
     XCheckWindowEvent(_W_Display,window->win,SYSTEM_EVENT_MASK,&event);
  
     switch(event.type)
     {
     case GraphicsExpose: case Expose: case NoExpose:
	  exp_event = (XExposeEvent *) &event;
	  if (exp_event->count == 0) { /* This is the last Expose event */
	       WFlushWindow(window);    /* Then restore the content of the window */
#ifdef W_DEBUG_ON
	       fprintf(stderr,"\nWSystemEvent : Expose Event (WFlushWindow done)\n");
	       fprintf(stderr,"               Window ID : %d\n",(int)window->win);
	       fprintf(stderr,"               Event Window ID : %d\n",
		       (int)exp_event->window);
#endif
	  }
	  break;

     case EnterNotify: break;    
     case LeaveNotify: break;    

     case ConfigureNotify : 
	  conf_event = (XConfigureEvent *) &event;
	  if (window->win != conf_event->window) break;
	  /* Set new location */
	  window->x = conf_event->x+1;
	  window->y = conf_event->y+1;
	  if  ((window->dx != conf_event->width) ||
	       (window->dy != conf_event->height))
	  {                           /* Window's size has changed */

#ifdef W_DEBUG_ON
	       fprintf(stderr,"\nWSystemEvent : Current window size = (%d,%d)\n",
		       window->dx,window->dy);
	       fprintf(stderr,"               New window size = (%d,%d)\n",
		       conf_event->width,conf_event->height);
	       fprintf(stderr,"               Window ID : %d\n",(int)window->win);
	       fprintf(stderr,"               Event Window ID : %d\n",
		       (int)conf_event->window);


#endif

	       WFlushWindow(window);
	       window->dx = conf_event->width; 
	       window->dy = conf_event->height;
	       if (window->ximage != NULL) 
		    WX_AllocXImage(window,window->dx,window->dy);
	       if (window->pixmap != (Pixmap) NULL) 
		    WX_AllocXPixmap(window,window->dx,window->dy);
	  }
	  break;

     default: return; break;
     }
}
  
/*                Define the user event that would be returned by WUserEvent */

void WSetUserEvent(Wframe *window, long unsigned int user_event_mask)
{

     WDEBUG(WSetUserEvent); 
     _W_KeyBuffer = 0; /* Init KeyBuffer */
     window->event_mask = (SYSTEM_EVENT_MASK | user_event_mask); 
     XSelectInput(_W_Display,window->win,window->event_mask);
     XSetGraphicsExposures(_W_Display,_W_GC,False);
}

/*            Return an event number defined by the user - or -         */
/*            it returns 0 if no event available for this window - or - */ 
/*            it returns -1 if the user does not care about this event  */

int WUserEvent(Wframe *window)
{
     XEvent event;
     XExposeEvent *exp_event;
     XConfigureEvent *conf_event;

     if (XPending(_W_Display) == 0) return(0); /* No event in queue */
     if (XCheckWindowEvent(_W_Display,window->win,window->event_mask,&event)==0)
	  return(0);

     switch(event.type)
     {
     case ButtonPress:  /* A Mouse button has been pressed */
	  switch(event.xbutton.button)  /* Which button ? */
	  {
	  case Button1: return(W_MS_LEFT); break;
	  case Button2: return(W_MS_MIDDLE); break;
	  case Button3: return(W_MS_RIGHT); break;
	  case Button4: return(W_MS_UP); break;
	  case Button5: return(W_MS_DOWN); break;
	  default:
	       WLIB_ERROR;
	       fprintf(stderr,
		       "[WUserEvent] This mouse button (#%d) is not recognized !\n",
		       event.xbutton.button);
	       return(-1); 
	       break;
	  }

     case LeaveNotify: return(W_LEAVE); break;    
     case EnterNotify: return(W_ENTER); break;    

     case DestroyNotify:
#ifdef W_DEBUG_ON
	  fprintf(stderr,"\nWUserEvent : DestroyNotify event\n");
#endif
	  return(W_DESTROY); break;

	  /* This seems to be needed when using XLookupString */
     case MappingNotify:
	  XRefreshKeyboardMapping((XMappingEvent *)&event);
	  return(-1);
	  break;

     case KeyPress: 
	  if (WX_KeyPress((XKeyEvent *)&event) < 0) return(-1);
	  else return(W_KEYPRESS);
	  break;    

     case GraphicsExpose: case Expose: case NoExpose:
	  exp_event = (XExposeEvent *) &event;
	  if (exp_event->count == 0) 
	  { /* This is the last Expose event */
	       WFlushWindow(window);    /* Then restore the content of the window */
#ifdef W_DEBUG_ON
	       fprintf(stderr,"\nWUserEvent : Expose Event (WFlushWindow done)\n");
	       fprintf(stderr,"               Window ID : %d\n",(int)window->win);
	       fprintf(stderr,"               Event Window ID : %d\n",
		       (int)exp_event->window);
#endif
	  }
	  return(-1);
	  break;

     case ConfigureNotify : 
	  conf_event = (XConfigureEvent *) &event;
	  if (window->win != conf_event->window) break;
	  /* Set new location */
	  window->x = conf_event->x+1;
	  window->y = conf_event->y+1;
	  if ((window->dx != conf_event->width) || 
	      (window->dy != conf_event->height))
	  {                           /* Window's size has changed */

#ifdef W_DEBUG_ON
	       fprintf(stderr,"\nWUserEvent : Current window size = (%d,%d)\n",
		       window->dx,window->dy);
	       fprintf(stderr,"               New window size = (%d,%d)\n",
		       conf_event->width,conf_event->height);
	       fprintf(stderr,"               Window ID : %d\n",(int)window->win);
	       fprintf(stderr,"               Event Window ID : %d\n",
		       (int)conf_event->window);
#endif

	       WFlushWindow(window);
	       window->dx = conf_event->width; 
	       window->dy = conf_event->height;
	       if (window->ximage != NULL) 
		    WX_AllocXImage(window,window->dx,window->dy);
	       if (window->pixmap != (Pixmap) NULL) 
		    WX_AllocXPixmap(window,window->dx,window->dy);
	       return(W_RESIZE);
	  }
	  else return(-1);  /* Window's size remains the same */
	  break;

     default: return(-1); break;
     }
}

/*               Get the current state of the mouse :                     */
/*               location in (*x,*y)                                      */
/*               *button_mask describes the mouse's buttons that are down */
/*               Return -1 if not in the window, 0 elsewhere              */

int WGetStateMouse(Wframe *window, int *x, int *y, int *button_mask)
{
     Window root,child;  /* Root (and child) windows ID where the mouse is */
     int root_x,root_y;  /* coordinates relative to the root's origin */
     int win_x,win_y;    /* coordinates relative to the origin of window */
     unsigned int keys_buttons; /* current state of keys and mouse buttons */
     Bool XQ;            /* Return of XQueryPointer */

     WDEBUG(WGetStateMouse); 

     XQ = XQueryPointer(_W_Display,window->win,
			&root,&child,&root_x,&root_y,&win_x,&win_y,&keys_buttons);

     *x = win_x; *y = win_y;
     *button_mask = 0;

     if ((keys_buttons & Button1Mask) == Button1Mask) *button_mask |= W_MS_LEFT;
     if ((keys_buttons & Button2Mask) == Button2Mask) *button_mask |= W_MS_MIDDLE;
     if ((keys_buttons & Button3Mask) == Button3Mask) *button_mask |= W_MS_RIGHT;

     if (XQ == False) return(-1);
     else return(0);
}

/*   Get the key pressed on the keyboard. Must be called after a W_KEYPRESS */
/*   event has be returned by WUserEvent().                                 */

int WGetKeyboard(void)
{
     return(_W_KeyBuffer);
}


/*----- Open-Window functions -----*/

/*               Make the Window structure */

Wframe *WNewImageWindow(void)
{
     Wframe *window;

     WDEBUG(WNewImageWindow); 

     window = (Wframe *) malloc(sizeof(Wframe));
     if (window == NULL)
     {
	  WLIB_ERROR;
	  fprintf(stderr,"[WNewImageWindow] Not enough memory\n");
	  return(window);
     }
  
     window->win = (Window) NULL; 
     window->pix = NULL; window->ximage = NULL;
     window->pixmap = (Pixmap) NULL;
     window->ix = window->iy = 0;
     window->px = window->py = 0;
     window->event_mask = 0;
     window->x=0;
     window->y=0;
     window->dx = 0;
     window->dy = 0;
     return(window);
}	

/*               Open a new Image-like Window */

Wframe *WOpenImageWindow(int width, int height, int ltx, int lty, char *label)
{
     XSetWindowAttributes	theWindowAttributes;
     XSizeHints		theSizeHints;
     unsigned	long	theWindowMask;
     Pixmap		theIconPixmap;
     XWMHints		theWMHints;
     XClassHint		theClassHint;
     Wframe *window;
     int connected;
  
     WDEBUG(WOpenImageWindow); 

     connected = 0;
     if (_W_Display == NULL) connected = WX_Init((char *) NULL);
     if ((connected < 0) || (_W_Display == NULL))
	  return((Wframe *) NULL);

     window = WNewImageWindow();
     if (window == NULL) return(window);

     theWindowAttributes.border_pixel      = _W_BlackPixel;
     theWindowAttributes.background_pixel  = _W_WhitePixel;
     theWindowAttributes.cursor            = _W_Cursor;    
     theWindowAttributes.override_redirect = False; 

     theWindowMask = CWBackPixel | CWBorderPixel | CWCursor | CWOverrideRedirect;

     window->win = XCreateWindow( _W_Display,
				  RootWindow( _W_Display , _W_Screen ),
				  ltx, lty,	
				  width, height,	
				  BORDER_WIDTH,
				  _W_Depth,
				  InputOutput,   /* Class */
				  CopyFromParent,
				  theWindowMask,
				  &theWindowAttributes );

     theIconPixmap = XCreateBitmapFromData( _W_Display,
					    window->win,
					    theIcon_bits,
					    theIcon_width, 
					    theIcon_height );
     theWMHints.initial_state = NormalState;
     theWMHints.icon_pixmap   = theIconPixmap;
     theWMHints.flags         = IconPixmapHint | StateHint;
     XSetWMHints( _W_Display, window->win, &theWMHints );

     XStoreName( _W_Display, window->win, label );
     XSetIconName( _W_Display, window->win, IMAGE_RES_NAME );
     theClassHint.res_name  = IMAGE_RES_NAME;
     theClassHint.res_class = IMAGE_RES_CLASS;
     XSetClassHint( _W_Display, window->win, &theClassHint );

     theSizeHints.flags      = USPosition | USSize | PMinSize | PMaxSize;	
     theSizeHints.x          = ltx;
     theSizeHints.y          = lty;
     theSizeHints.width      = width;
     theSizeHints.height     = height;
     theSizeHints.min_width  = IMAGE_MIN_WIDTH;
     theSizeHints.max_width  = IMAGE_MAX_WIDTH; 
     theSizeHints.min_height = IMAGE_MIN_HEIGHT;
     theSizeHints.max_height = IMAGE_MAX_HEIGHT;
     XSetNormalHints( _W_Display, window->win, &theSizeHints );

     WX_AllocXImage( window, width, height ); 
     WX_AllocXPixmap( window, width, height );  

     window->x=ltx;
     window->y=lty;
     window->dx=width;
     window->dy=height;

     WClearWindow(window);
     /*XMapWindow( _W_Display, window->win );*/
     XMapRaised( _W_Display, window->win );

     WFlushWindow(window);

     return( window );
}
	
/*               Re-open a new Image-like Window                     */
/*               (open in the case where a window ID already exists) */

void WReOpenImageWindow(Wframe *window, int width, int height, 
			int ltx, int lty, char *label)
{
     XWindowAttributes	theWindowAttributes;
     XSizeHints		theSizeHints;
     unsigned	long	theWindowMask;
     Pixmap		theIconPixmap;
     XWMHints		theWMHints;
     XClassHint		theClassHint;
     char                  newsize=0;

     WDEBUG(WReOpenImageWindow); 

     if (_W_Display == NULL)
     {
	  WLIB_ERROR;
	  fprintf(stderr,
		  "[WReOpenImageWindow] No connection to an X server ! (_W_Display is NULL)\n");
	  return;
     }

     if ((WX_Init((char *) NULL) < 0) || (_W_Display == NULL))
	  return;

     if ((window == NULL) || (window->win == 0))
     {
	  WLIB_ERROR;
	  fprintf(stderr,
		  "[WReOpenImageWindow] NULL Wframe or NULL X window !\n");
	  return;
     }

     XGetWindowAttributes(_W_Display,window->win,&theWindowAttributes);

     /* Test whether or not the size of the new window has changed */
     if ((theWindowAttributes.width != width) ||
	 (theWindowAttributes.height != height))
     {
	  XResizeWindow(_W_Display,window->win,width,height);
	  newsize=1;   
     }

     if ((theWindowAttributes.x != ltx) ||
	 (theWindowAttributes.y != lty))
	  XMoveWindow(_W_Display,window->win,ltx,lty);

     theIconPixmap = XCreateBitmapFromData( _W_Display,
					    window->win,
					    theIcon_bits,
					    theIcon_width, 
					    theIcon_height );
     theWMHints.initial_state = NormalState;
     theWMHints.icon_pixmap   = theIconPixmap;
     theWMHints.flags         = IconPixmapHint | StateHint;
     XSetWMHints( _W_Display, window->win, &theWMHints );

     XStoreName( _W_Display, window->win, label );
     XSetIconName( _W_Display, window->win, IMAGE_RES_NAME );
     theClassHint.res_name  = IMAGE_RES_NAME;
     theClassHint.res_class = IMAGE_RES_CLASS;
     XSetClassHint( _W_Display, window->win, &theClassHint );

     theSizeHints.flags      = USPosition | USSize | PMinSize | PMaxSize;	
     theSizeHints.x          = ltx;
     theSizeHints.y          = lty;
     theSizeHints.width      = width;
     theSizeHints.height     = height;
     theSizeHints.min_width  = IMAGE_MIN_WIDTH;
     theSizeHints.max_width  = IMAGE_MAX_WIDTH; 
     theSizeHints.min_height = IMAGE_MIN_HEIGHT;
     theSizeHints.max_height = IMAGE_MAX_HEIGHT;
     XSetNormalHints( _W_Display, window->win, &theSizeHints );

     if ((newsize==1)||(window->ximage == NULL))
     {
	  if (window->ximage != NULL) 
	  {
	       XDestroyImage(window->ximage);
	       window->ximage = NULL;
	  }
	  WX_AllocXImage( window, width, height ); 
     }

     if ((newsize==1)||(window->pixmap == 0))
     {
	  if (window->pixmap != 0) 
	  {
	       XFreePixmap(_W_Display,window->pixmap);
	       window->pixmap = 0;
	  }
	  WX_AllocXPixmap( window, width, height );  
     }

     window->x=ltx;
     window->y=lty;
     window->dx=width;
     window->dy=height;

     WClearWindow(window);
     /*XMapWindow( _W_Display, window->win );*/
     XMapRaised( _W_Display, window->win );
     WFlushWindow(window);
}
