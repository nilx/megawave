/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {fkview};
  version = {"2.2"};
  author = {"Lionel Moisan"};
  function = {"Interactive view of curves (Flists)"};
  usage = {
  'x':[pos_x=50]->x_0  "position of upleft window corner (x), default 50",
  'y':[pos_y=50]->y_0  "position of upleft window corner (y), default 50",
  'X':[size_x=800]->sx "window size (x), default 800",
  'Y':[size_y=600]->sy "window size (y), default 600",
  'r':ref->ref         "use ref to set the scaling",
  'b':bg->bg           "use background image bg",
  'i':[i=1]->i         "interpolation order for bg (0,1,3,-3..-11, default 1)",
  'a'->a               "performs affine scaling to fit both image dimensions",
  's'->s               "symmetrize y axis (origin is bottomleft)",
  'e'->e               "to mark extremal points",
  'd':[d=2]->d [1,3]   "display mode: 1=points, 2=lines (default), 3=both",
  'g':g->g [0,4]       "grid mode: 0=none .. 4=full (default 2 or 0 (if bg))",
  'c':c->c [0,999]     "display color (RGB, 1 decimal digit per channel)",
  'C':C->C [0,999]     "display color for ref (idem, default is no display)",
  'o':out<-out         "to save the last view as a Ccimage",
  'k':curve<-curve     "to save the selected curve as a Flist",
  'n'->n               "no display (useful with -o option)",
  in->in               "input curves (Flists)",
  notused->window      "window to plot the signal (internal use)"

  };
*/
/*----------------------------------------------------------------------
 v2.0: added grid, bg zoom, motion, full lines, ref, colors, etc (L.Moisan)
 v2.1: fixed color bug (L.Moisan)
 v2.2: added -k option plus miscellaneous corrections (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"

/* "plot string" module and associated constants */
#define FONTWIDTH 7
#define FONTHEIGHT 14
extern Ccimage ccputstring();

extern Curve disc();
extern Fimage fcrop();

#define ABS(x)   ((x)>0?(x):(-(x)))

/* color conversions */
#define C_RED(i)   ((((i) / 100) * 255)/9)
#define C_GREEN(i) (((((i) /10) %10)* 255)/9)
#define C_BLUE(i)  ((((i) %10)* 255)/9)

/*------------------- GLOBAL VARIABLES -------------------*/
Wframe *win;                  /* display window */
Ccimage image;                /* image displayed */
Fimage which_curve;           /* index of curve at each pixel */
int nx,ny;                    /* its dimensions */
Flists curves,ref_curves;     /* input/reference curves (Flists) */
double sx1,sx2,sy1,sy2;       /* part to be displayed */
int X1,X2,Y1,Y2;              /* corresponding coordinates in the window */
int draw_mode,grid_mode;      /* modes for plot_curves() */
int size_strleft;             /* max # chars needed for y axis */
int bg_order;                 /* interpolation order for bg */
int in_color,ref_color;       /* input/reference color */
char ref_display,bg_flag;     /* flags */
char a_flag,s_flag,e_flag;    /* flags */
Curve disc1,disc2,disc5;      /* discs to be displayed */
Fimage bg_image,tmp_fimage;   /* for bg display */
int cur_index,cur_display;    /* for curve selection */
int show_all,motion_flag;     /* for motion and single display */
/*--------------------------------------------------------*/

/* compute the rule (graduations) associated to a given interval */
void getrule(a,b,ofs,step,nsub)
     double a,b;
     double *ofs,*step;
     int *nsub;
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

/* modify the virtual window to achieve the real window x/y ratio */
void restore_xyratio()
{
  double ratio,m,d;

  ratio = (double)(X2-X1)/(double)(Y2-Y1);
  if ((sx2-sx1)/(sy2-sy1)>ratio) {
    m = .5*(sy1+sy2);
    d = .5*(sx2-sx1)/ratio;
    sy1 = m-d; sy2 = m+d;
  } else {
    m = .5*(sx1+sx2);
    d = .5*(sy2-sy1)*ratio;
    sx1 = m-d; sx2 = m+d;
  }
}

double trunc(v,ref)
     double v,ref;
{
  ref = v/ref; 
  ref = ABS(ref);
  if (ref < 1.0e-8) return(0.); else return(v);
}

/* draw a line with any coordinates (part can be out of frame) */
void draw_framed(xa,ya,xb,yb,r,g,b,xmin,xmax,ymin,ymax,i)
     int xa,ya,xb,yb,xmin,xmax,ymin,ymax,i;
     unsigned char r,g,b;
{
  double txa,tya,txb,tyb,dx,dy;

  if (xa>=xmin && xa<=xmax && ya>=ymin && ya<=ymax && 
      xb>=xmin && xb<=xmax && yb>=ymin && yb<=ymax) {
    mw_draw_ccimage(image,xa,ya,xb,yb,r,g,b);
    mw_draw_fimage(which_curve,xa,ya,xb,yb,(float)i);
  }

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
      xb = xa + (int)rint(txb*dx);
      yb = ya + (int)rint(txb*dy);
      xa +=     (int)rint(txa*dx);
      ya +=     (int)rint(txa*dy);
      if (xa>=xmin && xa<=xmax && ya>=ymin && ya<=ymax && 
	  xb>=xmin && xb<=xmax && yb>=ymin && yb<=ymax) {
	mw_draw_ccimage(image,xa,ya,xb,yb,r,g,b);
	mw_draw_fimage(which_curve,xa,ya,xb,yb,(float)i);
      }
    }
  }
}

/* draw a curve in local referential */
void draw_curve_framed(x,y,c,r,g,b,xmin,xmax,ymin,ymax,i)
int x,y,xmin,xmax,ymin,ymax,i;
unsigned char r,g,b;
Curve c;
{
  Point_curve p;
  int xx,yy,adr;
  
  for (p=c->first;p;p=p->next) {
    xx = x+p->x;
    yy = y+p->y;
    if (xx>=xmin && xx<=xmax && yy>=ymin && yy<=ymax) {
      adr = yy*nx+xx;
      image->red[adr] = r;
      image->green[adr] = g;
      image->blue[adr] = b;
      which_curve->gray[adr] = (float)i;
    }
  }
}

/* set display background (bg image or white) */
void put_bg()
{
  float v,sx,sy,fbg=255.;
  int adr,x,y;
  unsigned char c;

  mw_clear_fimage(which_curve,-1.);
  mw_clear_ccimage(image,255,255,255);
  if (bg_flag) {
    sx = (float)(X2-X1+1);
    sy = (float)(Y2-Y1+1);
    if (s_flag) 
      fcrop(bg_image,tmp_fimage,&sx,&sy,NULL,&fbg,&bg_order,NULL,
	    (float)sx1,(float)sy1,(float)sx2,(float)sy2);
    else
       fcrop(bg_image,tmp_fimage,&sx,&sy,NULL,&fbg,&bg_order,NULL,
	    (float)sx1,(float)sy2,(float)sx2,(float)sy1);
    for (x=0;x<tmp_fimage->ncol;x++) 
      for (y=0;y<tmp_fimage->nrow;y++) {
	v = tmp_fimage->gray[y*tmp_fimage->ncol+x];
	c = (v<0.?0:v>=256.?255:(unsigned char)v);
	adr = (Y1+y)*image->ncol + X1+x;
	image->red[adr] = image->green[adr] = image->blue[adr] = c;
      }
  } 
}

/* plot one of the curves */
void plot_one_curve(i,color,pcolor,mode)
     int i,color,pcolor,mode;
{
  Flist c;
  int line,j,x,y,ox,oy;
  double v;

  c = curves->list[i];
  line = 0;
  for (j=0;j<c->size;j++) {
    v = (double)c->values[j*c->dim];
    x = X1+(int)((double)(X2-X1)*(v-sx1)/(sx2-sx1));
    v = (double)c->values[j*c->dim+1];
    if (s_flag) y = Y1+(int)((double)(Y2-Y1)*(v-sy1)/(sy2-sy1));
    else        y = Y2+(int)((double)(Y1-Y2)*(v-sy1)/(sy2-sy1));
    
    if (mode & 2) /* lines */
      if (line) 
	draw_framed(ox,oy,x,y,C_RED(color),C_GREEN(color),C_BLUE(color),
		    X1+1,X2-1,Y1+1,Y2-1,i); 
    
    if (e_flag && (j==0 || j==c->size-1)) /* extremal points */
      draw_curve_framed(x,y,disc2,0,0,0,X1+1,X2-1,Y1+1,Y2-1,i);
    
    if (mode & 1) /* points */
      draw_curve_framed(x,y,disc1,C_RED(pcolor),C_GREEN(pcolor),
			C_BLUE(pcolor),X1+1,X2-1,Y1+1,Y2-1,i);
    
    line = 1; ox = x; oy = y;
  }
}

#define STRSIZE 15

/* plot everything: bg, axes, curves */
void plot_curves()
{
  Flist c;
  double xofs,xstep,yofs,ystep,v,ssx1,ssx2,ssy1,ssy2,truncref;
  int i,j,k,x,y,n1,n2,line,ox,oy,fgcolor,bgcolor,size0;
  int xsub,ysub,color,pcolor,mode;
  Ccimage tmp;
  char str[STRSIZE];

  if (grid_mode==0) {
    
    X1 = 0; X2 = nx-1; 
    Y1 = 0; Y2 = ny-1;
    if (!a_flag) restore_xyratio();
    put_bg();
   
  } else {
    
    X2 = nx-20;
    Y1 = 20;
    Y2 = ny-FONTHEIGHT-15;
    fgcolor=600;
    bgcolor=999;
    
    /*---------- DRAW AXES ----------*/

    /* we need to find a common solution to xyratio and strleft */
    j = 0;
    ssx1=sx1; ssx2=sx2; ssy1=sy1; ssy2=sy2;
    do {
      sx1=ssx1; sx2=ssx2; sy1=ssy1; sy2=ssy2;
      size0 = size_strleft;
      X1 = size_strleft*FONTWIDTH+20;
      if (!a_flag) restore_xyratio();
      
      v = 1.; while (sx1==sx2) {sx1-=v; sx2+=v; v*=10.;}
      v = 1.; while (sy1==sy2) {sy1-=v; sy2+=v; v*=10.;}

      getrule(sx1,sx2,&xofs,&xstep,&xsub);
      getrule(sy1,sy2,&yofs,&ystep,&ysub);
      if (!a_flag) {
	xstep = ystep; xsub = ysub;
	xofs = xsub*xstep*floor(sx1/(xsub*xstep));
      }
      
      /* y axis */
      size_strleft = 0;
      truncref = ABS(sy1)+ABS(sy2);
      for (k=0;(v=yofs+(double)k*ystep*(double)ysub)<=sy2;k++) {
	if (s_flag) y = Y1+(int)((double)(Y2-Y1)*(v-sy1)/(sy2-sy1));
	else        y = Y2+(int)((double)(Y1-Y2)*(v-sy1)/(sy2-sy1));
	if (y>=Y1 && y<=Y2) {
	  snprintf(str,STRSIZE,"%g",trunc(v,truncref));
	  i = strlen(str);
	  if (i>size_strleft) size_strleft=i;
	}
      }  
      j++;
    } while (!a_flag && size_strleft!=size0 && j<3);
     
    put_bg();

    for (k=0;(v=yofs+(double)k*ystep)<=sy2;k++) {
      if (s_flag) y = Y1+(int)((double)(Y2-Y1)*(v-sy1)/(sy2-sy1));
      else        y = Y2+(int)((double)(Y1-Y2)*(v-sy1)/(sy2-sy1));
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
	  snprintf(str,STRSIZE,"%g",trunc(v,truncref));
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
	  case 3: mw_draw_ccimage(image,x,Y1,x,Y2,255,180,180);
	  case 2: mw_draw_ccimage(image,x,Y1-7,x,Y1-1,255,0,0);
	  case 1: mw_draw_ccimage(image,x,Y2+1,x,Y2+7,255,0,0);
	  }
	  snprintf(str,STRSIZE,"%g",trunc(v,truncref));
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

  /*---------- PLOT REF_CURVES (if needed) ----------*/

  if (ref_curves && ref_display) 
    for (i=0;i<ref_curves->size;i++) {
      line = 0;
      c = ref_curves->list[i];
      for (j=0;j<c->size;j++) {
	v = (double)c->values[j*c->dim];
	x = X1+(int)((double)(X2-X1)*(v-sx1)/(sx2-sx1));
	v = (double)c->values[j*c->dim+1];
	if (s_flag) y = Y1+(int)((double)(Y2-Y1)*(v-sy1)/(sy2-sy1));
	else        y = Y2+(int)((double)(Y1-Y2)*(v-sy1)/(sy2-sy1));
	if (line) 
	  draw_framed(ox,oy,x,y,C_RED(ref_color),C_GREEN(ref_color),
		      C_BLUE(ref_color),X1+1,X2-1,Y1+1,Y2-1,-1); 
	line = 1; ox = x; oy = y;
      }
    }
  
  /*---------- PLOT CURVES ----------*/

  pcolor = 999-in_color;
  if (show_all) 
    for (i=0;i<curves->size;i++) 
      plot_one_curve(i,in_color,pcolor,(curves->list[i]->size>1?draw_mode:1));
  mode = (curves->list[cur_index]->size>1?draw_mode:1);
  if (cur_display) 
    plot_one_curve(cur_index,900,700,mode);
  else if (!show_all) plot_one_curve(cur_index,in_color,pcolor,mode);
  
}

/* compute initial virtual window */
void init_sxy()
{
  Flist c;
  int i,j,init;
  double x,y,d;

  if (bg_flag) {
    sx1 = sy1 = 0.;
    sx2 = (double)(bg_image->ncol+1);
    sy2 = (double)(bg_image->nrow+1);
  } else {
    init = 0;
    for (i=0;i<curves->size;i++) {
      c = curves->list[i];
      for (j=0;j<c->size;j++) {
	x = (double)c->values[j*c->dim];
	y = (double)c->values[j*c->dim+1];
	if (!init || x<sx1) sx1 = x;
	if (!init || x>sx2) sx2 = x;
	if (!init || y<sy1) sy1 = y;
	if (!init || y>sy2) sy2 = y;
	init = 1;
      }
    }
    if (!init) {sx1=sy1=0.; sx2=sy2=1.;}
    d = (sx2-sx1)*0.01; sx1 -= d; sx2 += d; 
    d = (sy2-sy1)*0.01; sy1 -= d; sy2 += d; 
  }
}


void zoom_sxy(x,y)
     int x,y;
{
  double d;
  
  d = .5*(sx2-sx1);
  sx1 += .5*(double)(x-X1)*(sx2-sx1)/(double)(X2-X1);
  sx2 = sx1 + d;

  d = .5*(sy2-sy1);
  if (s_flag) sy1 += .5*(double)(y-Y1)*(sy2-sy1)/(double)(Y2-Y1);
  else        sy1 += .5*(double)(y-Y2)*(sy2-sy1)/(double)(Y1-Y2);
  sy2 = sy1+d;
}

void unzoom_sxy()
{
  double d;
  
  d = .25*(sx2-sx1);
  sx1 -= d;
  sx2 += d;
  d = .25*(sy2-sy1);
  sy1 -= d;
  sy2 += d;
}

void shift_sx(p)
     double p;
{
  p *= (sx2-sx1<sy2-sy1?sx2-sx1:sy2-sy1);
  sx1 += p;
  sx2 += p;
}
  
void shift_sy(p)
     double p;
{
  p *= (sx2-sx1<sy2-sy1?sx2-sx1:sy2-sy1);
  sy1 += p;
  sy2 += p;
}
  
/* tell which curve is near the selected location */
int curve_selected(x,y)
     int x,y;
{
  Point_curve p;
  int i,besti,bestr2;

  besti = -1; bestr2 = 26;
  for (p=disc5->first;p;p=p->next) 
    if (x+p->x>=0 && x+p->x<nx && y+p->y>=0 && y+p->y<ny) {
      i = (int)which_curve->gray[(y+p->y)*nx+x+p->x];
      if (i!=-1 && p->x*p->x+p->y*p->y<bestr2) {
	besti = i;
	bestr2 = p->x*p->x+p->y*p->y;
      }
    }
  return(besti);
}

/*** refresh display with current image ***/
void redisplay()
{
  WLoadBitMapColorImage(win,image->red,image->green,image->blue,nx,ny);
  WRestoreImageWindow(win,0,0,nx,ny);
  WFlushWindow(win);
}

void help()
{
  printf("\n\t\tHelp on line\n");
  printf("\nMouse:\n");
  printf("\tLeft button:    Select a curve.\n");
  printf("\tMiddle button:  Restore the original display region.\n");
  printf("\tRight button:   Zoom x2 on the selected location.\n");
  printf("\nKeyboard:\n");
  printf("\th:  Help.\n");
  printf("\tq:  Quit.\n");
  printf("\ta:  Toggle all/single display.\n");
  printf("\tb:  Toggle background image.\n");
  printf("\tc:  Toggle current curve display.\n");
  printf("\te:  Toggle extremal points.\n");
  printf("\tg:  Switch grid mode.\n");
  printf("\tm:  Toggle motion.\n");
  printf("\tp:  Switch point/line representation.\n");
  printf("\ts:  Symmetrize y axis.\n");
  printf("\tu:  Unzoom x2.\n");
  printf("\tLeft arrow:   Go Left.\n");
  printf("\tRight arrow:  Go Right.\n");
  printf("\tUp arrow:     Go Up.\n");
  printf("\tDown arrow:   Go Down.\n");
  printf("\tPage Up:      Previous curve.\n");
  printf("\tPage Down:    Next curve.\n");
}

/* handle display events */
int win_notify(window,param)
Wframe *window;
void *param;
{
  int event,ret,x,y,button_mask,redisplay_flag,i;
  int c; /* Key code must be int and not char to handle non-printable keys */

  event = WUserEvent(window); 
  if (event<0) ret=1; else ret=event;
  WGetStateMouse(window,&x,&y,&button_mask);
  redisplay_flag = 1;

  if (motion_flag) cur_index = (cur_index+1)%curves->size;

  switch (event) 
    {

    case W_MS_LEFT:
      i = curve_selected(x,y);
      if (i!=-1) {
	cur_display = 1;
	cur_index = i;
      }
      break;

    case W_MS_RIGHT: zoom_sxy(x,y); break;

    case W_MS_MIDDLE: init_sxy(); break;

    case W_RESIZE:
      nx = win->dx; ny = win->dy;
      image = mw_change_ccimage(image,ny,nx);
      break;

    case W_DESTROY: redisplay_flag = 0; ret = -1; break;

    case W_KEYPRESS:
      c = WGetKeyboard();
      switch(c)
	{
	  /* translate left */
	case XK_Left: case XK_KP_Left: shift_sx(-0.2); break;

	  /* translate right */
	case XK_Right:case XK_KP_Right: shift_sx(0.2); break;

	  /* translate up */
	case XK_Up: case XK_KP_Up: shift_sy(s_flag?-.2:.2);  break;

	  /* translate down */
	case XK_Down: case XK_KP_Down: shift_sy(s_flag?.2:-.2); break;

	  /* decrease current index */
	case XK_Page_Up: case XK_KP_Page_Up: 
	  cur_index = (cur_index+curves->size-1)%curves->size;
	  break;

	  /* increase current index */
	case XK_Page_Down: case XK_KP_Page_Down:
	  cur_index = (cur_index+1)%curves->size;
	  break;

	  /* all / single */
	case 'a': case 'A': show_all = 1-show_all; break;

	  /* background */
	case 'b': case 'B': if (bg_image) bg_flag = 1-bg_flag; break;

	  /* current curve display */
	case 'c': case 'C': cur_display = 1-cur_display; break;

	  /* endpoints */
	case 'e': case 'E': e_flag = 1-e_flag; break;

	  /* shift grid mode */
	case 'g': case 'G': grid_mode = (grid_mode+1)%5; break;

	  /* help */
	case 'h': case 'H': help(); break;

	  /* toggle motion */
	case 'm': case 'M': 
	  motion_flag = 1-motion_flag; 
	  if (show_all) cur_display = 1;
	  break;

	  /* shift draw mode */
	case 'p': case 'P': draw_mode = 1+draw_mode%3; break;

	  /* quit */
	case 'q': case 'Q': ret = -1; redisplay_flag = 0; break;

	  /* symmetrize */
	case 's': case 'S': s_flag = 1-s_flag; break;

	  /* unzoom */
	case 'u': case 'U': unzoom_sxy(); break;

	default:
	  if (!motion_flag) redisplay_flag = 0;
	}
      break;

    default:
      if (!motion_flag) redisplay_flag = 0;
    }

  if (redisplay_flag) {
    plot_curves();
    redisplay();
  }

  return(ret);
}

/*------------------------------ MAIN MODULE ------------------------------*/

void fkview(in,out,sx,sy,ref,bg,i,a,s,e,d,g,c,C,n,window,x_0,y_0,curve)
     int *x_0,*y_0,*sx,*sy,*d,*c,*C,*i,*g;
     Flists in,ref;
     char *window,*n,*a,*s,*e;
     Ccimage *out;
     Fimage bg;
     Flist *curve;
{
  if (!in || !in->size || (in->size==1 && !in->list[0]->size)) 
    mwerror(FATAL,1,"Empty data. ");

  /* Initializations */
  curves = in; ref_curves = ref; bg_image = bg;
  cur_index = cur_display = motion_flag = 0; show_all = 1;
  nx = *sx; ny = *sy; bg_order = *i;
  a_flag = (a?1:0); s_flag = (s?1:0); e_flag = (e?1:0);
  size_strleft = 1;
  disc1 = disc(2.5,NULL);
  disc2 = disc(4.5,NULL);
  disc5 = disc(5.,NULL);

  image = mw_change_ccimage(NULL,ny,nx);
  which_curve = mw_change_fimage(NULL,ny,nx);
  if (!image || !which_curve) mwerror(FATAL,1,"Not enough memory\n");

  draw_mode = *d;
  grid_mode = (g?*g:bg_image?0:2);
  
  if (c) in_color = *c;
  else in_color = 555;

  if (C) {
    ref_display = 1;
    ref_color = *C;
  } else ref_display = 0;

  /* set display window */
  if (bg_image) {
    bg_flag = 1;
    tmp_fimage = mw_new_fimage();
    if (!c) in_color = 90;
  } else bg_flag = 0;
  init_sxy();

  /*** plot curves ***/
  plot_curves();

  if (!n) { 
    /* interactive mode */

    win = (Wframe *)mw_get_window((Wframe *)window,*sx,*sy,*x_0,*y_0,in->name);
    if (!win) mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");
    
    redisplay();
    
    WSetUserEvent(win,W_MS_BUTTON | W_KEYPRESS);
    mw_window_notify(win,NULL,win_notify);
    mw_window_main_loop();

  }

  /* return display image if requested */
  if (out) *out=image;

  /* return selected curve if requested */
  if (curve) *curve = in->list[cur_index];
}


