/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {tvdenoise2};
version = {"1.1"};
author = {"Lionel Moisan"};
function = {"Image denoising by TV minimization (Chambolle's dual algorithm)"};
usage = {
 'W':[W=10.]->W    "weight on regularization term",
 's':[s=0.25]->s   "time step",
 'r':[r=1e-5]->r   "stop when E(n)>(1-r)*E(n-1)",
 'n':n->n          "or perform a fixed number n of iterations",
 'v'->v            "verbose : print energy and L2 errors at each iteration",
 'V'->V            "select Video mode",
 in->in            "input Fimage",
 out<-out          "output Fimage"
        };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"
#include "mw-x11.h"

int nx,ny;            /* image size (global variable, never changes) */
unsigned char *image; /* image plane for display */
Wframe *win;          /* display window */


static void init_display(char *str)
{
  win = (Wframe *)mw_get_window(NULL,nx,ny+15,10,10,str);
  if (!win) mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");
  WSetUserEvent(win,W_KEYPRESS);
  WSetColorPencil(win,188); /* white */
  WFillRectangle(win,0,0,nx-1,ny+14);
  WSetColorPencil(win,0); /* black */
  WDrawLine(win,0,ny,nx-1,ny);
  WFlushWindow(win);
}

static void display(double *gray, char *str)
{
  double v;
  int adr;

  /* convert image to gray levels */
  for (adr=nx*ny;adr--;) {
    v = (floor)(gray[adr]);
    if (v>255.) v=255.;
    if (v<0.) v=0;
    image[adr] = (unsigned char)v;
  }
  
  WLoadBitMapColorImage(win,image,image,image,nx,ny);
  WRestoreImageWindow(win,0,0,nx,ny);
  WDrawString(win,0,ny+12,str);
  WFlushWindow(win);
}

/* compute u = ref-div(p), E=||u||^2 and update p */
static double energy_evol(double *px, double *py, float *ref, double *u, double lambda, double s)
{
  double d,E,gx,gy,norm;
  int x,y,adr;

  /* compute u (and E) from (px,py) */
  E = 0.;
  for (adr=y=0;y<ny;y++) 
    for (x=0;x<nx;x++,adr++) {
      d = -px[adr]-py[adr];
      if (x>0) d += px[adr-1];
      if (y>0) d += py[adr-nx];
      u[adr] = (double)ref[adr]+d;
      E += d*d;
    }

  /* update (px,py) from u */
  for (adr=y=0;y<ny;y++) 
    for (x=0;x<nx;x++,adr++) {
      gx = ((x<nx-1)?(u[adr+1 ]-u[adr]):0.);
      gy = ((y<ny-1)?(u[adr+nx]-u[adr]):0.);
      norm = sqrt(gy * gy + gx * gx);
      E += lambda*norm;
      norm = 1.+2.*s*norm/lambda;
      px[adr] = (px[adr]-s*gx)/norm;
      py[adr] = (py[adr]-s*gy)/norm;
    }

  return(E);
}

/*------------------------------ MAIN MODULE ------------------------------*/

Fimage tvdenoise2(Fimage in, Fimage out, double *s, int *v, int *n, double *r, double *W, int *V)
{
  double *px,*py,*u,E,oldE;
  int adr,i,cont,stop,key;
  char str[100];

  nx = in->ncol; ny = in->nrow;
  out = mw_change_fimage(out,ny,nx);

  /* memory allocation */
  px = (double *)calloc(nx*ny,sizeof(double));
  py = (double *)calloc(nx*ny,sizeof(double));
  u  = (double *)calloc(nx*ny,sizeof(double));
  if (V) image = (unsigned char *)malloc(nx*ny*sizeof(unsigned char));

  /* initialize display */
  if (V) init_display(in->name);

  i = 0; cont = 1; stop = 0;

  /***** MAIN LOOP *****/
  do {
    E = energy_evol(px,py,in->gray,u,*W,*s);
    if (n) cont = (i<=*n); else cont = (i==0 || E<=oldE*(1-*r));
    sprintf(str,"iteration %d: E=%.10g    ",i,E/(double)(nx*ny));
    oldE = E;
    i++;
    if (v) printf("%s\n",str);
    if (V) {
      display(u,str);
      if (!cont || WUserEvent(win)==W_KEYPRESS) {
	if (cont) key = WGetKeyboard();
	if (!cont || key==' ') { /* pause */
	  while (WUserEvent(win)!=W_KEYPRESS);
	  key = WGetKeyboard();
	}
	switch(key) {

	case (int)'q': /* quit */
	  stop=1; break;

	case (int)'r': /* restart */
	  memset(px,0,nx*ny*sizeof(double));
	  memset(py,0,nx*ny*sizeof(double));
	  i = 0; cont = 1;
	  break;
	}
      }
    }
  } while (cont && !stop);
  /***** END OF MAIN LOOP *****/

  /* convert output */
  for (adr=nx*ny;adr--;) out->gray[adr]=(float)u[adr];

  /* free memory */
  if (V) free(image); 
  free(u); free(py); free(px);

  return out;
}

