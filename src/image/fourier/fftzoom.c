/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fftzoom};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Zoom an image using Fourier interpolation (zero-padding)"};
   usage = {

   'z':[z=2]->z     "zoom factor (must be a power of 2), default 2",
   'i'->i_flag      "performs inverse zooming",
   in->in           "input Fimage",
   out<-out         "output Fimage"
   };
*/

#include "mw.h"


void fftzoom(in,out,z,i_flag)
Fimage in, out;
int    *z;
char   *i_flag;
{
  Fimage in_re,in_im,out_re,out_im;
  int nx,ny,nxz,nyz,x,y,hx,hy,adr,adrz;
  float z2,factor;

  /* DIRECT FFT */
  in_re = mw_new_fimage();
  in_im = mw_new_fimage();
  fft2d(in,NULL,in_re,in_im,NULL);

  /* COMPUTE DIMENSIONS */
  nx = in_re->ncol;
  ny = in_re->nrow;
  if (i_flag) {
    /* UNZOOM */
    nxz = nx/(*z);
    nyz = ny/(*z);
    z2 = 1.0/(float)((*z)*(*z));
    hx = nxz/2;
    hy = nyz/2;
  } else {
    /* ZOOM */
    nxz = nx*(*z);
    nyz = ny*(*z);
    z2 = (float)((*z)*(*z));
    hx = nx/2;
    hy = ny/2;
  }
  
  /* ALLOCATE AND INITIALIZE IMAGES */
  out_re = mw_change_fimage(NULL,nyz,nxz);
  out_im = mw_change_fimage(NULL,nyz,nxz);
  if (!out_re || !out_im) mwerror(FATAL,1,"Not enough memory.\n");
  mw_clear_fimage(out_re,0.0);
  mw_clear_fimage(out_im,0.0);
  
  /* DISPATCH FOURIER COEFFICIENTS */
  for (x=-hx;x<=hx;x++)
    for (y=-hy;y<=hy;y++) {
      adr  = (x+nx )%nx +((y+ny )%ny )*nx ;
      adrz = (x+nxz)%nxz+((y+nyz)%nyz)*nxz;
      factor = z2 * ((x==hx || x==-hx)?0.5:1.0)*((y==hy || y==-hy)?0.5:1.0);
      out_re->gray[adrz] = in_re->gray[adr] * factor;
      out_im->gray[adrz] = in_im->gray[adr] * factor;
    }

  /* INVERSE FFT */
  fft2d(out_re,out_im,out,NULL,!NULL);
}


