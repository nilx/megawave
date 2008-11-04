/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {fview};
  version = {"1.8"};
  author = {"Jacques Froment"};
  function = {"View a floating point image on a window"};
  usage = {
  'x':[pos_x=50]->x0    "upper-left corner of the Window (X coordinate)",
  'y':[pos_y=50]->y0    "upper-left corner of the Window (Y coordinate)",
  'z':[zoom=1.0]->zoom  "Zoom factor",
  'o':[order=0]->order  "Zoom order: 0,1=linear,-3=cubic,3,5..11=spline",
  'l'->linear           "allow linear rescaling only (preserve 0)",
  'm':m->m              "specify minimum displayed value (black)",
  'M':M->M              "specify maximum displayed value (white)",
  'd':d->d              "...or discard d percent of the extremal values",
  'N'->no_refresh       "Do not refresh the window (library call)",
  fimage->input         "Input image (should be a fimage)",
  notused->window       "Window to view the image (internal use)"
};
*/
/*----------------------------------------------------------------------
 v1.2: added -o option + several minor modifications (L.Moisan)
 v1.3: added -l option and removed "constant values" error (L.Moisan)
 v1.4: fixed bug with non char input keys (L.Moisan)
 v1.5: fixed lag due to usleep() (L.Moisan)
 v1.6: fixed 'c' bug (nrow->ncol) (L.Moisan)
 v1.7: added -m -M -d options, removed s key (L.Moisan)
 v1.8 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mw.h"
#include "mw-modules.h" /* for fthre() */

static Wframe *PlotWindow=NULL;  

/* Param structure used to send parameters to fview_notify() */

typedef struct fview_SParam {
  Fimage image_float;
  Cimage image_work;  
  unsigned char *image_save;
  float *fimage_save;
  char has_to_delete_image;
  Fsignal section;
} *fview_Param;

int GLprint;  /* Toggle to print the Gray Level values */
int oldx1,oldy1,oldevent,zfactor,cscale_shown;

static void fview_notify_help()

{
  printf("\n\t\tHelp on line\n");

  printf("\nMouse:\n");
  printf("\tLeft button: toggle to print the gray level at the current coordinates (x,y).\n");

  printf("\tMiddle button: Restore the image\n");
  printf("\tRight button: Local zooming\n");

  printf("\nKeyboard:\n");
  printf("\tQ: Quit.\n");
  printf("\tH: Help.\n");
  printf("\tL: Plot the current Line section.\n");
  printf("\tC: Plot the current Column section.\n");
}


/*     A notify function must return a value ....      */
/*       0 if there was no event catched               */
/*     > 0 if there was an event catched (but Destroy) */
/*      -1 if the event Destroy was catched (or 'Q')   */

static int fview_notify(ImageWindow,param)

Wframe *ImageWindow;
void *param;          /* Users's parameters: don't forget the cast ! */

{
  int x1,y1,wz,event,button_mask,ret,c;
  char mess[80];
  float ng;
  fview_Param images;
  Cimage image;
  Fimage fimage;
  unsigned char *gray_save;
  float *fgray_save;
  char delete_image;

  /* For section */
  Fsignal section;
  int x0=0;
  int y0=0;
  int sx=500;
  int sy=200;
  char cflag=1;
  int norefresh=1;

  images = (fview_Param) param;  /* Cast */
  image = images->image_work;
  fimage = images->image_float;
  gray_save = images->image_save;
  fgray_save = images->fimage_save;
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
	      ng = fimage->gray[ x1 + (image->ncol)* y1 ];
	      sprintf(mess," (%3d,%3d): %f ",x1,y1,ng);
	    }
	  else sprintf(mess," (%3d,%3d): ----------",x1,y1);
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
      memcpy(fimage->gray,fgray_save,fimage->ncol*fimage->nrow*sizeof(float));
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
	      memcpy(fimage->gray,fgray_save,
		     fimage->ncol*fimage->nrow*sizeof(float));
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
	  memcpy(fimage->gray,fgray_save,
		 fimage->ncol*fimage->nrow*sizeof(float));
	}
      
      /* Size of the zoom window */
      if (image->ncol >= image->nrow) 
	wz = zfactor*image->nrow/16;
      else 
	wz = zfactor*image->ncol/16;

      image = (Cimage) clocal_zoom(image, &x1, &y1, &wz, &zfactor);
      fimage = (Fimage) flocal_zoom(fimage, &x1, &y1, &wz, &zfactor);
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

	case 'h': case 'H': fview_notify_help();
	  break;

	case 'l' :
	  if ((y1>=0)&&(y1<fimage->nrow))
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
	      if (PlotWindow != NULL) {x0=PlotWindow->x;y0=PlotWindow->y;}
	      else 
		{
		  x0 = ImageWindow->x + 2 + fimage->ncol;
		  y0 = ImageWindow->x + 2 + ((fimage->nrow - sy) / 2);
		  if (y0 < 0) y0 = 0;
		}
	      fline_extract((char *) NULL, fimage, section, y1);
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
	  if ((x1>=0)&&(x1<fimage->ncol))
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
	      if (PlotWindow != NULL) {x0 = PlotWindow->x;y0 = PlotWindow->y;}
	      else 
		{
		  x0 = ImageWindow->x + 2 + fimage->ncol;
		  y0 = ImageWindow->x + 2 + ((fimage->nrow - sy) / 2);
		  if (y0 < 0) y0 = 0;
		}
	      fline_extract(&cflag, fimage, section,x1);
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
      free(fgray_save);
      free(images);
    }

  return(ret);

}


static Cimage fimage_to_cimage(in,out)
     Fimage in;
     Cimage out;
{
  int n;
  float v;

  out = mw_change_cimage(out,in->nrow,in->ncol);
  for (n=in->nrow*in->ncol;n--;) {
    v = in->gray[n];
    if (v<0.) v=0.;
    if (v>255.) v=255.;
    out->gray[n] = (unsigned char)v;
  }

  return(out);
}


/*------------------------------ MAIN MODULE ------------------------------*/

void fview(input,x0,y0,zoom,order,no_refresh,window,linear,m,M,d)
     int    *x0,*y0,*no_refresh,*order;
     float  *zoom,*m,*M,*d;
     Fimage input;
     Wframe *window;
     char   *linear;
{
  Wframe *ImageWindow;
  Fimage fimage,tmp;
  Cimage cimage;
  Fsignal section;
  float  *fgray_save,inverse_zoom;
  unsigned char *gray_save;
  fview_Param param;
  int i,j,smax;

  if (*zoom != 1.0) 
    {
      fimage = mw_change_fimage(NULL,0,0);
      if (fimage == NULL) mwerror(FATAL,1,"Not enough memory\n");
      if (*zoom>1.0) 
	fzoom(input,fimage,NULL,NULL,zoom,order,NULL,NULL);
      else {
	inverse_zoom = 1./(*zoom);
	fzoom(input,fimage,NULL,NULL,&inverse_zoom,order,NULL,(char *)1);
      }
      sprintf(fimage->name,"%s %.1fX",input->name,*zoom);
    }
  else 
      fimage=input;

  ImageWindow = mw_get_window(window, fimage->ncol, fimage->nrow,
			      *x0, *y0, fimage->name);
  if (ImageWindow == NULL)
    mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");

  tmp = mw_new_fimage();
  fthre(fimage,tmp,NULL,(char *)1,m,M,NULL,NULL,d,linear,linear);
  cimage = fimage_to_cimage(tmp,NULL);
  mw_delete_fimage(tmp);
    
  WLoadBitMapImage(ImageWindow,cimage->gray,cimage->ncol,cimage->nrow); 
  WRestoreImageWindow(ImageWindow,0,0,cimage->ncol,cimage->nrow); 
  WFlushWindow(ImageWindow);

  GLprint = 0;
  oldx1 = oldy1 = oldevent = -1;
  zfactor = 2;
  gray_save = (unsigned char *) malloc(cimage->ncol * cimage->nrow);
  if (gray_save == NULL) mwerror(FATAL,1,"Not enough memory\n");
  fgray_save = (float *) malloc(cimage->ncol * cimage->nrow * sizeof(float));
  if (fgray_save == NULL) mwerror(FATAL,1,"Not enough memory\n");

  cscale_shown = 0;

  /* Allocate section signal */
  smax = fimage->ncol;
  if (smax < fimage->nrow) smax = fimage->nrow;
  if ((section = mw_change_fsignal(NULL, smax)) == NULL)
      mwerror(FATAL,1,"Not enough memory\n");

  param = (fview_Param) malloc(sizeof(struct fview_SParam));
  if (param == NULL) mwerror(FATAL,1,"not enough memory\n");

  memcpy(gray_save,cimage->gray,cimage->ncol * cimage->nrow);
  memcpy(fgray_save,fimage->gray,fimage->ncol * fimage->nrow*sizeof(float));
  WSetUserEvent(ImageWindow,W_MS_BUTTON | W_KEYPRESS);
  param->image_float = fimage;
  param->image_work = cimage;
  param->image_save = gray_save;
  param->fimage_save = fgray_save;
  param->section = section;

  if (fimage == input) param->has_to_delete_image=0;
  else param->has_to_delete_image=1;
  mw_window_notify(ImageWindow,(void *)param,fview_notify);
  if (!no_refresh) mw_window_main_loop();
}







