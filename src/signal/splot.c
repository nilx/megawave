/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {splot};
  version = {"1.6"};
  author = {"Jacques Froment"};
  function = {"Plot on a window a signal of floating points values"};
  usage = {
  'x':[pos_x=50]->x_0  
      "X coordinate for the upper-left corner of the Window (default:50)",
  'y':[pos_y=50]->y_0
      "Y coordinate for the upper-left corner of the Window (default:50)",
  'X':[size_x=500]->sx
      "size in X axis (or width) of the Window (default:500)",
  'Y':[size_y=200]->sy
      "size in Y axis (or height) of the Window (default:200)",
  'N'->no_refresh
      "do not refresh the window (library call)",
  fsignal->signal
      "input fsignal",
   notused->window 
      "Window to plot the signal (internal use)"

  };
*/
/*----------------------------------------------------------------------
 v1.6: added Boxes representation (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"

/* Structure for the Graph */

typedef struct SGraph
{
  int axis;         /* 1 to draw axis, 0 elsewhere */
  int Gx,Gy;        /* Width and Height of the graph */
  int Gdx,Gdy;      /* Position of the graph */
  int n;            /* Number of points in the graph */
  int data_c;       /* Color for the data */
  int scaling;      /* Scale computed automatically (0) or given (1) */
  char Gx_label[BUFSIZ];   /* Comment for the X axis */
  char Gy_label[BUFSIZ];   /* Comment for the Y axis */
  char Gtitle[BUFSIZ];     /* Title of the graph */
  int Gstyle;       /* Style of the points */
}  Graph;


/* Param structure used to send parameters to splot_notify() */

typedef struct splot_SParam 
  {
    Graph graph;
    Fsignal signal;
    float *X,*Y;
  } *splot_Param;


#define SPACE_GRAPH_Y 30
#define GRAPH_MARGIN_Y 60

#define Y_TITLE_HEIGHT 7               /* Height of title font             */
#define PXTICK 4.0                     /* Size (in pixels) of a small tick */

#define MAX(x,y)  ((x) > (y) ? (x) : (y))

static int n_xticks, n_yticks;
static double x_min, x_max, y_min, y_max; /* Scale */
static double x_gmin, y_gmin;
static double x_scale, y_scale;
static double xticks[10], yticks[10];

/* --- Routines used by splot_notify() --- */

/* Compute the fields of the size of the graph */
/* from the size of the window.                */

void Compute_Graph_Size(G,sx,sy)

Graph *G;
int sx,sy;

{
  /* if superpose */
  (*G).Gy = sy - GRAPH_MARGIN_Y - SPACE_GRAPH_Y;
  (*G).Gdy = GRAPH_MARGIN_Y;
  /* if not: tableau uniquement. Cf plot.c */

  (*G).Gx= (int) (sx*5.0/6.0);
  (*G).Gdx= (int) (sx/12.0);
}

/* Compute the location (px,py) in the graph from the location (x,y) */
/* of the mouse in the window.                                       */

locate_graph(G, x, y, px, py)

Graph G;
int x, y;
double *px, *py;

{
  *px = (double)(x - G.Gdy - 3 * PXTICK) / x_scale + x_gmin;
  *py = (double)(G.Gy + Y_TITLE_HEIGHT + G.Gdx - y) / y_scale + y_gmin;
}


do_labels(window,dx, dy, y, x, x_text, y_text, title_text)

     Wframe *window;
     int dx, dy, y, x;
     char *x_text, *y_text, *title_text;
{
	int i, f_x,f_y, pos_x, pos_y;
	char temp_str[80];
	
	dy += PXTICK * 3;
	
	f_x = 7;   
	f_y = Y_TITLE_HEIGHT;
	
	/*
	 * Draw in the x-axis
	 */
	
	pos_x = dy;
	pos_y = y + Y_TITLE_HEIGHT + dx;
	
	WSetSpecialColorPencil(window);
	WDrawLine(window,pos_x, pos_y, 
		  (int)((x_max - x_gmin) * x_scale + .5)+pos_x, pos_y);
	
	/*
	 * First, the x_axis ticks
	 */
	
	pos_y = y + Y_TITLE_HEIGHT + f_y + 3 
	  + 3 * PXTICK + dx;
	
	for (i = 0; i < n_xticks + 1; i++){
		
		sprintf(temp_str, "%g", xticks[i]);
		pos_x = (int) ((xticks[i] - x_gmin) * x_scale);
		pos_x -= (int) ((double) strlen(temp_str) / 2.0 * f_x); 
		pos_x += dy;
		
		WDrawString(window, pos_x, pos_y, temp_str);
		/* Size of the font should be 10 */
	}
	
	/*
	 * Label the x axis
	 */
	
	pos_y = y + Y_TITLE_HEIGHT + 2 * 
	  f_y + 5 + 3 * PXTICK + dx;
	
	pos_x = (int) ((x_max - x_gmin) * x_scale);
	pos_x -= (int) ((double) strlen(x_text) * f_x);
	pos_x += dy;
	
	WDrawString(window, pos_x, pos_y, x_text);
	/* Size of the font should be 10 */

	/*
	 * Draw the y-axis
	 */
	
	pos_x = dy;
	pos_y = y + Y_TITLE_HEIGHT + dx;
	
	WDrawLine(window, pos_x, pos_y, pos_x, 
		  (int)((y_gmin - y_max) * y_scale + .5)+pos_y);
	
	/*
	 * Now, the y axis ticks
	 */
	
	for (i = 0; i < n_yticks + 1; i++) {
		sprintf(temp_str, "%g", yticks[i]);
		
		pos_x = dy - 3 * PXTICK - strlen(temp_str) * f_x;
		if (pos_x < 0) pos_x = 0;
		
		pos_y = y + Y_TITLE_HEIGHT + dx;
		pos_y -= (int)((double) (yticks[i] - y_gmin) * y_scale);
		pos_y += f_y/2.0;
		
		WDrawString(window, pos_x, pos_y, temp_str);
		/* Size of the font should be 10 */

	}
	
	/*
	 * Label the y axis
	 */
	
	pos_x = dy - 3.0 * PXTICK - strlen(y_text) / 2.0 * f_x;
	if (pos_x < 0) pos_x = 0;
	
	pos_y = y + Y_TITLE_HEIGHT + dx;
	pos_y -= (int)((double) (y_max - y_gmin) * y_scale) + 3 * PXTICK;
	
	WDrawString(window, pos_x, pos_y, y_text);
	/* Size of the font should be 10 */
	
	/*
	 * Now do the title
	 */
	
	pos_x = dy + x/2;
	pos_x -= (int) ((double) strlen(title_text)/2.0 * f_x);
	
	pos_y = dx + Y_TITLE_HEIGHT;
	
	WDrawString(window, pos_x, pos_y, title_text);
	/* Size of the font should be 12 */
}



/*
 * The following is derived from the quickdraw source ( with major modif.)
 * David Baraff
 * AT&T Bell Laboratories
 */

#define ALMOST1		.9999999
#define ALMOST1_DIV_5		.1999999

static double pmfbg_stsize();

fbtick(window, dx, dy, y, axis)
     Wframe *window;
     int dx, dy, y;
     int axis;
{
	double xvmin, xvmax;
	double value, rstep2, rmin;
	double xx[200], yy[200];
	int j, k, n12, nn, itype, nn0;
	int temp1,temp2;
	
	n_yticks = -1;
	k = -1;
	dy += PXTICK * 3;
	
	rstep2 = pmfbg_stsize(x_min, x_max, &n12, &xvmin, &xvmax, &rmin, &nn0);
	x_gmin = rmin;
	
	rstep2 = pmfbg_stsize(y_min, y_max, &n12, &xvmin, &xvmax, &rmin, &nn0);
	y_gmin = rmin;
	
	for(value = rmin, nn = nn0, j = 0; j < n12; j++)
	  {
	    if(((value-xvmax)*(value-xvmin)) <= 0)
	      {
		if(fabs(value) < 1.0e-3*MAX(fabs(xvmax),fabs(xvmin)))
		  value = 0;
		
		itype = 1;
	    
		if(nn == 0)
		  {	/*Big tick*/
		    n_yticks++;
		    yticks[n_yticks] = value;
		    itype = 3;
		  }
	    
		k++;
	    
		xx[k] = x_gmin;
		yy[k] = value;
		k++;
		
		xx[k] = -itype * PXTICK * (double) (1.0 / x_scale);
		xx[k] += x_gmin;
		yy[k] = value;
	      }
	    
	    if(nn == 0)
	      nn = 5;
	    
	    nn--;
	    
	    value = rmin + (j + 1) *rstep2;
	    
	  }
	
	
	n_xticks = -1;
	
	rstep2 = pmfbg_stsize(x_min, x_max, &n12, &xvmin, &xvmax, &rmin, &nn0);
	x_gmin = rmin;
	
	for(value = rmin, nn = nn0, j = 0; j < n12; j++)
	  {
	    if((value-xvmax)*(value-xvmin) <= 0)
	      {
		if(fabs(value) < 1.0e-3*MAX(fabs(xvmax),fabs(xvmin)))
		  value = 0;
	    
		itype = 1;
		if(nn == 0)
		  {	/*Big tick*/
		    n_xticks++;
		    xticks[n_xticks] = value;
		    itype = 3;
		  }
		
		k++;
		xx[k] = value;
		yy[k] = y_gmin;
		++k;
	    
		xx[k] = value;
		yy[k] = itype*PXTICK * (double) (1.0 / y_scale);
	    
		yy[k] += y_gmin;
	      }
	  
	    if(nn == 0)
	      nn = 5;
	  
	    nn--;
	  
	    value = rmin+(j+1)*rstep2;
	  }
	
	k++; 
	
	{
	  register int i;
	  
	  WSetSpecialColorPencil(window);
  
	  if (axis == 1) 
	
	    for (i = 0; i < k ; i+= 2){
	      temp1 =  (int)((xx[i] - x_gmin) * x_scale) + dy;
	      temp2 =   y + Y_TITLE_HEIGHT + dx - 
		(int) ((yy[i] - y_gmin) * y_scale);
		
	      WDrawLine(window,temp1,temp2, 
			(int) ((xx[i + 1] - xx[i]) * x_scale)+temp1, 
			(int) ((yy[i + 1] - yy[i]) * y_scale)+temp2);
	      
	    }
	  
	}
	
      }

static double Round[] = { 1.0, 2.0, 2.5, 5.0, 10.0, 20.0 };

static double pmfbg_stsize(vmin, vmax, n12, xvmin, xvmax, rmin , nn0)
	double vmin, vmax;
	double *xvmin, *xvmax, *rmin;
	int *n12, *nn0;
{
	double pstep, log10, rstep, order, power, smin, use1, vdif;
	int i, rmin_old;
	
	vdif = vmax - vmin;
	pstep = fabs(vmax - vmin) / 6;
	log10 = log(10.0);
	order = log(pstep)/log10;
	
	if(order < 0)
	  order = order - ALMOST1;
	
	order = (int)order;
	power = pow(10.0, order);
	
	for(i = 0; i < 6; i++)
  {
	  rstep = Round[i]*power;
	  if(rstep>=pstep)
	    break;
  }
	
	smin = vmin/rstep;
	if(smin < 0)
	  smin = smin - ALMOST1_DIV_5;
	if(vmax < vmin)
	  smin += ALMOST1_DIV_5;	
	*rmin = (int)(5 * smin) / 5.0 ;
	rmin_old = (int)(smin) ;
	*nn0 = (int)((*rmin - rmin_old) * 5) ;
	if(*nn0 <= 0)
	  *nn0 = - *nn0 ;
	else
	  *nn0 = 5 - *nn0 ;
	*rmin *= rstep ;
	use1 = fabs(rstep);
	
	rstep = (vdif > 0) ? use1 : -use1;
	*xvmin = vmin - vdif * 1.0e-5;
	*xvmax = vmax + vdif * 1.0e-5;
	
	*n12 = (6 + 1) * (5 + 1);
	
	return (rstep / 5.0);
}


float *make_X_signal(N,scale,shift)

int N;
float scale,shift;
     
{
  float *output;
  int i;

  output = (float *) malloc(N*sizeof(float));
  if (output == NULL) mwerror(FATAL,1,"Not enough memory\n");
  for (i=0;i<N;i++) output[i] = i*scale+shift;
  return(output);
}

/*+++++++++++++++++++++++++++++++++++*/

plot_XY(window,X,Y,G)

Wframe *window;
float *X, *Y;     /* Points to plot in (X,Y) */
Graph G;          /* Parameters for the graph */

{
  int axis;         /* 1 to draw axis, 0 elsewhere */
  int Gx,Gy;        /* Width and Height of the graph */
  int Gdx,Gdy;      /* Position of the graph */
  int n;            /* Number of points in the graph */
  int data_c;       /* Color for the data */
  int scaling;      /* Scale computed automatically (0) or given (1) */
  char *Gx_label;   /* Comment for the X axis */
  char *Gy_label;   /* Comment for the Y axis */
  char *Gtitle;     /* Title of the graph */
  int Gstyle;       /* Style of the points */

  int x, y, dx, dy, i, j, i0, ix, base_y;
  int curr_x, curr_y, last_x, last_y;

  if (window == NULL) mwerror(INTERNAL,1,"plot_fsignal: NULL window !\n");
  if (X == NULL) mwerror(INTERNAL,1,"plot_fsignal: NULL X vector !\n");
  if (Y == NULL) mwerror(INTERNAL,1,"plot_fsignal: NULL Y vector !\n");

  axis = G.axis;
  Gx=G.Gx;
  Gy=G.Gy;
  Gdx=G.Gdx;
  Gdy=G.Gdy;
  n=G.n;
  data_c=G.data_c;
  scaling=G.scaling;
  Gx_label=G.Gx_label;
  Gy_label=G.Gy_label;  
  Gtitle=G.Gtitle;
  Gstyle=G.Gstyle;

  /* Put copies of often-used variables into local variables.
     Also, define x,y,dx,dy for call to pm_fbrecord.
   */
	
  x = Gx + Gdx;
  y = (window->dy + Gy - Gdy + Y_TITLE_HEIGHT)/2;
  dx = y - Gy - Y_TITLE_HEIGHT;
  dy = Gdx;

  if (scaling == 0) /* Compute the scale */
    {
      for (i = 0, x_min = X[0]; i < n; i++)
	if (x_min > X[i]) x_min = X[i];

      for (i = 0, x_max = X[0]; i < n; i++) 
	if (x_max < X[i]) x_max = X[i];

      for (i = 0, y_min = Y[0]; i < n; i++)
	if (y_min > Y[i]) y_min = Y[i];

      for (i = 0, y_max = Y[0]; i < n; i++) 
	if (y_max < Y[i]) y_max = Y[i];
    }
	
  if ((x_max - x_min) < 0.0000001) x_max += 1, x_min -= 1;
  if ((y_max - y_min) < 0.0000001) y_max += 1, y_min -= 1;
	
  x_scale = (double)(x - dy) / (x_max - x_min) ;
  y_scale = (double)(y - dx - Y_TITLE_HEIGHT) / (y_max - y_min) ;
	
  /* Use  the quickdraw routine to do the labels & ticks. */
  fbtick(window, dx, dy, y, axis);

  if (axis) do_labels(window, dx, dy, y, x, Gx_label, Gy_label, Gtitle);
	
  /* Now, plot the points. */
  i0= 0;
  while((i0 < n) && ((X[i0] < x_min) || (X[i0] > x_max) || (Y[i0] < y_min)
	|| (Y[i0] > y_max))) ++i0;

  last_x = (int) ((X[i0] - x_gmin) * x_scale + 0.5) + dy + PXTICK * 3;
  last_y = y + Y_TITLE_HEIGHT + dx;
  last_y -= (int) ((Y[i0] - y_gmin)* y_scale + 0.5);
	
  WSetColorPencil(window,data_c);
  for (i = i0 ; i < n; i++)
    if ((X[i] >= x_min) && (X[i] <= x_max) && (Y[i] >= y_min) && 
	(Y[i] <= y_max))
      {
	curr_x = (int) ((X[i] - x_gmin) * x_scale + 0.5) + dy + PXTICK * 3;
	curr_y = y + Y_TITLE_HEIGHT + dx;
	curr_y -= (int) ((Y[i] - y_gmin)* y_scale + 0.5);
		
	switch(Gstyle) 

	  {
	  case 0 : /* Points */
	    WDrawPoint(window,curr_x, curr_y);
	    break;

	  case 1 : /* Lines */
	    WDrawLine(window,last_x, last_y, curr_x, curr_y);
	    break;
	    
	  case 2 : /* Boxes */
	    base_y = y+Y_TITLE_HEIGHT+dx-(int)((y_min-y_gmin)*y_scale+0.5);
	    for (ix=last_x;ix<=(last_x+curr_x)/2;ix++)
	      WDrawLine(window, ix, base_y, ix, last_y);
	    for (;ix<=curr_x;ix++)
	      WDrawLine(window, ix, base_y, ix, curr_y);
	    break;
	  }
	
	    
	last_x = curr_x;
	last_y = curr_y;
      }
    	
  /* Flush the Window */
  WFlushWindow(window);
}



/*+++++++++++++++++++++++++++++++++++*/

void splot_notify_help()

{
  printf("\n\t\tHelp on line\n");

  printf("\nMouse:\n");
  printf("\tLeft button: Select the left-border for the drawing.\n");
  printf("\tMiddle button: Restore the original drawing.\n");
  printf("\tRight button: Select the right-border for the drawing.\n");

  printf("\nKeyboard:\n");
  printf("\tQ: Quit.\n");
  printf("\tH: Help.\n");
  printf("\tL: Go Left.\n");
  printf("\tR: Go Right.\n");
  printf("\tZ: Zoom.\n");
  printf("\tU: UnZoom.\n");
  printf("\tP: Point/Line/boxes representation switch.\n");

}

/*     A notify function must return a value ....      */
/*       0 if there was no event catched               */
/*     > 0 if there was an event catched (but Destroy) */
/*      -1 if the event Destroy was catched (or 'Q')   */

int splot_notify(ImageWindow,param)

Wframe *ImageWindow;
void *param;          /* Users's parameters: don't forget the cast ! */

{
  int x1,y1,event,button_mask,ret;
  double px,py,D;
  char c;
  splot_Param P;

  P = (splot_Param) param; /* Cast */

  event = WUserEvent(ImageWindow); /* User's event on ImageWindow */
  if (event < 0) ret=1; else ret=event;
  if (event != W_DESTROY)  
    WGetStateMouse(ImageWindow,&x1,&y1,&button_mask);

  switch(event)
    {
    case W_MS_LEFT:
      locate_graph(P->graph, x1, y1, &px, &py);
      x_min=px;
      (P->graph).scaling=1;
      WClearWindow(ImageWindow);
      plot_XY(ImageWindow,P->X,P->Y,P->graph);
      break;
      
    case W_MS_MIDDLE: 
      (P->graph).scaling=0;
      WClearWindow(ImageWindow);
      plot_XY(ImageWindow,P->X,P->Y,P->graph);
      break;

    case W_MS_RIGHT:
      locate_graph(P->graph, x1, y1, &px, &py);
      x_max=px;
      (P->graph).scaling=1;
      WClearWindow(ImageWindow);
      plot_XY(ImageWindow,P->X,P->Y,P->graph);
      break;

    case W_DESTROY:
      ret=-1;
      break;

    case W_RESIZE:
      WClearWindow(ImageWindow);
      Compute_Graph_Size(&(P->graph),ImageWindow->dx,ImageWindow->dy);
      plot_XY(ImageWindow,P->X,P->Y,P->graph);
      break;

    case W_KEYPRESS:
      c = (char) WGetKeyboard();
      switch(c)
	{
	case 'q': case 'Q': ret =-1;
	  break;

	case 'h': case 'H': splot_notify_help();
	  break;

	case 'p': case 'P': /* Point/Line representation switch */
	  (P->graph).Gstyle=((P->graph).Gstyle+1)%3;
	  WClearWindow(ImageWindow);
	  plot_XY(ImageWindow,P->X,P->Y,P->graph);
	  break;

	case 'l': case 'L': /* go Left */
	  D=x_max-x_min;
	  x_min-=D/10.0;
	  x_max-=D/10.0;
	  (P->graph).scaling=1;
	  WClearWindow(ImageWindow);
	  plot_XY(ImageWindow,P->X,P->Y,P->graph);
	  break;

	case 'r': case 'R': /* go Right */
	  D=x_max-x_min;
	  x_min+=D/10.0;
	  x_max+=D/10.0;
	  (P->graph).scaling=1;
	  WClearWindow(ImageWindow);
	  plot_XY(ImageWindow,P->X,P->Y,P->graph);
	  break;

	case 'u': case 'U': /* Unzoom */
	  D=x_max-x_min;
	  x_min-=D/10.0;
	  x_max+=D/10.0;
	  (P->graph).scaling=1;
	  WClearWindow(ImageWindow);
	  plot_XY(ImageWindow,P->X,P->Y,P->graph);
	  break;

	case 'z': case 'Z': /* Zoom */
	  D=x_max-x_min;
	  x_min+=D/10.0;
	  x_max-=D/10.0;
	  if (x_max < x_min) x_max=x_min;
	  (P->graph).scaling=1;
	  WClearWindow(ImageWindow);
	  plot_XY(ImageWindow,P->X,P->Y,P->graph);
	  break;

	default:
	  mwerror(WARNING,1,"Unrecognized Key '%c'. Type H for Help.\n",c);
	}
      break;
    }

  if (ret == -1)
    {  /* Release the memory */
      free(P->X);
      P->X = NULL;
      free(P);
    }

  return(ret);

}


splot(signal,x_0,y_0,sx,sy,no_refresh,window)

int *x_0,*y_0,*sx,*sy,*no_refresh;
Fsignal signal;
char *window;

{
  Wframe *ImageWindow;
  splot_Param param;
  float *X,*Y;
  Graph G;

  ImageWindow = (Wframe *)mw_get_window((Wframe *) window,*sx,*sy,*x_0,
					*y_0,signal->name);

  if (ImageWindow == NULL)
    mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");

  param = (splot_Param) malloc(sizeof(struct splot_SParam));
  if (param == NULL) mwerror(FATAL,1,"not enough memory\n");
  
  WSetForegroundColorPencil(ImageWindow);

  Y = signal->values;
  X = make_X_signal(signal->size,signal->scale,signal->shift);

  Compute_Graph_Size(&G,*sx,*sy);

  G.axis=1;
  G.n=  signal->size;
  G.data_c=0;
  G.scaling=0;
  strcpy(G.Gx_label,"X");
  strcpy(G.Gy_label,"Y");  
  strcpy(G.Gtitle,signal->cmt);
  G.Gstyle=1;

  WClearWindow(ImageWindow);
  usleep(100); /* Seems to be needed to avoid garbage in the background */
  plot_XY(ImageWindow,X,Y,G);
  
  WSetUserEvent(ImageWindow,W_MS_BUTTON | W_KEYPRESS);

  param->graph=G;
  param->signal=signal;
  param->X=X;
  param->Y=Y;
  mw_window_notify(ImageWindow,(void *) param,splot_notify);
  
  if (!no_refresh)  mw_window_main_loop();

}







