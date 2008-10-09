/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cview};
 version = {"1.12"};
 author = {"Jacques Froment"};
 function = {"View an image on a window"};
 usage = {
  'x':[pos_x=50]->x0    "upper-left corner of the Window (X coordinate)",
  'y':[pos_y=50]->y0    "upper-left corner of the Window (Y coordinate)",
  'z':[zoom=1.0]->zoom  "Zoom factor (float value)",
  'o':[order=0]->order  "Zoom order: 0,1=linear,-3=cubic,3,5..11=spline",
  'N'->no_refresh       "Do not refresh the window (library call)",
   cimage->input        "Input image (should be a cimage)",
   notused->window      "Window to view the image (internal use)"
};
*/
/*----------------------------------------------------------------------
 v1.8: added -o option + several minor modifications (L.Moisan)
 v1.9: fixed bug with non char input keys (L.Moisan)
 v1.10: fixed lag due to usleep() (L.Moisan)
 v1.11: fixed 'c' bug (nrow->ncol) (L.Moisan)
 v1.12 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mw.h"

extern Cimage clocal_zoom();
extern void czoom();
extern void splot();
extern void cline_extract();


static Wframe *PlotWindow=NULL;  

/* Param structure used to send parameters to cview_notify() */

typedef struct cview_SParam {
  Cimage image_work;  
  unsigned char *image_save;
  unsigned char *image_cscale;    
  char has_to_delete_image;
  Fsignal section;
} *cview_Param;

#define H_CSCALE 20 /* Height of the color_scale box */

int GLprint;  /* Toggle to print the Gray Level values */
int oldx1,oldy1,oldevent,zfactor,cscale_shown;

void cview_notify_help()

{
  printf("\n\t\tHelp on line\n");

  printf("\nMouse:\n");
  printf("\tLeft button: toggle to print the gray level at the current coordinates (x,y).\n");

  printf("\tMiddle button: Restore the image\n");
  printf("\tRight button: Local zooming\n");

  printf("\nKeyboard:\n");
  printf("\tQ: Quit.\n");
  printf("\tH: Help.\n");
  printf("\tS: Show the color scale box.\n");
  printf("\tL: Plot the current Line section.\n");
  printf("\tC: Plot the current Column section.\n");
}


/*     A notify function must return a value ....      */
/*       0 if there was no event catched               */
/*     > 0 if there was an event catched (but Destroy) */
/*      -1 if the event Destroy was catched (or 'Q')   */

int cview_notify(ImageWindow,param)

Wframe *ImageWindow;
void *param;          /* Users's parameters: don't forget the cast ! */

{
  int x1,y1,wz,event,button_mask,ret,c;
  char mess[80];
  int ng;
  cview_Param images;
  Cimage image;
  unsigned char *gray_save,*color_scale;
  char delete_image;

  /* For section */
  Fsignal section;
  int x0;
  int y0;
  int sx=500;
  int sy=200;
  char cflag=1;
  int norefresh=1;

  images = (cview_Param) param;  /* Cast */
  image = images->image_work;
  gray_save = images->image_save;
  color_scale = images->image_cscale;
  delete_image = images->has_to_delete_image;
  section = images->section;

#define ZFMAX 4 /* Zoom Factor Max */

  event = WUserEvent(ImageWindow); /* User's event on ImageWindow */
  if (event < 0) ret=1; else ret=event;
  if (event != W_DESTROY)
    {
      WGetStateMouse(ImageWindow,&x1,&y1,&button_mask);
      if (GLprint == 1)  
	{
	  if ((x1>=0)&&(x1<image->ncol)&&(y1>=0)&&(y1<image->nrow))
	    {
	      if ((cscale_shown == 1) && (y1 < H_CSCALE))
		ng = color_scale[ x1 + (image->ncol)* y1 ];
	      else ng = image->gray[ x1 + (image->ncol)* y1 ];
	    }
	  else ng = -1;
	  sprintf(mess," (%3d,%3d): %3d ",x1,y1,ng);
	  WDrawString(ImageWindow,0,10,mess);
	  WFlushAreaWindow(ImageWindow,0,0,image->ncol-1,12);
	}
    }

  switch(event)
    {
    case W_MS_LEFT:
      GLprint = 1 - GLprint;
      if (GLprint == 0)
	{
	  WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
	  WFlushWindow(ImageWindow);
	}
      oldevent = event;
      break;
      
    case W_MS_MIDDLE: 
      memcpy(image->gray,gray_save,image->ncol*image->nrow);
      if (WLoadBitMapImage(ImageWindow,image->gray,image->ncol,image->nrow)
	  != 0) return(-1);
      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
      WFlushWindow(ImageWindow);
      zfactor = 2; 
      cscale_shown = 0;
      oldevent = event;
      break;

    case W_MS_RIGHT:
      if ((oldx1 == x1) && (oldy1 == y1) && (oldevent == event)) 
	{
	  zfactor+=1;
	  if (zfactor > ZFMAX) zfactor = ZFMAX;
	}
      else 
	{
	  if (zfactor > 2)
	    {
	      memcpy(image->gray,gray_save,image->ncol*image->nrow);
	      if (WLoadBitMapImage(ImageWindow,image->gray,image->ncol,image->nrow) != 0) return(-1);
	      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow);
	      WFlushWindow(ImageWindow);
	      zfactor = 2; 
	      oldevent = event;
	      oldx1 = x1;
	      oldy1 = y1;
	      break;
	    }
	  zfactor = 2;
	  memcpy(image->gray,gray_save,image->ncol*image->nrow);
	}
      
      /* Size of the zoom window */
      if (image->ncol >= image->nrow) 
	wz = zfactor*image->nrow/16;
      else 
	wz = zfactor*image->ncol/16;

      image = (Cimage) clocal_zoom(image, &x1, &y1, &wz, &zfactor);
      if (WLoadBitMapImage(ImageWindow,image->gray,image->ncol,image->nrow)
	  != 0) return(-1);
      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
      WFlushWindow(ImageWindow);
      oldevent = event;
      oldx1 = x1;
      oldy1 = y1;
      break;

    case W_RESIZE:
      oldevent = event;
      WLoadBitMapImage(ImageWindow,image->gray,image->ncol,image->nrow); 
      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
      WFlushWindow(ImageWindow);	 
      break;  

    case W_DESTROY:
      ret=-1;
      break;

    case W_KEYPRESS:
      c = WGetKeyboard();
      switch(c)
	{
	case 'q': case 'Q': ret = -1;
	  break;

	case 'h': case 'H': cview_notify_help();
	  break;

	case 's': case 'S': 
	  if (color_scale != NULL)
	    {
	      WLoadBitMapImage(ImageWindow,color_scale,image->ncol,H_CSCALE);
	      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
	      WFlushWindow(ImageWindow);	  
	      cscale_shown = 1;
	    }
	  break;

	case 'l' :
	  if ((y1>=0)&&(y1<image->nrow))
	    {
	      /* Draw an horizontal red line */
	      WSetTypePencil(W_COPY);
	      WSetSpecialColorPencil(ImageWindow);
	      if (WLoadBitMapImage(ImageWindow,image->gray,image->ncol,
				   image->nrow)!= 0) return(-1);
	      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
	      WDrawLine(ImageWindow,0,y1,image->ncol-1,y1);  
	      WFlushWindow(ImageWindow);	  
	      cscale_shown = 0;
	      
	      /* Compute and plot section */
	      x0 = 52 + image->ncol;
	      y0 = 52 + ((image->nrow - sy) / 2);
	      if (y0 < 0) y0 = 0;
	      cline_extract((char *) NULL, image, section, y1, NULL);
	      sprintf(section->name,"Plot a section");
	      PlotWindow = (Wframe *)
		mw_get_window(PlotWindow,sx,sy,x0,y0,"");
	      splot(section,&x0,&y0,&sx,&sy,
		    &norefresh,(char *)PlotWindow,NULL,NULL);
	      
	      /* Restore image without red line */
	      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
	      WFlushWindow(ImageWindow);
	    }
	  break;
	    
	case 'c' :
	  if ((x1>=0)&&(x1<image->ncol))
	    {
	      /* Draw a vertical red line */
	      WSetTypePencil(W_COPY);
	      WSetSpecialColorPencil(ImageWindow);
	      if (WLoadBitMapImage(ImageWindow,image->gray,image->ncol,
				   image->nrow)!= 0) return(-1);
	      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
	      WDrawLine(ImageWindow,x1,0,x1,image->nrow-1);  
	      WFlushWindow(ImageWindow);	  
	      cscale_shown = 0;

	      /* Compute and plot section */
	      x0 = 52 + image->ncol;
	      y0 = 52 + ((image->nrow - sy) / 2);
	      if (y0 < 0) y0 = 0;
	      cline_extract(&cflag, image, section,x1,NULL);
	      sprintf(section->name,"Plot a section");
	      PlotWindow = (Wframe *)
		mw_get_window(PlotWindow,sx,sy,x0,y0,"");
	      splot(section,&x0,&y0,&sx,&sy,
		    &norefresh,(char *)PlotWindow,NULL,NULL);

	      /* Restore image without red line */
	      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
	      WFlushWindow(ImageWindow);
	    }
	  break;

	default:
	  if (c>>8==0)
	    mwerror(WARNING,1,"Unrecognized Key '%c'. Type H for Help.\n",c);
	}    
      oldevent = event;
      break;
    }

  if (ret == -1)
    {
      if (delete_image == 1) mw_delete_cimage(image);
      free(gray_save);
      free(images);
    }

  return(ret);

}


void cview(input,x0,y0,zoom,order,no_refresh,window)

int *x0,*y0,*no_refresh,*order;
float *zoom;
Cimage input;
char *window;

{
  Wframe *ImageWindow;
  Cimage image=NULL;
  unsigned char *gray_save,*color_scale;
  Fsignal section;
  cview_Param param;
  int i,j,smax;
  float inverse_zoom;

  if (*zoom != 1.0) 
    {
      image = mw_change_cimage(image,0,0);
      if (image == NULL) mwerror(FATAL,1,"Not enough memory\n");
      if (*zoom>1.0) 
	czoom(input,image,NULL,NULL,zoom,order,NULL);
      else {
	inverse_zoom = 1./(*zoom);
	czoom(input,image,NULL,NULL,&inverse_zoom,order,(char *)1);
      }
      sprintf(image->name,"%s %.1fX",input->name,*zoom);
    }
  else 
      image=input;

  ImageWindow = (Wframe *)
    mw_get_window((Wframe *) window,image->ncol,image->nrow,*x0,*y0,
		  image->name);
  if (ImageWindow == NULL)
    mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");

  WLoadBitMapImage(ImageWindow,image->gray,image->ncol,image->nrow); 
  WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
  WFlushWindow(ImageWindow);

  GLprint = 0;
  oldx1 = oldy1 = oldevent = -1;
  zfactor = 2;
  gray_save = (unsigned char *) malloc(image->ncol * image->nrow);

  if ((image->ncol >= 128) && (image->nrow > H_CSCALE))
    {
      color_scale = (unsigned char *) malloc(image->ncol*H_CSCALE);
      if (color_scale != NULL)
	for (i=0;i<image->ncol;i++) 
	  for (j=0;j<H_CSCALE*image->ncol;j+=image->ncol)
	    color_scale[i+j] = i*255/(image->ncol-1);
    }

  else color_scale = NULL;
  cscale_shown = 0;

  /* Allocate section signal */
  smax = image->ncol;
  if (smax < image->nrow) smax = image->nrow;
  if ((section = mw_change_fsignal(NULL, smax)) == NULL)
    mwerror(FATAL,1,"Not enough memory\n");

  param = (cview_Param) malloc(sizeof(struct cview_SParam));
  if ((gray_save == NULL) || (param == NULL))
    mwerror(FATAL,1,"not enough memory\n");

  memcpy(gray_save,image->gray,image->ncol * image->nrow);
  WSetUserEvent(ImageWindow,W_MS_BUTTON | W_KEYPRESS);
  param->image_work = image;
  param->image_save = gray_save;
  param->image_cscale = color_scale;
  param->section = section;
  if (image == input) param->has_to_delete_image=0;
  else param->has_to_delete_image=1;
  mw_window_notify(ImageWindow,(void *)param,cview_notify);
  if (!no_refresh) mw_window_main_loop();
}







