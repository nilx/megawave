/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {prolate};
 author = {"Lionel Moisan"};
 version = {"1.1"};
 function = {"Create a prolate kernel"};
 usage = {
   'n':[n=256]->m  "image size (power of 2, default 256)",
   s->s            "size of kernel in spatial domain (odd, e.g. 3)",
   d->d            "relative diameter of kernel in Fourier domain (0<d<=1)",
   out<-out        "resulting kernel (sxs Fimage)"
};
*/
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

extern void fft2d();

Fimage prolate(s,d,m,out)
     int s;
     float d;
     Fimage out;
     int *m;
{
  Fimage re,im,tmp;
  int n,r,i,x,y,xx,yy,cont;
  float v,rad2;
  double norm,energy,last_energy;

  n = *m;
  rad2 = (float)(n*n)*d*d*0.25;

  tmp = mw_change_fimage(NULL,n,n);
  re = mw_new_fimage();
  im = mw_new_fimage();
  out = mw_change_fimage(out,s,s);
  mw_clear_fimage(out,1./(float)s);

  /* MAIN LOOP */
  i = 0;
  do {
    
    /* copy kernel */
    mw_clear_fimage(tmp,0.);
    for (x=s;x--;) for (y=s;y--;)
      tmp->gray[((y-s/2+n)%n)*n+(x-s/2+n)%n] = out->gray[y*s+x];
    
    /* take Fourier Transform */
    fft2d(tmp,NULL,re,im,0);
    
    /* project on Fourier essential support */
    norm = 0.;
    for (x=n;x--;) for (y=n;y--;) {
      xx = (x>n/2?n-x:x);
      yy = (y>n/2?n-y:y);
      if ((float)(xx*xx+yy*yy)>rad2) re->gray[y*n+x] = im->gray[y*n+x] = 0.;
      else norm += (double)re->gray[y*n+x]*(double)re->gray[y*n+x]
	     +(double)im->gray[y*n+x]*(double)im->gray[y*n+x];
    }
    energy = sqrt(norm)/(double)n;
    
    /* take inverse Fourier Transform */
    fft2d(re,im,tmp,NULL,1);
    
    /* copy new kernel and normalize */
    norm = 0.;
    for (x=s;x--;) for (y=s;y--;) {
      v = tmp->gray[((y-s/2+n)%n)*n+(x-s/2+n)%n];
      out->gray[y*s+x] = v;
      norm += (double)v*(double)v;
    }
    norm = sqrt(norm);
    for (x=s*s;x--;) out->gray[x] /= (float)norm;

    cont = (i==0 || (i<=100 && energy>last_energy*1.00001));
    last_energy = energy;
    i++;
  } while (cont);

  printf("%d iterations, energy = %f\n",i,(float)energy);

  /* normalize total weight */
  norm = 0.;
  for (x=s*s;x--;) {
    v = out->gray[x];
    if (v>0) norm += (double)v; else  norm -= (double)v;
  }
  for (x=s*s;x--;) out->gray[x] /= (float)norm;

  mw_delete_fimage(im);
  mw_delete_fimage(re);
  mw_delete_fimage(tmp);

  return(out);
}
