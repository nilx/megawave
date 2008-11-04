/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fthre};
 author = {"Jacques Froment, Lionel Moisan"};
 version = {"2.3"};
 function = {"Threshold/normalize the pixel's gray-levels of a fimage"};
 usage = {
 'n'->norm    "to normalize pixel values from actual to [min,max]",
 'N'->N       "to normalize pixel values from actual or [min,max] to [0,255]",
 'm':min->min "specify minimal pixel value (min)",
 'M':max->max "specify maximal pixel value (max)",
 'p':p->p[0,100] "to discard a ratio of p percent pixels with minimal values",
 'q':q->q[0,100] "to discard a ratio of q percent pixels with maximal values",
 'd':d->d[0,100] "to discard a ratio of d percent pixels with extremal values",
 'a'->aff     "to prevent thresholding (affine normalization only)",
 'l'->linear  "force linear normalization (preserve 0)",
 in->in       "input Fimage", 
 out<-out     "output thresholded/renormalized Fimage"
};
*/
/*----------------------------------------------------------------------
 v2.0: added -N option + some other changes (L.Moisan)
 v2.1: added -p -q -d -a -l options (L.Moisan)
 v2.2 (04/2007): fixed a/aff bug (LM)
 v2.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include  "mw.h"


static int compare(const void *a,const void *b)
{
  float v;

  v = *((float *)a)-*((float *)b);
  return(v>0.?1:(v<0.?-1:0));
}


Fimage fthre(in,out,norm,N,min,max,p,q,d,aff,linear)
     Fimage  in,out;
     char    *norm,*N,*aff,*linear;
     float   *min,*max,*p,*q,*d;
{
  float m0,m1,a,b,w,bestw,*v;
  int i,k,bestk,l,n,adr;

  n = in->nrow*in->ncol;
  out = mw_change_fimage(out,in->nrow,in->ncol);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");

  if (!norm && !N) {

    /* simple thresholding */
    if (p || q || d || aff) 
      mwerror(USAGE,1,"-p -q -d -a options useless for simple thresholding\n");
    if (!m0 && !m1)
      mwerror(USAGE,1,"Please specify at least min or max threshold value\n");
    for (adr=n;adr--;) {
      w = in->gray[adr];
      if (min && w<*min) w=*min;
      if (max && w>*max) w=*max;
      out->gray[adr] = w;
    }

  } else { /* affine normalization and maybe thresholding */

    if (norm && N) 
      mwerror(USAGE,1,"-n and -N options cannot be used together\n");

    i = (p?1:0)+(q?1:0)+(d?2:0);
    if (i==0) {

      if (aff && !linear) 
	mwerror(USAGE,1,"-a option is useless without -p -q -d or -l option\n");
      /* compute min (m0) and max (m1) */
      m0 = m1 = in->gray[0];
      for (adr=n;adr--;) {
	w = in->gray[adr];
	if (w<m0) m0=w;
	if (w>m1) m1=w;
      }

    } else if (i<=2) { /* discard some values */
      
      /* sort values */
      v = (float *)malloc(n*sizeof(float));
      for (adr=n;adr--;) v[adr]=in->gray[adr];
      qsort((void *)v,n,sizeof(float),&compare);
      m0 = v[0]; m1 = v[n-1];

      /* compute (pseudo) min (m0) and max (m1) */
      if (p) {
	k = (int)(0.01*(*p)*(float)n);
	m0 = v[k<0?0:(k>=n?n-1:k)];
      }
      if (q) {
	k = n-1-(int)(0.01*(*q)*(float)n);
	m1 = v[k<0?0:(k>=n?n-1:k)];
      }
      if (d) {
	l = (int)(0.01*(*d)*(float)n);
        if (l<0) l=0;
	if (l>=n) l=n-1;
	for (k=0;k<=l;k++) {
	  w = v[k+n-l-1]-v[k];
	  if (k==0 || w<bestw) {
	    bestw = w;
	    bestk = k;
	  }
	}
	m0 = v[bestk];
	m1 = v[bestk+n-l-1];
      }

      mwdebug("m0=%f m1=%f\n",m0,m1);
      free(v);

    } else mwerror(USAGE,1,"You cannot combine -d with -p or -q option\n");
    
    /* compute affine transform u -> a*u+b */
    if (norm) {
      if (linear) {
	a = *max/(m1==0.?1.:m1);
	b = 0.;
	m0 = *min; m1 = *max; /* for thresholding */
      } else {
	if (!min || !max) 
	  mwerror(USAGE,1,"Please specify min and max values (-n option)\n");
	a = (*max-*min)/(m1-m0==0.?1.:m1-m0);
	b = *min-a*m0;
	m0 = *min; m1 = *max; /* for thresholding */
      }
    } else {
      if (min) m0=*min;
      if (max) m1=*max;
      if (linear) m0=0.;
      a = 255/(m1-m0==0.?1.:m1-m0);
      b = -a*m0;
      m0 = 0.; m1 = 255.; /* for thresholding */
    }

    mwdebug("a=%f b=%f\n",a,b);

    /* apply normalization (and maybe thresholding) */
    for (adr=n;adr--;) {
      w = in->gray[adr]*a+b;
      if (!aff) {
	if (w<m0) w=m0;
	if (w>m1) w=m1;
      }
      out->gray[adr] = w;
    }
  }

  return(out);
}

