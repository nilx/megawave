/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {prolatef};
 author = {"Lionel Moisan"};
 version = {"1.1"};
 function = {"Create a prolate kernel in Fourier domain"};
 usage = {
 'p'->p          "if set, force nonnegative Fourier transform",
 'c'->c          "if set, use a circular Fourier domain (instead of a square)",
 'n':[n=256]->n  "image size",
 s->s            "size of kernel in spatial domain (odd, e.g. 3)",
 d->d            "relative diameter of kernel in Fourier domain (0<d<=1)",
 ker<-ker        "output resulting kernel in Fourier domain (nxn Fimage)",
 e<-prolatef     "fraction of L^2 energy kept in spatial domain (output)"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fft2d() */

#define ABS(x) ((x)>0?(x):-(x))

float prolatef(s,d,n,ker,p,c)
     int s;
     float d;
     Fimage ker;
     int *n,*p,*c;
{
  Fimage sker,tmp;
  int m,i,x,y,xx,yy,cont,out_domain;
  float rad2,side;
  double norm,energy,last_energy;

  m = *n;
  rad2 = (float)(m*m)*d*d*0.25;
  side = (float)(m)*d*0.5;

  tmp = mw_change_fimage(NULL,m,m);
  sker = mw_change_fimage(NULL,m,m);
  mw_clear_fimage(sker,0.);
  ker = mw_change_fimage(ker,m,m);
  mw_clear_fimage(ker,0.);
  ker->gray[0] = 1.;

  /* MAIN LOOP */
  i = 0;
  do {
    
    /* take inverse Fourier Transform */
       fft2d(ker, NULL, tmp, NULL, (char *) 1);

    /* project on spatial support */
    for (x=s;x--;) {
      xx = (x-s/2+m)%m;
      for (y=s;y--;) {
	yy = (y-s/2+m)%m;
	sker->gray[yy*m+xx] = tmp->gray[yy*m+xx];
      }
    }
    
    /* take direct Fourier Transform */
    fft2d(sker, NULL, ker, NULL, NULL);

    /* project on Fourier support */
    norm = 0.;
    for (x=m;x--;) for (y=m;y--;) {
      xx = (x>m/2?m-x:x);
      yy = (y>m/2?m-y:y);
      if (c) out_domain = ((float)(xx*xx+yy*yy)>rad2);
      else out_domain = ((float)ABS(xx)>side || (float)ABS(yy)>side);
      if (out_domain || (p && ker->gray[y*m+x]<0.)) 
	ker->gray[y*m+x] = 0.;
      else norm += (double)ker->gray[y*m+x]*(double)ker->gray[y*m+x];
    }
    energy = sqrt(norm);
    for (x=m*m;x--;) ker->gray[x] /= energy;

    cont = (i<=10 || (i<=100 && energy>last_energy*1.00001));
    last_energy = energy;
    i++;
  } while (cont);

  mwdebug("%d iterations, energy = %f\n",i,(float)energy);

  /* normalize total weight */
  norm = ker->gray[0];
  for (x=m*m;x--;) ker->gray[x] /= norm;

  mw_delete_fimage(sker);
  mw_delete_fimage(tmp);

  return((float)energy);
}
