/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fkeepphase};
 version = {"1.0"};
 author = {"Lionel Moisan"};
 function = {"Change the modulus of the DFT of a Fimage (keep phase)"};
 usage = {
    in->in       "input Fimage (phase information)",
    mod->mod     "input Fimage (modulus information)",
    out<-out     "output Fimage"
}; 
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

extern void fft2d(),fft2dpol();

void fkeepphase(in,mod,out)
     Fimage in,mod,out;
{
  Fimage re,im,rho;
  int adr;
  double r;
  
  re = mw_new_fimage();
  im = mw_new_fimage();
  rho = mw_new_fimage();

  fft2d(in,NULL,re,im,NULL);
  fft2dpol(mod,NULL,rho,NULL,NULL);
  for (adr=in->ncol*in->nrow;adr--;) {
    r = hypot((double)im->gray[adr],(double)re->gray[adr]);
    if (r!=0.) {
      re->gray[adr] *= rho->gray[adr]/r;
      im->gray[adr] *= rho->gray[adr]/r;
    }
  }
  fft2d(re,im,out,NULL,1);

  mw_delete_fimage(rho);
  mw_delete_fimage(im);
  mw_delete_fimage(re);
}
