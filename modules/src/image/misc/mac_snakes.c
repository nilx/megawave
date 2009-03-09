/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {mac_snakes};
 version = {"1.1"};
 author = {"Lionel Moisan"};
 function = {"Maximizing Average Contrast Snakes for contour detection"};
 usage = {
   'p':[power=1.]->power   "g(s)=|s|^power",
   'n':[niter=1000]->niter "number of iterations",
   's':[step=1.]->step     "evolution step",
   'v'->v                  "verbose mode",
   'V':V->V                "select video mode and specify zoom (e.g. 2)",
    u->u                   "input Fimage",
    in->in                 "input curves (Dlists)",
    out<-mac_snakes        "output curves (modified input)"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for czoom(), fderiv() */

Cimage bg;
double *xt,*yt;
double thepower;
Wframe *win;
Dlists ref;

/* the g function in the energy (energy is int g(Du.n)ds) */
static void gdgh(double t, double *g, double *dg, double *h)
{
  if (t>0.) {
    *g = pow(t,thepower);
    *dg = thepower*(*g)/t;
  } else if (t<0.) {
    *g = pow(-t,thepower);
    *dg = thepower*(*g)/t;
  } else {
    *g = 0.;
    *dg = 0.;
  }
  *h = *g-t*(*dg);
}

/* reparameterize a curve with respect to arclength */
static void param(double *c, int size)
{
  double length,cur,norm,x,y;
  int k,i;

  for (k=0;k<size;k++) {
    xt[k]=c[2*k]; yt[k]=c[2*k+1];
  }
  length = 0.;
  for (k=0;k<size-1;k++) 
  {
    x = xt[k + 1] - xt[k];
    x = yt[k + 1] - yt[k];
    length += sqrt(x * x + y * y);
  }
  length /= (double)(size-1);
  cur = 0.; i = 1;
  for (k=0;k<size-1;k++) {
    x = xt[k + 1] - xt[k];
    y = yt[k + 1] - yt[k];
    cur += (norm = sqrt(x * x + y * y));
    while (cur>=(double)i*length && i<size-1) {
      c[2*i  ] = xt[k+1] + (xt[k]-xt[k+1])*((cur-(double)i*length)/norm);
      c[2*i+1] = yt[k+1] + (yt[k]-yt[k+1])*((cur-(double)i*length)/norm);
      i++;
    }
  }  
}

/* bilinear interpolation */
static double interpolate(Fimage u, double x, double y)
{
  int ix,iy;
  float *p;
  
  x -= 0.5; y -= 0.5;
  ix = floor(x);
  iy = floor(y);
  if (ix<0 || ix>=u->ncol || iy<0 || iy>=u->nrow) return(0);
  x -= (double)ix; y -= (double)iy;
  p = u->gray+iy*u->ncol+ix;
  return( (1.-y)*((1.-x)*(double)p[0]+x*(double)p[1])
	  +y*((1.-x)*(double)p[u->ncol]+x*(double)p[u->ncol+1]) );
}

/* init video mode */
static void init_visu(Fimage u, Dlists in, float zoom)
{
  int order=1;

  bg = mw_fimage_to_cimage(u,NULL);
  ref = mw_copy_dlists(in,NULL);
  czoom(bg,bg,NULL,NULL,&zoom,&order,NULL);
  win = (Wframe *)mw_get_window(NULL,bg->ncol,bg->nrow+12,10,10,bg->name);
  if (!win) mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");
  /*WfillRectangle*/
  WSetUserEvent(win,W_KEYPRESS);
}

/* refresh display */
static int visu(Dlists in, float zoom, int niter, double energy, int refresh)
{
  int i,j;
  char str[100];

  if (refresh) {
    WLoadBitMapColorImage(win,bg->gray,bg->gray,bg->gray,bg->ncol,bg->nrow);
    WRestoreImageWindow(win,0,0,bg->ncol,bg->nrow);
    mw_copy_dlists(in,ref);
  } else {
    WSetColorPencil(win,164); /* red */
    for (j=0;j<in->size;j++) 
      for (i=0;i<in->list[j]->size-1;i++) 
	WDrawLine(win,
		  (int)(in->list[j]->values[2*i  ]*zoom),
		  (int)(in->list[j]->values[2*i+1]*zoom),
		  (int)(in->list[j]->values[2*i+2]*zoom),
		  (int)(in->list[j]->values[2*i+3]*zoom));
  }
  WSetColorPencil(win,84); /* green */
  for (j=0;j<ref->size;j++) 
    for (i=0;i<ref->list[j]->size-1;i++) 
      WDrawLine(win,
		(int)(ref->list[j]->values[2*i  ]*zoom),
		(int)(ref->list[j]->values[2*i+1]*zoom),
		(int)(ref->list[j]->values[2*i+2]*zoom),
		(int)(ref->list[j]->values[2*i+3]*zoom));
  sprintf(str,"iter = %8d     energy = %g             ",niter,energy);
  WSetColorPencil(win,0); /* black */
  WDrawString(win,0,bg->nrow+10,str);
  WFlushWindow(win);
  return(WUserEvent(win)==W_KEYPRESS && WGetKeyboard()==(int)'q');
}

/*------------------------------ MAIN MODULE ------------------------------*/

Dlists mac_snakes(Fimage u, Dlists in, int *niter, double *step, double *power, char *v, float *V)
{
  Fimage imux,imuy,imuxx,imuxy,imuyy;
  int nx,ny,i,j,k,kp,stop;
  double *ux,*uy,*uxx,*uxy,*uyy,*dx,*dy,*dn,g,*dg,*h;
  float mingrad;
  double mx,my,e,length,s,tx,ty,energy;
  int nsize,size,sizemax,closed;

  thepower = *power;
  sizemax = stop = 0;
  for (j=0;j<in->size;j++) 
    if (in->list[j]->size>sizemax) sizemax=in->list[j]->size;
  if (V) init_visu(u,in,*V);

  xt = (double *)malloc(sizemax*sizeof(double));
  yt = (double *)malloc(sizemax*sizeof(double));
  dg = (double *)malloc(sizemax*sizeof(double));
  h = (double *)malloc(sizemax*sizeof(double));
  dx = (double *)malloc(sizemax*sizeof(double));
  dy = (double *)malloc(sizemax*sizeof(double));
  dn = (double *)malloc(sizemax*sizeof(double));
  ux = (double *)malloc(sizemax*sizeof(double));
  uy = (double *)malloc(sizemax*sizeof(double));
  uxx = (double *)malloc(sizemax*sizeof(double));
  uxy = (double *)malloc(sizemax*sizeof(double));
  uyy = (double *)malloc(sizemax*sizeof(double));
  nx = u->ncol; ny = u->nrow;
  imux = mw_change_fimage(NULL,ny,nx);
  imuy = mw_change_fimage(NULL,ny,nx);
  imuxx = mw_change_fimage(NULL,ny,nx);
  imuxy = mw_change_fimage(NULL,ny,nx);
  imuyy = mw_change_fimage(NULL,ny,nx);
  if (!xt || !yt || !dg || !h || !dx || !dy || !dn || !ux || !uy || !uxx
      || !uxy || !uyy || !imux || !imuy || !imuxx || !imuxy || !imuyy)
    mwerror(FATAL,1,"Not enough memory\n");
  
  mingrad = 0.;
  nsize = 8;
  fderiv(u,NULL,NULL,NULL,NULL,imux,imuy,NULL,NULL,&mingrad,&nsize);
  fderiv(imux,NULL,NULL,NULL,NULL,imuxx,imuxy,NULL,NULL,&mingrad,&nsize);
  fderiv(imuy,NULL,NULL,NULL,NULL,NULL,imuyy,NULL,NULL,&mingrad,&nsize);
  
  for (i=0;i<=*niter;i++) {
    
    /********** ONE ITERATION **********/
    energy = 0.;

    for (j=0;j<in->size;j++) {
      
      /********** ONE CURVE **********/
      
      size = in->list[j]->size;
      closed = ((in->list[j]->values[0]==in->list[j]->values[2*size-2] &&
		 in->list[j]->values[1]==in->list[j]->values[2*size-1])?1:0);

      /* pre-computation */
      e = length = 0.;
      for (k=0;k<size-1;k++) {
	/* Gradient of potential */
	mx = 0.5*(in->list[j]->values[2*k  ]+in->list[j]->values[2*k+2]);
	my = 0.5*(in->list[j]->values[2*k+1]+in->list[j]->values[2*k+3]);
	ux[k] = interpolate(imux,mx,my);
	uy[k] = interpolate(imuy,mx,my);
	uxx[k] = interpolate(imuxx,mx,my);
	uxy[k] = interpolate(imuxy,mx,my);
	uyy[k] = interpolate(imuyy,mx,my);
	
	/* energy */
	dx[k] = in->list[j]->values[2*k+2]-in->list[j]->values[2*k  ];
	dy[k] = in->list[j]->values[2*k+3]-in->list[j]->values[2*k+1];
	dn[k] = sqrt(dx[k] * dx[k] + dy[k] * dy[k]);
	if (dn[k]==0.) dn[k]=1.0e-20;
	s = (-uy[k]*dx[k]+ux[k]*dy[k])/dn[k];
	gdgh(s,&g,dg+k,h+k);
	e += g*dn[k];
	h[k] /= dn[k];
	length += dn[k];
      }
      energy += e/length;
      
      /* evolution */
      if (i<*niter) for (k=(closed?0:1);k<size-1;k++) {
	if (k==0) kp=size-2; else kp=k-1;
	tx = (-dg[kp]*uy[kp]+dg[k]*uy[k]
	      +h[kp]*dx[kp]-h[k]*dx[k]
	      +0.5*dg[k ]*(-uxy[k ]*dx[k ]+uxx[k ]*dy[k ])
	      +0.5*dg[kp]*(-uxy[kp]*dx[kp]+uxx[kp]*dy[kp]) )/length
	  -(dx[kp]/dn[kp]-dx[k]/dn[k])*e/(length*length);
	ty = ( dg[kp]*ux[kp]-dg[k]*ux[k]
	       +h[kp]*dy[kp]-h[k]*dy[k]
	       +0.5*dg[k ]*(-uyy[k ]*dx[k ]+uxy[k ]*dy[k ])
	       +0.5*dg[kp]*(-uyy[kp]*dx[kp]+uxy[kp]*dy[kp]) )/length
	  -(dy[kp]/dn[kp]-dy[k]/dn[k])*e/(length*length);
	in->list[j]->values[2*k  ] += *step * tx;
	in->list[j]->values[2*k+1] += *step * ty;
      }
      if (closed) {
	in->list[j]->values[2*size-2] = in->list[j]->values[0];
	in->list[j]->values[2*size-1] = in->list[j]->values[1];
      }
      param(in->list[j]->values,size);
    }
    
    if (V) {
      stop = (WUserEvent(win)==W_KEYPRESS && WGetKeyboard()==(int)'q');
      if (stop) *niter=i;
      if (i%100==0 || i==*niter) visu(in,*V,i,energy,1);
      else if (i%10==0) visu(in,*V,i,energy,0);
    }
    if (i==0 || i==*niter || v) printf("energy: %g\n",energy);
  }
  if (V && !stop) 
    while (WUserEvent(win)!=W_KEYPRESS || WGetKeyboard()!=(int)'q');

  mw_delete_fimage(imuyy); mw_delete_fimage(imuxy); mw_delete_fimage(imuxx);
  mw_delete_fimage(imuy); mw_delete_fimage(imux);
  free(uyy); free(uxy); free(uxx); free(uy); free(ux);
  free(dn); free(dy); free(dx); free(h); free(dg); free(yt); free(xt);

  return(in);
}
