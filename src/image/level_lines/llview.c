/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {llview};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Interactive view of the level lines of an image"};
  usage = {
  'x':[x0=50]->x0     "window upleft corner (x coordinate, default 50)",
  'y':[y0=50]->y0     "window upleft corner (y coordinate, default 50)",
  'z':[zoom=2.0]->z   "zoom factor (default 2.0)",
  's':[step=20]->s    "starting step of level grid (default 20)",
  'p':[position=0]->p "starting position of level grid (default 0)",
  'd'->d              "to start with bi-level sets (instead of level lines)",
  'b'->b              "to start without the image as background",
  'a':scale->scale    "to perform first 'amss -l scale' (e.g scale=2.0)",
  'o':output<-output  "to save the last view as a Cimage",
  cimage->input       "Input Cimage",
  notused->window     "Window to view the image (internal use)"
  };
*/

#include <stdio.h>
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"


#define ZFMAX         4       /* Zoom Factor Max */

/*--------------------------------------------------------------*/
/* global variables avoids to pass parameters to ll_view_notify */
/*--------------------------------------------------------------*/
int PARprint;             /* Toggle to print the parameters     */
int oldx1,oldy1;
int zfactor;              /* zoom factor                        */
int back_flag,mode;       /* background and lines/sets toggles  */
int grid_ofs,grid_step;   /* grid offset and step               */
Cimage display_image;     /* image being currently displayed    */
Cimage ref_image;         /* reference image                    */
Cimage real_image;        /* real image displayed (ref + zoom)  */
/*--------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Level lines / Bi-level sets  Map */

Cimage ll_map(input, output, step,ofs,flag,mode)
Cimage input,output;
int step,ofs,flag,mode;
{
  unsigned char *in,*out,background;
  int dx,dy,size,l;

  dy= input->nrow;
  dx= input->ncol;
  size=dx*dy;

  output=mw_change_cimage(output,dy,dx);
  mw_clear_cimage(output,255);

  if (flag) background=84; else background=255;

  switch(mode) {

  case 1: /* level lines */
    for (l=dx, in=input->gray+dx, out = output->gray+dx;
	 l<size; 
	 l++,in++,out++) {
      if ( (((*(in-1)-ofs+step)/step)!=((*in-ofs+step)/step)) 
	   || (((*(in-dx)-ofs+step)/step)!=((*in-ofs+step)/step)) )
	*out=0;
      else
	if (flag) *out=64+((*in)*3)/4; else *out=255;    
    }
    break;
    
  case 2: /* bi-level sets */
    for (l=0, in=input->gray, out = output->gray; 
	 l<size; 
	 l++,in++,out++) {
      if ( ! (((*in-ofs)/step) & 1) )
	*out=0;
      else
	if (flag) *out=64+((*in)*3)/4; else *out=255;
    }
    break;
  }

  return output;
}


/*----------------------------------------------------------------------*/

void llview_notify_help()

{
  printf("\n\t\tHelp on line\n");

  printf("\nMouse:\n");

  printf("\tLeft button: Select grid offset (current level)\n");
  printf("\tMiddle button: Restore the image\n");
  printf("\tRight button: Local zooming\n");

  printf("\nKeyboard:\n");
  printf("\tq: Quit.\n");
  printf("\th: Help.\n");
  printf("\tb: Toggle image background.\n");
  printf("\td: Toggle display (level lines / bi-level sets).\n");
  printf("\t+: Translate right level grid.\n");
  printf("\t-: Translate left level grid.\n");
  printf("\t*: Increase grid step.\n");
  printf("\t/: Decrease grid step.\n");
}


/*     A notify function must return a value ....      */
/*       0 if there was no event catched               */
/*     > 0 if there was an event catched (but Destroy) */
/*      -1 if the event Destroy was catched (or 'Q')   */

int llview_notify(ImageWindow,param)

Wframe *ImageWindow;
void *param;          /* Users's parameters: don't forget the cast ! */

{
  Cimage im;
  int x1,y1,wz,event,button_mask,ret;
  char c,mess[200];
  int ng,refresh;

  event = WUserEvent(ImageWindow); /* User's event on ImageWindow */
  if (event < 0) ret=1; else ret=event;
  if (event != W_DESTROY)
    {
      WGetStateMouse(ImageWindow,&x1,&y1,&button_mask);
      if (PARprint == 1) { 
	/* print parameters on top of the window */
	if ((x1>=0)&&(x1<display_image->ncol)
	    &&(y1>=0)&&(y1<display_image->nrow))
	  ng = real_image->gray[y1*ref_image->nrow+x1];
	else ng = -1;
	switch(mode) {
	  case 1: /* level lines */
	    sprintf(mess," %3d (level lines %d+%dn)",
		    ng,grid_ofs,grid_step);
	    break;
	  case 2: /* bi-level sets */
	    sprintf(mess," %3d (bi-level sets %d <= u-%dn < %d)",
		    ng,grid_ofs,grid_step*2,grid_ofs+grid_step+1);
	    break;
	}
	WDrawString(ImageWindow,0,10,mess);
	WFlushAreaWindow(ImageWindow,0,0,display_image->ncol-1,12);
      }
    }
  
  refresh = 0;

  switch(event)
    {
    case W_MS_LEFT:
      PARprint = 1 - PARprint;
      if (PARprint == 0)
	{
	  WRestoreImageWindow(ImageWindow,0,0,
			      display_image->ncol,display_image->nrow); 
	  WFlushWindow(ImageWindow);
	}
      break;
      
    case W_MS_MIDDLE:
      refresh = 1;
      zfactor = 1;
      break;

    case W_MS_RIGHT:
      if ((oldx1 == x1) && (oldy1 == y1)) 
	{
	  zfactor+=1;
	  if (zfactor > ZFMAX) zfactor = ZFMAX;
	  else refresh = 1;
}
      else {
	zfactor=2;
	refresh = 1;
	oldx1 = x1;
	oldy1 = y1;
      }
      break;

    case W_RESIZE:
      WLoadBitMapImage(ImageWindow,display_image->gray,
		       display_image->ncol,display_image->nrow); 
      WRestoreImageWindow(ImageWindow,0,0,
			  display_image->ncol,display_image->nrow); 
      WFlushWindow(ImageWindow);	 
      break;  

    case W_DESTROY:
      ret=-1;
      break;

    case W_KEYPRESS:
      c = (char) WGetKeyboard();
      switch(c)
	{
	case 'q': case 'Q': ret = -1;
	  break;
	  
	case 'h': case 'H': llview_notify_help();
	  break;
	  
	case 'b': case 'B': 
	  back_flag = 1-back_flag;
	  refresh=1;
	  break;

	case 'd': case 'D': 
	  mode = 3-mode;
	  if (mode==1) grid_ofs = grid_ofs%grid_step;
	  refresh=1;
	  break;

	case '+':
	  grid_ofs = (grid_ofs+1)%(grid_step*mode);
	  refresh=1;
	  break;

	case '-':
	  grid_ofs = (grid_ofs+grid_step*mode-1)%(grid_step*mode);
	  refresh=1;
	  break;

	case '*':
	  grid_step ++;
	  refresh=1;
	  break;

	case '/':
	  if (grid_step>1) grid_step--;
	  refresh=1;
	  break;

	default:
	  mwerror(WARNING,1,"Unrecognized Key '%c'. Type H for Help.\n",c);
	}    
      break;
    }

  if (refresh) {
    /* compute and display new image */
    if (display_image->ncol >= display_image->nrow) 
      wz = zfactor*display_image->nrow/16;
    else 
      wz = zfactor*display_image->ncol/16;
    memcpy(real_image->gray,ref_image->gray,ref_image->ncol*ref_image->nrow);
    if (zfactor!=1) 
      clocal_zoom(real_image, &oldx1, &oldy1, &wz, &zfactor);
    ll_map(real_image,display_image,grid_step,grid_ofs,back_flag,mode);
    if (WLoadBitMapImage(ImageWindow,display_image->gray,
			 display_image->ncol,display_image->nrow)
	!= 0) return(-1);
    WRestoreImageWindow(ImageWindow,0,0,
			display_image->ncol,display_image->nrow); 
    WFlushWindow(ImageWindow);
  }
 
  return(ret);

}

/*----------------------------------------------------------------------*/

llview(input,x0,y0,z,s,p,d,b,scale,output,window)

Cimage input,output;
int *x0,*y0,*s,*p;
char *d,*b,*window;
float *z,*scale;

{
  Wframe *ImageWindow;
  Cimage tmp1,tmp2;
  Fimage ftmp1,ftmp2;
  int i,j;
  float v,p1,p2,p3,p4;
  char text[BUFSIZ];

  /*--- Initialize the parameters ---*/
  PARprint = 0;
  oldx1 = oldy1 = -1;
  zfactor = 1;
  if (b) back_flag=0; /* no background */
    else back_flag=1; /* image as background */
  if (d) mode=2; /* bi-level sets */
    else mode=1; /* level lines */
  grid_ofs = *p;
  grid_step = *s;

  /* zoom if requested */
  if (*z!=1.0) {
    tmp1 = mw_new_cimage();
    czoom(input,tmp1,NULL,NULL,z);
  } else tmp1 = input;
  
  /* smooth if requested */
  if (scale) {    
    ftmp1 = mw_change_fimage(NULL,tmp1->nrow,tmp1->ncol);
    ftmp2 = mw_new_fimage();
    for (i=tmp1->nrow*tmp1->ncol;i--;)
      ftmp1->gray[i] = (float)tmp1->gray[i];
    p1=0.1;p2=6.0;p3=0.1;p4=0.0;i=0;
    amss(NULL,NULL,&p1,&p2,&p3,&p4,scale,ftmp1,&ftmp2,&i,&i,
	 NULL,NULL,NULL,NULL);
    tmp2 = mw_change_cimage(NULL,ftmp2->nrow,ftmp2->ncol);
    for (i=tmp2->nrow*tmp2->ncol;i--;) {
      v = ftmp2->gray[i];
      tmp2->gray[i] = (v<0.0?0:(v>255.0?255:v));
    }
  } else tmp2 = tmp1;

  ref_image = tmp2;

  real_image = mw_change_cimage(NULL,ref_image->nrow,ref_image->ncol);
  memcpy(real_image->gray,ref_image->gray,ref_image->nrow*ref_image->ncol);

  display_image = ll_map(ref_image,NULL,*s,*p,back_flag,mode);

  sprintf(text,"level lines of %s",input->name);

  /* display window */
  ImageWindow = (Wframe *)
    mw_get_window((Wframe *) window,
		  display_image->ncol,display_image->nrow,*x0,*y0,text);
  if (ImageWindow == NULL)
    mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");
  WLoadBitMapImage(ImageWindow,display_image->gray,
		   display_image->ncol,display_image->nrow); 
  WRestoreImageWindow(ImageWindow,0,0,
		      display_image->ncol,display_image->nrow); 
  WFlushWindow(ImageWindow);

  WSetUserEvent(ImageWindow,W_MS_BUTTON | W_KEYPRESS);
  mw_window_notify(ImageWindow,NULL,llview_notify);

  /* loops on events */
  mw_window_main_loop();

  /* save output if requested */
  if (output) {
    mw_change_cimage(output,display_image->nrow,display_image->ncol);
    memcpy(output->gray,display_image->gray,
	   display_image->nrow*display_image->ncol);
  }
}







