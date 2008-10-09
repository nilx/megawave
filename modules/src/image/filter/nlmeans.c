/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {nlmeans};
 version = {"1.2"};
 author = {"Lionel Moisan"};
 function = {"Image denoising by Non-Local means (Buades/Coll/Morel method)"};
 usage = {
   'h':[h=10.]->h  "regularization parameter",
   's':[s=7]->s    "patch side length (an odd integer)",
   'a':a->a        "decay of Euclidean patch distance, default (s-1)/4",
   'd':[d=10]->d   "maximum patch distance",
   'c':[c=1.]->c   "weight for self-patch",
   in->in          "input Fimage",
   out<-out        "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): fixed "s odd" test bug (M.Primet)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

Fimage nlmeans(in,out,h,a,s,d,c)
     Fimage in,out;
     double *h,*a,*c;
     int *s,*d;
{
  int *dadr,*dd,nx,ny,x,y,xp,yp,i,adr,adrp,wsize,ds;
  double *w,*ww,dist,new,sum,e,A;
  register float v;

  if (*s<1 || ((*s)&1)==0) 
    mwerror(FATAL,1,"s must be a positive odd integer.");
  A = (a?*a:((double)(*s)-1.)/4.); 
  A = 2.*A*A; if (A==0.) A=1.;

  nx = in->ncol; ny = in->nrow;
  out = mw_change_fimage(out,ny,nx);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");
  mw_copy_fimage(in,out);

  /* precompute weights */
  wsize = *s*(*s); 
  w = (double *)malloc(wsize*sizeof(double));
  dadr = (int *)malloc(wsize*sizeof(int));
  ds = *s/2; /* patch = [-ds,ds]x[-ds,ds] */
  for(sum=0.,i=0,x=-ds;x<=ds;x++)
    for(y=-ds;y<=ds;y++,i++) {
      dadr[i] = y*nx+x;
      w[i] = exp(-(double)(x*x+y*y)/A);
      sum += w[i];
    }
  for (i=wsize;i--;) w[i] /= sum*2.*(*h)*(*h);

  /* main loop */
  for (x=ds;x<nx-ds;x++)
    for (y=ds;y<ny-ds;y++) {
      adr = y*nx+x;
      new = sum = 0.;
      /* loop on patches */
      for (xp=MAX(x-*d,ds);xp<=MIN(x+*d,nx-1-ds);xp++)
	for (yp=MAX(y-*d,ds);yp<=MIN(y+*d,ny-1-ds);yp++)
	  if ((x-xp)*(x-xp)+(y-yp)*(y-yp)<=(*d)*(*d)) {
	    adrp = yp*nx+xp;
	    for (i=wsize,dist=0.,ww=w,dd=dadr;i--;ww++,dd++) {
	      v = in->gray[adr+*dd]-in->gray[adrp+*dd];
	      dist += *ww*(double)(v*v);
	    }
	    e = (adrp==adr?*c:exp(-dist));
	    new += e*(double)in->gray[adrp];
	    sum += e;
	  }
      out->gray[adr] = (float)(new/sum);
    }
  free(dadr);
  free(w);

  return(out);
}
