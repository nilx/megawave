/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {fkview};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"Interactive view of Fcurves"};
  usage = {
  'X':[x=800]->x       "image size along x coordinate (default 800 or bg)",
  'Y':[y=600]->y       "image size along y coordinate (default 600 or bg)",
  'b':bg->bg           "use background image bg (set image size)",
  'a'->a_flag          "performs affine scaling to fit both image dimensions",
  's'->s_flag          "symmetrize y coordinate (origin is bottomleft)",
  'e'->e_flag          "to mark extremal points",
  'd':[d=2]->d [1,3]   "display mode: 1=points, 2=lines (default), 3=both",
  'o':out<-out         "to save the last view as a Cimage",
  'n'->nodisplay       "no display (useful with -o option)",
  in->in               "input Fcurves",
  notused->window      "Window to view the image (internal use)"
  };
*/

#include <stdio.h>
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"

#ifdef __STDC__
extern void fkbox(Fcurves, float *, float *, float *, float *);
extern Curve disk(float *);
#else
extern void fkbox();
extern Curve disk();
#endif


#define BACKGROUND  255
#define FOREGROUND  128
#define FOREGROUND2 192

#define ZOOMSTEP 2.0   /* magnification for interactive zoom */

/* Parameter structure */
typedef struct {  
  float x,y;
  float cx,cy;
  int s,d,e;
} Parameters;

/*--------------------------------------------------------------*/
/* global variables avoids to pass parameters to fkview_notify  */
/*--------------------------------------------------------------*/
int PARprint;                /* Toggle to print the parameters  */
int oldx1,oldy1;
Parameters cur,save;
Cimage background_image;     /* background image                */
Cimage current_image;        /* image being currently displayed */
Fcurves ref_fcurves;         /* reference Fcurves               */
float zfactor;
Curve dsk1,dsk2;
/*--------------------------------------------------------------*/


/* draw a curve in local referential */
void draw_curve(u,x,y,c,g)
Cimage u;
int x,y;
Curve c;
unsigned char g;
{
  Point_curve p;
  int xx,yy,nx,ny;
  
  nx = u->ncol;
  ny = u->nrow;
  for (p=c->first;p;p=p->next) {
    xx = x+p->x;
    yy = y+p->y;
    if (xx>=0 && xx<nx && yy>=0 && yy<ny) 
      u->gray[yy*nx+xx] = g;
  }
}

/* plot ref_fcurves into current_image using global parameters */

void plot()
{
  Fcurve        c;
  Point_fcurve  p;
  int           ix,iy,nx,ny,onx,ony,n,ok,first;
  float         x1,y1,x2,y2,xmin,ymax,dx,dy,cx,cy;

  if (background_image)
    mw_copy_cimage(background_image,current_image);
  else mw_clear_cimage(current_image,BACKGROUND);

  /***** MAIN LOOP *****/

  for (c=ref_fcurves->first;c;c=c->next) {
    first=1;
    for (p=c->first;p;p=p->next) {
      nx = current_image->ncol/2 + (int) rint ( (p->x-cur.x)*cur.cx );
      ny = (int) rint ( (p->y-cur.y)*cur.cy );
      if (cur.s) ny = current_image->nrow/2 + ny; 
      else ny = current_image->nrow/2 - ny; 
      ok = (nx>=0 && nx<current_image->ncol && 
	    ny>=0 && ny<current_image->nrow);
      if (ok) {

	if (cur.d & 1) 
	  /*** pset point ***/
	  draw_curve(current_image,nx,ny,dsk1,FOREGROUND2);
	
	if (cur.e && (!p->previous || !p->next)) 
	  /*** mark extremity ***/
	  draw_curve(current_image,nx,ny,dsk2,FOREGROUND2);
	
	if ((cur.d & 2) && !first) 
	  /*** draw segment ***/
	  mw_draw_cimage(current_image,onx,ony,nx,ny,FOREGROUND);

	onx = nx;  ony = ny;
 	first = 0;
      } else first=1;
    }
  }
}


/*----------------------------------------------------------------------*/

void fkview_notify_help()

{
  printf("\n\t\tHelp on line\n");

  printf("\nMouse:\n");

  printf("\tLeft button: center at mouse origin\n");
  printf("\tMiddle button: Restore original settings\n");
  printf("\tRight button: zoom x2, centered at mouse origin\n");

  printf("\nKeyboard:\n");
  printf("\tq: Quit.\n");
  printf("\th: Help.\n");
  printf("\tu: Unzoom x2.\n");
  printf("\ts: Symmetrize along the y coordinate.\n");
  printf("\tp: Toggle point/line representation.\n");
  printf("\te: Toggle extremal points.\n");

}


/*     A notify function must return a value ....      */
/*       0 if there was no event catched               */
/*     > 0 if there was an event catched (but Destroy) */
/*      -1 if the event Destroy was catched (or 'Q')   */

int fkview_notify(ImageWindow,param)

Wframe *ImageWindow;
void *param;

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
	if ((x1>=0)&&(x1<current_image->ncol)
	    &&(y1>=0)&&(y1<current_image->nrow))
	  ng = current_image->gray[y1*current_image->nrow+x1];
	else ng = -1;
	sprintf(mess," %3d ",ng);
	WDrawString(ImageWindow,0,10,mess);
	WFlushAreaWindow(ImageWindow,0,0,current_image->ncol-1,12);
      }
    }
  
  refresh = 0;

  switch(event)
    {
    case W_MS_LEFT:
      /*      PARprint = 1 - PARprint;
      if (PARprint == 0)
	{
	  WRestoreImageWindow(ImageWindow,0,0,
			      current_image->ncol,current_image->nrow); 
	  WFlushWindow(ImageWindow);
	}
      */
      cur.x += (float)(x1-current_image->ncol/2)/cur.cx;
      if (cur.s) cur.y += (float)(y1-current_image->nrow/2)/cur.cy;
      else cur.y -= (float)(y1-current_image->nrow/2)/cur.cy;
      refresh = 1;
      break;
      
    case W_MS_MIDDLE:
      cur = save;
      refresh = 1;
      break;

    case W_MS_RIGHT:
      cur.x += ((float)(x1-current_image->ncol/2)/cur.cx)*(1.0-1.0/zfactor);
      if (cur.s) 
	cur.y += ((float)(y1-current_image->nrow/2)/cur.cy)*(1.0-1.0/zfactor);
      else 
	cur.y -= ((float)(y1-current_image->nrow/2)/cur.cy)*(1.0-1.0/zfactor);
      cur.cx *= zfactor;
      cur.cy *= zfactor;
      refresh = 1;
      break;

    case W_RESIZE:
      WLoadBitMapImage(ImageWindow,current_image->gray,
		       current_image->ncol,current_image->nrow); 
      WRestoreImageWindow(ImageWindow,0,0,
			  current_image->ncol,current_image->nrow); 
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
	  
	case 'h': case 'H': fkview_notify_help();
	  break;
	  
	case 'u': case 'U': 
	  cur.cx /=zfactor;
	  cur.cy /=zfactor;
	  refresh = 1;
	  break;

	case 's': case 'S': 
	  cur.s = 1-cur.s;
	  refresh = 1;
	  break;

	case 'p': case 'P': 
	  cur.d ++;
	  if (cur.d==4) cur.d=1;
	  refresh = 1;
	  break;

	case 'e': case 'E': 
	  cur.e = 1-cur.e;
	  refresh = 1;
	  break;

	default:
	  mwerror(WARNING,1,"Unrecognized Key '%c'. Type H for Help.\n",c);
	}    
      break;
    }

  if (refresh) {

    plot();
    if (WLoadBitMapImage(ImageWindow,current_image->gray,
			 current_image->ncol,current_image->nrow)
	!= 0) return(-1);
    WRestoreImageWindow(ImageWindow,0,0,
			current_image->ncol,current_image->nrow); 
    WFlushWindow(ImageWindow);
  }
 
  return(ret);

}

/*----------------------------------------------------------------------*/

void fkview(in,out,x,y,bg,a_flag,s_flag,e_flag,d,nodisplay,window)

Fcurves in;
Cimage out,bg;
int *x,*y,*d;
char *a_flag,*s_flag,*e_flag,*nodisplay,*window;

{
  Wframe *ImageWindow;
  char text[BUFSIZ];
  float x1,y1,x2,y2,r;
  int nx,ny;

  /* test background image */
  if (bg) {
    nx = bg->ncol;
    ny = bg->nrow;
  } else {
    nx = *x;
    ny = *y;
  }
  background_image = bg;

  /* compute normalization */

  fkbox(in,&x1,&y1,&x2,&y2);
  cur.x = (x1+x2)/2.0;
  cur.y = (y1+y2)/2.0;
  cur.cx = (float)nx/(x2-x1);
  cur.cy = (float)ny/(y2-y1);
  if (!a_flag) {if (cur.cx>cur.cy) cur.cx=cur.cy; else cur.cy=cur.cx;}
  cur.cx /= 1.2; cur.cy /= 1.2;
  if (s_flag) cur.s=1; else cur.s=0;
  cur.d = *d;
  if (e_flag) cur.e=1; else cur.e=0;

  /* save initial position */
  save = cur;

  /*--- Initialize the parameters ---*/  
  PARprint = 0;  
  oldx1 = oldy1 = -1;
  zfactor = ZOOMSTEP;
  r=2.5; dsk1=disk(&r);
  r=4.5; dsk2=disk(&r);

  current_image = mw_change_cimage(NULL,ny,nx);
  if (!current_image) 
    mwerror(FATAL,1,"Not Enough Memory.\n");
 
  ref_fcurves = in;
  plot();
  sprintf(text,"%s",in->name);

  if (!nodisplay) {
    /* display window */
    ImageWindow = (Wframe *)
      mw_get_window((Wframe *) window,
		    current_image->ncol,current_image->nrow,50,50,text);
    if (ImageWindow == NULL)
      mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");
    WLoadBitMapImage(ImageWindow,current_image->gray,
		     current_image->ncol,current_image->nrow); 
    WRestoreImageWindow(ImageWindow,0,0,
			current_image->ncol,current_image->nrow); 
    WFlushWindow(ImageWindow);
    
    WSetUserEvent(ImageWindow,W_MS_BUTTON | W_KEYPRESS);
    mw_window_notify(ImageWindow,NULL,fkview_notify);
    
    /* loops on events */
    mw_window_main_loop();
  }

  /* save output if requested */
  if (out) {
    mw_change_cimage(out,current_image->nrow,current_image->ncol);
    memcpy(out->gray,current_image->gray,
	   current_image->nrow*current_image->ncol);
  }
  mw_delete_curve(dsk2);
  mw_delete_curve(dsk1);
}







