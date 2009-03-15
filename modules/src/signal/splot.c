/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {splot};
  version = {"2.3"};
  author = {"Lionel Moisan"};
  function = {"Plot a Fsignal"};
  usage = {
  'x':[pos_x=50]->x_0  "position of upleft window corner (x)",
  'y':[pos_y=50]->y_0  "position of upleft window corner (y)",
  'X':[size_x=800]->sx "window size (x)",
  'Y':[size_y=600]->sy "window size (y)",
  'o':out<-out         "to save the last view as a Ccimage",
  'N'->no_refresh      "do not refresh the window (library call)",
  'n'->n               "no display (useful with -o option)",
  in->in               "input fsignal",
  notused->window      "window to plot the signal (internal use)"

  };
*/
/*----------------------------------------------------------------------
 v1.6: added Boxes representation (L.Moisan)
 v1.7: return void (L.Moisan)
 v2.0: complete new version with image output (L.Moisan)
 v2.1: added W_LEAVE test (L.Moisan)
 v2.2: added up and down scrolling with mouse (JF)
 v2.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for ccputstring() */

/* "plot string" module and associated constants */
#define FONTWIDTH 7
#define FONTHEIGHT 14

#define ABS(x)   ((x)>0?(x):(-(x)))

/*------------------- GLOBAL VARIABLES -------------------*/
static Wframe *win;                  /* display window */
static Ccimage image;                /* image displayed */
static Ccimage copy_image;           /* temporary copy of image */
static int nx,ny;                    /* its dimensions */
static Fsignal signal;               /* input signal */
static double sx1,sx2,sy1,sy2;       /* signal part to be displayed */
static int X1,X2,Y1,Y2;              /* corresponding coordinates */
static int draw_mode,grid_mode;      /* modes for plot_signal() */
static int selrect,fx1,fx2,fy1,fy2;  /* for rectangle selection */
/*--------------------------------------------------------*/

/* compute the rule (graduations) associated to a given interval */
static void getrule(double a, double b, double *ofs, double *step, int *nsub)
{
  double x,r;

  x = b-a; r=1.0;
  if (x<=0.) mwerror(FATAL,1,"(getrule): empty interval [%f,%f]\n",a,b);
  while (x<=3.) {x*=10.; r*=10.;}
  while (x>31.) {x/=10.; r/=10;}
  if (x<=5.) {*step=1./r; *nsub=5;}
  else if (x<=15.) {*step=2./r; *nsub=2;}
  else {*step = 5./r; *nsub=5;}
  *ofs = *step*floor(a/(*step));
  *step /= (double)*nsub;
}

/* compute max interval [n1,n2] such that [sx1,sx2] contains x([n1,n2]) */
static void interval(int *n1, int *n2)
{
  *n1 = ceil ((sx1-(double)signal->shift)/(double)signal->scale);
  *n2 = floor((sx2-(double)signal->shift)/(double)signal->scale);
  if (*n1<0) *n1=0;
  if (*n2>=signal->size) *n2=signal->size-1;
}

static int minmax(double *min, double *max)
{
  int n1,n2,i;
  double v;

  interval(&n1,&n2);
  if (n1>n2) return(1);
  *min = *max = (double)signal->values[n1];
  for (i=n1+1;i<=n2;i++) {
    v = (double)signal->values[i];
    if (v<*min) *min=v;
    if (v>*max) *max=v;
  }
  return(0);
}

static double trunc(double v, double ref)
{
  ref = v/ref; 
  ref = ABS(ref);
  if (ref < 1.0e-8) return(0.); else return(v);
}

/* draw a line with any coordinates (part can be out of frame) */
static void draw_framed_ccimage(Ccimage u, int xa, int ya, int xb, int yb, unsigned char r, unsigned char g, unsigned char b, int xmin, int xmax, int ymin, int ymax)
{
  double txa,tya,txb,tyb,dx,dy;

  if (xa>=xmin && xa<=xmax && ya>=ymin && ya<=ymax && 
      xb>=xmin && xb<=xmax && yb>=ymin && yb<=ymax) 
    mw_draw_ccimage(u,xa,ya,xb,yb,r,g,b);

  else {
    txa = tya = 0.; txb = tyb = 1.; 
    dx = (double)(xb-xa);
    dy = (double)(yb-ya);
    if (dx) {
      txa = (double)((xa<xmin?xmin:(xa>xmax?xmax:xa))-xa)/dx;
      txb = (double)((xb<xmin?xmin:(xb>xmax?xmax:xb))-xa)/dx;
    }
    if (dy) {
      tya = (double)((ya<ymin?ymin:(ya>ymax?ymax:ya))-ya)/dy;
      tyb = (double)((yb<ymin?ymin:(yb>ymax?ymax:yb))-ya)/dy;
    }
    if (tya>txa) txa=tya;
    if (tyb<txb) txb=tyb;
    if (txa<txb) {
      xb = xa + floor(txb * dx + .5);
      yb = ya + floor(txb * dy + .5);
      xa +=     floor(txa * dx + .5);
      ya +=     floor(txa * dy + .5);
      if (xa>=xmin && xa<=xmax && ya>=ymin && ya<=ymax && 
	  xb>=xmin && xb<=xmax && yb>=ymin && yb<=ymax) 
	mw_draw_ccimage(u,xa,ya,xb,yb,r,g,b);
    }
  }
}

#define STRSIZE 15

static void plot_signal(void)
{
  double xofs,xstep,yofs,ystep,v,truncref;
  int i,k,x,y,n1,n2,line,zero,x1,x2,ox,oy,fgcolor,bgcolor,max,xsub,ysub;
  char str[STRSIZE];

  mw_clear_ccimage(image,255,255,255);

  if (grid_mode==0) {
    
    X1 = 1; X2 = nx-2; 
    Y1 = 1; Y2 = ny-2;

  } else {
    
    /*---------- DRAW AXES ----------*/
    getrule(sx1,sx2,&xofs,&xstep,&xsub);
    getrule(sy1,sy2,&yofs,&ystep,&ysub);
    
    X2 = nx-20;
    Y1 = 20;
    Y2 = ny-FONTHEIGHT-15;
    fgcolor=600;
    bgcolor=999;
    
    /* y axis */
    truncref = ABS(sy1)+ABS(sy2);
    max = 0;
    for (k=0;(v=yofs+(double)k*ystep*(double)ysub)<=sy2;k++) {
      y = Y2+(int)((double)(Y1-Y2)*(v-sy1)/(sy2-sy1));
      if (y>=Y1 && y<=Y2) {
	sprintf(str,"%g",trunc(v,truncref));
	i = strlen(str);
	if (i>max) max=i;
      }
    }  
    X1 = max*FONTWIDTH+25;
    for (k=0;(v=yofs+(double)k*ystep)<=sy2;k++) {
      y = Y2+(int)((double)(Y1-Y2)*(v-sy1)/(sy2-sy1));
      if (y>=Y1 && y<=Y2) {
	if (k%ysub) {
	  switch(grid_mode) {
	  case 4: mw_draw_ccimage(image,X1,y,X2,y,200,255,200);
	  case 3:
	  case 2: mw_draw_ccimage(image,X2+1,y,X2+4,y,255,0,0);
	  case 1: mw_draw_ccimage(image,X1-4,y,X1-1,y,255,0,0);
	  }
	} else {
	  switch(grid_mode) {
	  case 4:
	  case 3: mw_draw_ccimage(image,X1,y,X2,y,255,200,200);
	  case 2: mw_draw_ccimage(image,X2+1,y,X2+7,y,255,0,0);
	  case 1: mw_draw_ccimage(image,X1-7,y,X1-1,y,255,0,0);
	  }
	  sprintf(str,"%g",trunc(v,truncref));
	  ccputstring(image,X1-7-FONTWIDTH*strlen(str),y-FONTHEIGHT/2,
		      &fgcolor,&bgcolor,NULL,str);
	}
      }
    }  
    
    /* x axis */
    truncref = ABS(sx1)+ABS(sx2);
    for (k=0;(v=xofs+(double)k*xstep)<=sx2;k++) {
      x = X1+(int)((double)(X2-X1)*(v-sx1)/(sx2-sx1));
      if (x>=X1 && x<=X2) {
	if (k%xsub) {
	  switch(grid_mode) {
	  case 4: mw_draw_ccimage(image,x,Y1,x,Y2,200,255,200);
	  case 3:
	  case 2: mw_draw_ccimage(image,x,Y1-4,x,Y1-1,255,0,0);
	  case 1: mw_draw_ccimage(image,x,Y2+1,x,Y2+4,255,0,0);
	  }
	} else {
	  switch(grid_mode) {
	  case 4:
	  case 3: mw_draw_ccimage(image,x,Y1,x,Y2,255,200,200);
	  case 2: mw_draw_ccimage(image,x,Y1-7,x,Y1-1,255,0,0);
	  case 1: mw_draw_ccimage(image,x,Y2+1,x,Y2+7,255,0,0);
	  }
	  sprintf(str,"%g",trunc(v,truncref));
	  ccputstring(image,x-strlen(str)*FONTWIDTH/2,Y2+7,
		      &fgcolor,&bgcolor,NULL,str);
	}
      }
    }
    
    mw_draw_ccimage(image,X1-1,Y1-1,X1-1,Y2+1,255,0,0);
    mw_draw_ccimage(image,X1-1,Y2+1,X2+1,Y2+1,255,0,0);
    
    if (grid_mode!=1) {
      mw_draw_ccimage(image,X2+1,Y1,X2+1,Y2+1,255,0,0);
      mw_draw_ccimage(image,X1-1,Y1-1,X2+1,Y1-1,255,0,0);
    }
  }

  /*---------- PLOT SIGNAL ----------*/
  interval(&n1,&n2);
  if (n1>0) n1--;
  if (n2+1<signal->size) n2++;
  zero = Y2+(int)((double)(Y1-Y2)*(0.-sy1)/(sy2-sy1));
  if (zero<=Y1) zero=Y1+1; 
  if (zero>=Y2) zero=Y2-1;
  line = 0;

  for (i=n1;i<=n2;i++) {
    v = (double)signal->shift+(double)i*(double)signal->scale;
    x = X1+(int)((double)(X2-X1)*(v-sx1)/(sx2-sx1));
    v = (double)signal->values[i];
    y = Y2+(int)((double)(Y1-Y2)*(v-sy1)/(sy2-sy1));

    switch(draw_mode) {

    case 2: /* boxes */
      v = (double)signal->shift+(double)i*(double)signal->scale;
      x1 = X1+(int)((double)(X2-X1)*(v-sx1)/(sx2-sx1))+1;
      v = (double)signal->shift+((double)i+1.)*(double)signal->scale;
      x2 = X1+(int)((double)(X2-X1)*(v-sx1)/(sx2-sx1));
      if (x1<=X1) x1=X1+1;
      if (x2>=X2) x2=X2-1;
      if (y<=Y1) y=Y1+1; 
      if (y>=Y2) y=Y2-1;
      if (y!=zero && x1<X2 && x2>X1)
	for (x=x1;x<=x2;x++)
	  mw_draw_ccimage(image,x,zero,x,y,0,0,0); 
      break;

    case 1: /* lines */
      if (line) {
	draw_framed_ccimage(image,ox,oy,x,y,0,0,0,X1+1,X2-1,Y1+1,Y2-1); 
	break;
      }

    case 0: /* points */
      if (x>X1 && x<X2 && y>Y1 && y<Y2) mw_plot_ccimage(image,x,y,0,0,0);
    }

    line = 1; ox = x; oy = y;
  }
}

static void draw_selection_rectangle(void)
{
  mw_draw_ccimage(image,fx1,fy1,fx2,fy1,0,0,255);
  mw_draw_ccimage(image,fx2,fy1,fx2,fy2,0,0,255);
  mw_draw_ccimage(image,fx2,fy2,fx1,fy2,0,0,255);
  mw_draw_ccimage(image,fx1,fy2,fx1,fy1,0,0,255);
}

static void rescale_sy(void)
{
  double d;
  
  if (minmax(&sy1,&sy2)) return;
  d = (sy2-sy1)*0.025; 
  if (d==0.) 
  {
    if (sy1>0.) {sy1=0.; sy2*=2.;}
    else if (sy1<0.) {sy1*=2.; sy2=0.;}
    else {sy1=-1.; sy2=1.;}
  }
  do {
    sy1 -= d; sy2 += d; 
    d *= 2.;
  } while (sy1==sy2);
}

static void init_sxy(void)
{
  double d;
  
  sx1 = (double)signal->shift;
  sx2 = (double)signal->shift+(double)(signal->size-1)*(double)signal->scale;
  if (draw_mode==2) sx2+=(double)signal->scale;
  d = (sx2-sx1)*0.01; sx1 -= d; sx2 += d; 
  rescale_sy();
}

static void zoom_sx(int x)
{
  double d,m;
  
  d = .5*(sx2-sx1);
  m = sx1 + .5*(double)(x-X1)*(sx2-sx1)/(double)(X2-X1);
  if (m!=m+d) {
    sx1 = m;
    sx2 = m+d;
  }
}

static void unzoom_sxy(void)
{
  double d;
  
  d = .25*(sx2-sx1);
  sx1 -= d;
  sx2 += d;
  d = .25*(sy2-sy1);
  sy1 -= d;
  sy2 += d;
}

static void shift_sx(double p)
{
  p *= sx2-sx1;
  sx1 += p;
  sx2 += p;
}
  
static void shift_sy(double p)
{
  p *= sy2-sy1;
  sy1 += p;
  sy2 += p;
}
  
static void select_region(void)
{
  double tx1,tx2,ty1,ty2;

  tx1 = sx1 + (double)(fx1-X1)*(sx2-sx1)/(double)(X2-X1);
  tx2 = sx1 + (double)(fx2-X1)*(sx2-sx1)/(double)(X2-X1);
  ty1 = sy1 + (double)(fy1-Y2)*(sy2-sy1)/(double)(Y1-Y2);
  ty2 = sy1 + (double)(fy2-Y2)*(sy2-sy1)/(double)(Y1-Y2);
  if (tx1==tx2 || ty1==ty2) return;
  if (tx1<tx2) {sx1=tx1; sx2=tx2;} else {sx1=tx2; sx2=tx1;}
  if (ty1<ty2) {sy1=ty1; sy2=ty2;} else {sy1=ty2; sy2=ty1;}
}

/*** refresh display with current image ***/
static void redisplay(void)
{
  WLoadBitMapColorImage(win,image->red,image->green,image->blue,nx,ny);
  WRestoreImageWindow(win,0,0,nx,ny);
  WFlushWindow(win);
}

static void help(void)
{
  printf("\n\t\tHelp on line\n");
  printf("\nMouse:\n");
  printf("\tLeft button:    Select a region of interest.\n");
  printf("\tMiddle button:  Restore the original display region.\n");
  printf("\tRight button:   Zoom on the selected x location.\n");
  printf("\nKeyboard:\n");
  printf("\tQ:  Quit.\n");
  printf("\tH:  Help.\n");
  printf("\tG:  Grid mode (switch).\n");
  printf("\tS:  Scaling (zoom y axis).\n");
  printf("\tU:  Unzoom (x and y).\n");
  printf("\tP:  Point/Line/boxes representation (switch).\n");
  printf("\tL/Left arrow:   Go Left.\n");
  printf("\tR/Right arrow:  Go Right.\n");
  printf("\tUp arrow:       Go Up.\n");
  printf("\tDown arrow:     Go Down.\n");
}

/* handle display events */
static int win_notify(Wframe *window, void *param)
{
  int event,ret,x,y,button_mask,redisplay_flag;
  int c; /* Key code must be int and not char to handle non-printable keys */
  
  /* FIXME: unused parameter */
  param = param;
  event = WUserEvent(window); 
  if (event<0) ret=1; else ret=event;
  WGetStateMouse(window,&x,&y,&button_mask);
  redisplay_flag = 1;

  if (selrect) {
    mw_copy_ccimage(copy_image,image);
    fx2 = x; fy2 = y;
  }

  switch (event) 
    {

    case W_MS_LEFT: 
      if (selrect) {
	selrect = 0;
	select_region();
      } else {
	selrect = 1;
	copy_image = mw_change_ccimage(copy_image,image->nrow,image->ncol);
	if (!copy_image) mwerror(FATAL,1,"Not enough memory\n");
	mw_copy_ccimage(image,copy_image);
	fx1 = fx2 = x; fy1 = fy2 = y;
      }
      break;

    case W_MS_RIGHT: 
      if (selrect) selrect=0; else zoom_sx(x); break;

    case W_MS_MIDDLE:       
      if (selrect) selrect=0; else init_sxy(); break;

    case W_MS_UP:       
      shift_sy(0.1);  break;
   
    case W_MS_DOWN:       
      shift_sy(-0.1);  break;

    case W_LEAVE:       
      selrect=0;

   case W_RESIZE:
      nx = win->dx; ny = win->dy;
      image = mw_change_ccimage(image,ny,nx);
      break;

    case W_DESTROY: redisplay_flag = 0; ret = -1; break;

    case W_KEYPRESS:
      if (selrect) {selrect=0; break;}
      c = WGetKeyboard();
      switch(c)
	{
	  /* translate left */
	case 'l':case 'L':case XK_Left: case XK_KP_Left: shift_sx(-0.1); break;

	  /* translate right */
	case 'r':case 'R':case XK_Right:case XK_KP_Right: shift_sx(0.1); break;

	  /* translate up */
	case XK_Up: case XK_KP_Up:     shift_sy(0.1);  break;

	  /* translate down */
	case XK_Down: case XK_KP_Down: shift_sy(-0.1); break;

	  /* shift grid mode */
	case 'g': case 'G': grid_mode = (grid_mode+1)%5; break;

	  /* help */
	case 'h': case 'H': help(); break;

	  /* shift draw mode */
	case 'p': case 'P': 
	  if (draw_mode==2 && sx2-(double)signal->scale>sx1) 
	    sx2-=(double)signal->scale;
	  draw_mode = (draw_mode+1)%3; 
	  if (draw_mode==2) sx2+=(double)signal->scale;
	  break;

	  /* scaling */
	case 's': case 'S': rescale_sy(); break;

	  /* unzoom */
	case 'u': case 'U': unzoom_sxy(); break;

	  /* quit */
	case 'q': case 'Q': ret = -1; redisplay_flag = 0; break;

	default:
	  redisplay_flag = 0;
	}
      break;

    default:
      redisplay_flag = 0;
    }

  if (redisplay_flag) plot_signal();
  if (selrect) draw_selection_rectangle();
  if (redisplay_flag || selrect) redisplay();

  return(ret);
}

/*------------------------------ MAIN MODULE ------------------------------*/

void splot(Fsignal in, int *x_0, int *y_0, int *sx, int *sy, int *no_refresh, char *window, Ccimage *out, char *n)
{
  /* Initializations */
  nx=*sx;
  ny=*sy;
  signal=in;
  init_sxy();

  image = mw_change_ccimage(NULL,ny,nx);
  copy_image = mw_new_ccimage();
  if (!image || !copy_image) mwerror(FATAL,1,"Not enough memory\n");

  draw_mode = 1;
  grid_mode = 4;
  
  plot_signal();

  if (!n) { 
    /* interactive mode */

    win = (Wframe *)mw_get_window((Wframe *)window,*sx,*sy,*x_0,*y_0,in->name);
    if (!win) mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");
    
    redisplay();
    
    WSetUserEvent(win,W_MS_BUTTON | W_KEYPRESS);
    mw_window_notify(win,NULL,win_notify);
    if (!no_refresh)  mw_window_main_loop();

  }

  /* return display image if requested */
  if (out) *out=image;
}


