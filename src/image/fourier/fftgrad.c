/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
  name = {fftgrad};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Compute the gradient of an image using Fourier interpolation"};
  usage = {
'x':gradx<-gradx 
      "gradient (x coordinate)",
'y':grady<-grady 
      "gradient (y coordinate)",
'n':gradn<-gradn 
      "gradient norm |Du|",
'p':gradp<-gradp 
      "gradient phase (degrees) in [-180,180] U {mw_not_an_argument (=1.0e9)}",
in->in           
      "input Fimage"
  };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define RADIANS_TO_DEGREES (180.0/M_PI)


void fftgrad(in,gradx,grady,gradn,gradp)
Fimage in,gradx,grady,gradn,gradp;
{
  int nx,ny,x,y,adr;
  float normx,normy,cx,cy,xx,yy;
  Fimage re,im;

  nx = in->ncol;
  ny = in->nrow;

  gradx = mw_change_fimage(gradx,ny,nx);
  if (!gradx) mwerror(FATAL,1,"Not enough Memory.\n");
  else mw_clear_fimage(gradx,0.0);

  grady = mw_change_fimage(grady,ny,nx);
  if (!grady) mwerror(FATAL,1,"Not enough Memory.\n");
  else mw_clear_fimage(grady,0.0);
  
  if (gradn) 
    if (!mw_change_fimage(gradn,ny,nx))
      mwerror(FATAL,1,"Not enough Memory.\n");
    else mw_clear_fimage(gradn,0.0);

  if (gradp) 
    if (!mw_change_fimage(gradp,ny,nx))
      mwerror(FATAL,1,"Not enough Memory.\n");
    else mw_clear_fimage(gradp,mw_not_an_argument);

  normx = 2.0*M_PI/(float)(nx);
  normy = 2.0*M_PI/(float)(ny);

  re = mw_new_fimage();
  im = mw_new_fimage();

  /* Fourier transform */
  fft2d(in,NULL,re,im,0);

  /*** MAIN LOOP ***/
  for (x=0;x<nx;x++)
    for (y=0;y<ny;y++) {

      adr = y*nx+x;

      cx = re->gray[adr];
      cy = im->gray[adr];

      xx = normx * (float)(x>nx/2?x-nx:x);
      yy = normy * (float)(y>ny/2?y-ny:y);

      if (x==nx/2) xx=0.0;
      if (y==ny/2) yy=0.0;

      re->gray[adr] =  yy * cx + xx * cy;
      im->gray[adr] = -xx * cx + yy * cy;
    }

  /* inverse Fourier transform */
  fft2d(re,im,gradx,grady,1);
  
  if (gradp)
    for (adr=nx*ny;adr--;)
      if (gradx->gray[adr]!=0.0 || grady->gray[adr]!=0.0)
	  gradp->gray[adr] = RADIANS_TO_DEGREES *
	    (float)atan2( (double)grady->gray[adr],
			  (double)gradx->gray[adr]);

  mw_delete_fimage(re);
  mw_delete_fimage(im);
}
