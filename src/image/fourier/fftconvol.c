/*----------------------Megawave2 Module-----------------------------------*/
/*mwcommand
  name = {fftconvol};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"2D Fourier-convolution of a fimage"};
  usage = {
            in->in           "Input Fimage",
            filter->filter   "convolution filter in Fourier domain (Fimage)",
            out<-out         "Output Fimage"
          };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

extern void fft2d();


/* NB : calling this module with out=in is possible */

int is_a_power_of_two(n)
int n;
{
  if (n<1) return(0);
  while ((n&1)==0) n=(n>>1);
  return(n==1);
}

/*-------------------- MAIN MODULE --------------------*/

Fimage fftconvol(in,filter,out)
     Fimage in,filter,out;
{
  int i,nx,ny;
  Fimage re,im;

  nx = in->ncol;
  ny = in->nrow;
  
  if (!is_a_power_of_two(nx) || !is_a_power_of_two(ny))
    mwerror(USAGE,1,"Image dimensions must be powers of 2 !\n");

  if (filter->ncol!=nx || filter->nrow!=ny) 
    mwerror(USAGE,1,"Input image and filter dimensions do not match !\n");

  re = mw_new_fimage();
  im = mw_new_fimage();

  /* Fourier transform */
  fft2d(in,NULL,re,im,0);

  /* multiplication in Fourier domain */
  for (i=nx*ny;i--;) {
    re->gray[i] *= filter->gray[i];
    im->gray[i] *= filter->gray[i];
  }

  out = mw_change_fimage(out,ny,nx);

  /* inverse Fourier transform */
  fft2d(re,im,out,NULL,1);

  /* free memory */
  mw_delete_fimage(im);
  mw_delete_fimage(re);

  return(out);
}
