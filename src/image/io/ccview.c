/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {ccview};
  version = {"1.2"};
  author = {"Jacques Froment"};
  function = {"View a color image on a window"};
  usage = {
  'x':[pos_x=50]->x0
      "X coordinate for the upper-left corner of the Window",
  'y':[pos_y=50]->y0
      "Y coordinate for the upper-left corner of the Window",
  'z':[zoom=1.0]->zoom
      "Zoom factor",
  'N'->no_refresh
      "Do not refresh the window (library call)",
  ccimage->image
        "Input image (should be a ccimage)",
   notused->window 
      "Window to view the image (internal use)"
  };
*/
/*--- MegaWave - Copyright (C) 1994 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 include file */
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"


/* Param structure used to send parameters to ccview_notify() */

typedef struct ccview_SParam {
    Ccimage image_work;  
    unsigned char *image_red_save;
    unsigned char *image_green_save;
    unsigned char *image_blue_save;
  } *ccview_Param;

int GLprint;  /* Toggle to print the R,G,B values */
int oldx1,oldy1,oldevent,zfactor;

void ccview_notify_help()

{
  printf("\n\t\tHelp on line\n");

  printf("\nMouse:\n");
  printf("\tLeft button: toggle to print the RGB value at the current coordinates (x,y).\n");

  printf("\tMiddle button: Restore the image\n");
  printf("\tRight button: Local zooming\n");

  printf("\nKeyboard:\n");
  printf("\tQ: Quit.\n");
  printf("\tH: Help.\n");
}

/*     A notify function must return a value ....      */
/*       0 if there was no event catched               */
/*     > 0 if there was an event catched (but Destroy) */
/*      -1 if the event Destroy was catched (or 'Q')   */

int ccview_notify(ImageWindow,param)

Wframe *ImageWindow;
void *param;          /* Users's parameters: don't forget the cast ! */

{
  int x1,y1,wz,event,button_mask,ret,imsize;
  char c,mess[90];
  int nred,ngreen,nblue,l;
  ccview_Param images;
  Ccimage image;
  unsigned char *red_save, *green_save, *blue_save;


  images = (ccview_Param) param;  /* Cast */
  image = images->image_work;
  red_save = images->image_red_save;
  green_save = images->image_green_save;
  blue_save = images->image_blue_save;
  imsize = image->ncol*image->nrow;

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
	      l =  x1 + (image->ncol)* y1;
	      nred = image->red[l];
	      ngreen = image->green[l];
	      nblue = image->blue[l];
	    }
	  else { nred = ngreen = nblue = -1; }
	  sprintf(mess," (%3d,%3d): (%3d,%3d,%3d) ",x1,y1,nred,ngreen,nblue);
	  WDrawString(ImageWindow,0,10,mess);
	  WFlushAreaWindow(ImageWindow,0,0,image->ncol-1,12);
	  usleep(100);
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
      memcpy(image->red,red_save,imsize);
      memcpy(image->green,green_save,imsize);
      memcpy(image->blue,blue_save,imsize);
      if (WLoadBitMapColorImage(ImageWindow,image->red,image->green,
				image->blue,image->ncol,image->nrow) != 0) 
	return(-1);
      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
      WFlushWindow(ImageWindow);
      zfactor = 2; 
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
	      memcpy(image->red,red_save,imsize);
	      memcpy(image->green,green_save,imsize);
	      memcpy(image->blue,blue_save,imsize);	
	      if (WLoadBitMapColorImage(ImageWindow,image->red,image->green,
				image->blue,image->ncol,image->nrow) != 0) 
		return(-1);
	      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow);
	      WFlushWindow(ImageWindow);
	      zfactor = 2; 
	      oldevent = event;
	      oldx1 = x1;
	      oldy1 = y1;
	      break;
	    }
	  zfactor = 2;
	  memcpy(image->red,red_save,imsize);
	  memcpy(image->green,green_save,imsize);
	  memcpy(image->blue,blue_save,imsize);	
	}

      /* Size of the zoom window */
      if (image->ncol >= image->nrow) 
	wz = zfactor*image->nrow/16;
      else 
	wz = zfactor*image->ncol/16;

      image = (Ccimage) cclocal_zoom(image, &x1, &y1, &wz, &zfactor);
      if (WLoadBitMapColorImage(ImageWindow,image->red,image->green,
				image->blue,image->ncol,image->nrow) != 0) 
	return(-1);
      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
      WFlushWindow(ImageWindow);
      oldevent = event;
      oldx1 = x1;
      oldy1 = y1;
      break;

    case W_RESIZE:
      oldevent = event;
      break;

    case W_DESTROY:
      ret=-1;
      break;

     case W_KEYPRESS:
      c = (char) WGetKeyboard();
      switch(c)
	{
	case 'q': case 'Q': ret =-1;
	  break;

	case 'h': case 'H': ccview_notify_help();
	  break;

	default:
	  mwerror(WARNING,1,"Unrecognized Key '%c'. Type H for Help.\n",c);
	}
      oldevent = event; 
      break;
    }

  if (ret == -1)
    {
      free(param);
      free(blue_save);
      free(green_save);
      free(red_save);
    }

  return(ret);
}


ccview(image,x0,y0,zoom,no_refresh,window)

int *x0,*y0,*no_refresh;
float *zoom;
Ccimage image;
char *window;

{
  Wframe *ImageWindow;
  Ccimage zimage=NULL;
  unsigned char *red_save,*green_save,*blue_save;
  ccview_Param param;
  int i,j;
  char text[BUFSIZ];

  if (*zoom != 1) 
    {
      zimage = mw_change_ccimage(zimage,0,0);
      if (zimage == NULL) mwerror(FATAL,1,"Not enough memory\n");
      cczoom(image,zimage,NULL,NULL,zoom);
      sprintf(text,"%s (%.1fX)",image->name,*zoom);
      mw_delete_ccimage(image);
      image = zimage;
    }
  else strcpy(text,image->name);

  
  ImageWindow = (Wframe *)
    mw_get_window((Wframe *) window,image->ncol,image->nrow,*x0,*y0,
		  text);

  if (ImageWindow == NULL)
    mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");

  WLoadBitMapColorImage(ImageWindow,image->red,image->green,image->blue,
			image->ncol,image->nrow); 
  WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
  WFlushWindow(ImageWindow);

  GLprint = 0;
  oldx1 = oldy1 = oldevent = -1;
  zfactor = 2;
  red_save = (unsigned char *) malloc(image->ncol * image->nrow);
  green_save = (unsigned char *) malloc(image->ncol * image->nrow);
  blue_save = (unsigned char *) malloc(image->ncol * image->nrow);

  param = (ccview_Param) malloc(sizeof(struct ccview_SParam));
  if ((red_save == NULL) || (green_save == NULL) || 
      (blue_save == NULL) || (param == NULL))
    mwerror(FATAL,1,"not enough memory\n");
  memcpy(red_save,image->red,image->ncol * image->nrow);
  memcpy(green_save,image->green,image->ncol * image->nrow);
  memcpy(blue_save,image->blue,image->ncol * image->nrow);

  WSetUserEvent(ImageWindow,W_MS_BUTTON | W_KEYPRESS);

  param->image_work = image;
  param->image_red_save = red_save;
  param->image_green_save = green_save;
  param->image_blue_save = blue_save;

  mw_window_notify(ImageWindow,(void *)param,ccview_notify);
  if (!no_refresh) mw_window_main_loop();
}







