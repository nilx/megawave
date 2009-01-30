/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {wiener};
 version = {"2.2"};
 author = {"Lionel Moisan"};
 function = {"Wiener Filter (least squares with H1 regularization)"};
 usage = {
   'W':[lambda=1.]->lambda "weight on regularization term",
   'K':kernel->kernel      "specify blur kernel in Fourier domain",
   'R':rad->rad            "... or radial kernel in Fourier domain",
   'g':g->g                "... or gaussian standart deviation",
   in->in                  "input Fimage",
   out<-out                "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.2: minor bug fixed (missing 2 mw_delete_fimage) (G.Blanchet)
 v2.0: changed -w option to -W, ie weight on REGULARIZATION term (LM)
 v2.1: allow any image size (not only powers of two !) (LM)
 v2.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fft2d() */

/*** build a Gaussian kernel with std g in Fourier domain ***/
static void gausskernel(Fimage kernel, float g)
{
  int nx,ny,x,y,adr;
  double cx,cy;

  nx = kernel->ncol;
  ny = kernel->nrow;
  cx = (double)(g*g)*2.*M_PI*M_PI/(double)(nx*nx);
  cy = (double)(g*g)*2.*M_PI*M_PI/(double)(ny*ny);
  for (x=-nx/2;x<(nx+1)/2;x++)
    for (y=-ny/2;y<(ny+1)/2;y++) {
      adr = (y<0?ny+y:y)*nx+(x<0?nx+x:x);
      kernel->gray[adr] = (float)exp(-cx*(double)(x*x)-cy*(double)(y*y));
    }
}

/*** build a radial kernel in Fourier domain ***/
static void radialkernel(Fimage kernel, Fsignal rad)
{
  int nx,ny,x,y,adr,dist;
  double cx,cy;

  nx = kernel->ncol;
  ny = kernel->nrow;
  cx = 1./(double)(nx*nx);
  cy = 1./(double)(ny*ny);
  for (x=-nx/2;x<(nx+1)/2;x++)
    for (y=-ny/2;y<(ny+1)/2;y++) {
      adr = (y<0?ny+y:y)*nx+(x<0?nx+x:x);
      dist = (int) floor(rad->scale * sqrt(cx * (double) (x * x)
					   + cy * (double) (y * y)) + .5);
      if (dist>=rad->size) kernel->gray[adr] = 0.;
      else kernel->gray[adr] = rad->values[dist];
    }
}

/****************************** MAIN MODULE ******************************/

Fimage wiener(Fimage in, Fimage out, Fimage kernel, Fsignal rad, float *g, float *lambda)
{
  Fimage re,im;
  int x,y,nx,ny,adr,kernel_delete,deblur;
  float c,k,rho2,cx,cy;

  re = mw_new_fimage();
  im = mw_new_fimage();
  fft2d(in,NULL,re,im,(char *)0);

  deblur = (kernel?1:0)+(g?1:0)+(rad?1:0);
  if (deblur>1)
    mwerror(FATAL,1,"Please specify no more than one blur kernel.\n");

  nx = re->ncol; ny = re->nrow;
  kernel_delete = 0;
  if (kernel) {
    if (nx!=kernel->ncol || ny!=kernel->nrow) 
      mwerror(FATAL,1,"Incompatible kernel and input dimensions.\n");;
  } else if (deblur) {
    kernel = mw_change_fimage(NULL,ny,nx); 
    kernel_delete = 1;
    if (g) gausskernel(kernel,*g);
    else radialkernel(kernel,rad);
  }
  out = mw_change_fimage(out,ny,nx); 
  if (!out) mwerror(FATAL,1,"Not enough memory.\n");

  cx = M_PI*M_PI/(float)(nx*nx);
  cy = M_PI*M_PI/(float)(ny*ny);

  if (deblur) 
    for (x=-nx/2;x<(nx+1)/2;x++)
      for (y=-ny/2;y<(ny+1)/2;y++) {
	adr = (y<0?ny+y:y)*nx+(x<0?nx+x:x);
	rho2 = cx*(double)(x*x)+cy*(double)(y*y);
	k = kernel->gray[adr];
	c = (k!=0.?k/(k*k+(*lambda)*rho2):0.);
	re->gray[adr] *= c;
	im->gray[adr] *= c;
      }
  else 
    for (x=-nx/2;x<(nx+1)/2;x++)
      for (y=-ny/2;y<(ny+1)/2;y++) {
	adr = (y<0?ny+y:y)*nx+(x<0?nx+x:x);
	rho2 = cx*(double)(x*x)+cy*(double)(y*y);
	c = 1./(1.+(*lambda)*rho2);
	re->gray[adr] *= c;
	im->gray[adr] *= c;
      }
  
  fft2d(re,im,out,NULL,(char *)1);

  if (kernel_delete) mw_delete_fimage(kernel);
  mw_delete_fimage(im);
  mw_delete_fimage(re);

  return(out);
}

