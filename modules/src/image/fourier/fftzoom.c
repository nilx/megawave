/*--------------------------- MegaWave2 module ----------------------------*/
/* mwcommand
 name = {fftzoom};
 version = {"1.3"};
 author = {"Lionel Moisan"};
 function = {"Zoom an image using Fourier interpolation (zero-padding)"};
 usage = {
   'z':[z=2.]->z    "zoom factor",
   'i'->i_flag      "performs inverse zooming (same as z->1/z)",
   in->in           "input Fimage",
   out<-out         "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.2: changed processing of non-Shannon (N/2) frequencies (L.Moisan)
 v1.3: allow any image size and zoom factors (LM)
 v1.4 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <math.h>
#include "mw.h"
#include "mw-modules.h"


void fftzoom(in,out,z,i_flag)
Fimage in, out;
float  *z;
char   *i_flag;
{
  Fimage in_re,in_im,out_re,out_im;
  int nx,ny,nxz,nyz,x,y,adr,adrz;
  float z2,factor,zoom;

  /* DIRECT FFT */
  in_re = mw_new_fimage();
  in_im = mw_new_fimage();
  fft2d(in,NULL,in_re,in_im,NULL);

  /* COMPUTE DIMENSIONS */
  nx = in_re->ncol;
  ny = in_re->nrow;
  zoom = (i_flag?1./(*z):*z);
  if (zoom<=0.) mwerror(FATAL,1,"Zoom factor must be positive.\n");
  nxz = (int) floor((double)nx*(double)zoom + .5);
  nyz = (int) floor((double)ny*(double)zoom + .5);
  z2 = zoom*zoom;
  
  /* ALLOCATE AND INITIALIZE IMAGES */
  out_re = mw_change_fimage(NULL,nyz,nxz);
  out_im = mw_change_fimage(NULL,nyz,nxz);
  if (!out_re || !out_im) mwerror(FATAL,1,"Not enough memory.\n");
  mw_clear_fimage(out_re,0.0);
  mw_clear_fimage(out_im,0.0);
  
  /* DISPATCH FOURIER COEFFICIENTS */
  if (zoom<1.) {
    /* UNZOOM */
    mw_clear_fimage(out_re,0.);
    mw_clear_fimage(out_im,0.);
    for (x=-nxz/2;x<=nxz/2;x++)
      for (y=-nyz/2;y<=nyz/2;y++) {
        adr  = (x+nx )%nx +((y+ny )%ny )*nx ;
        adrz = (x+nxz)%nxz+((y+nyz)%nyz)*nxz;
        out_re->gray[adrz] += in_re->gray[adr] * z2;
        out_im->gray[adrz] += in_im->gray[adr] * z2;
      }
  } else {
    /* ZOOM */
    for (x=-nx/2;x<=nx/2;x++)
      for (y=-ny/2;y<=ny/2;y++) {
        adr  = (x+nx )%nx +((y+ny )%ny )*nx ;
        adrz = (x+nxz)%nxz+((y+nyz)%nyz)*nxz;
        factor = z2 
	  * ((2*x==nx || 2*x==-nx)?0.5:1.0) 
	  * ((2*y==ny || 2*y==-ny)?0.5:1.0);
        out_re->gray[adrz] = in_re->gray[adr] * factor;
        out_im->gray[adrz] = in_im->gray[adr] * factor;
      }
  }
  
  /* INVERSE FFT */
  fft2d(out_re,out_im,out,NULL,!NULL);
}

