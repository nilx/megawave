/*--------------------------- MegaWave2 Command -----------------------------*/
/* mwcommand
name = {bicontrast};
version = {"1.0"};
author = {"Lionel Moisan"};
function = {"Find optimal contrast change between two images"};
usage = { 
 'v'->verb        "verbose mode",
 'r':r<-r         "output Flist : raw (unconstrained) contrast change",
 'g':g<-g         "output Flist : resulting (nondecreasing) contrast change",
 u->u             "input Fimage 1 (reference)",
 v->v             "input Fimage 2",
 {
   out<-out       "output (Fimage 2 after contrast change g)"
 }
};
*/

#include <stdio.h>
#include "mw.h"

extern Fsignal fvalues();

void bicontrast(u,v,verb,r,g,out)
     Fimage u,v,out;
     char *verb;
     Flist r,g;
{
  Fsignal values;
  Fimage rank;
  int adr,i,size,n,k,nx,ny;
  float *ubar,*unum,min,max,*a,*b,*o,*t,tbar;

  nx = v->ncol;
  ny = v->nrow;
  if (u->ncol!=nx || u->nrow!=ny)
    mwerror(FATAL,1,"The dimensions of u and v should be the same.");

  /* compute rank (in v), ubar, unum */
  rank = mw_new_fimage();
  values = fvalues(NULL,NULL,rank,v);
  size = values->size;
  ubar = (float *)calloc(size,sizeof(float));
  unum = (float *)calloc(size,sizeof(float));
  for (adr=ny*nx;adr--;) {
    i = (int)rank->gray[adr];
    ubar[i] += u->gray[adr];
    unum[i] += 1.0;
  }
  for (i=size;i--;) {
    ubar[i] /= unum[i];
    if (i==size-1 || ubar[i]>max) max = ubar[i];
    if (i==size-1 || ubar[i]<min) min = ubar[i];
  }

  /* initialize cost function : a*t^2 -2 b*t + ... */
  t = (float *)malloc((size+2)*sizeof(float)); 
  a = (float *)malloc((size+2)*sizeof(float)); 
  b = (float *)malloc((size+2)*sizeof(float)); 
  o = (float *)malloc((size+2)*sizeof(float)); 
  n = 1;
  t[0] = min - 1.;
  t[1] = max + 1.;
  a[0] = b[0] = 0.;
  
  /* main loop (compute optimums o[] for cost function) */
  for (i=0;i<size;i++) {
    if (verb) printf("i=%d ubar=%f n=%d\n",i,ubar[i],n);

    for (k=0;k<n;k++) {
      a[k] += unum[i];
      b[k] += unum[i]*ubar[i];
      tbar = b[k]/a[k];
      if (tbar<t[k+1]) {
	t[k+2] = t[n];
	o[i] = t[k+1] = tbar;
	n = k+2;
	a[k+1] = b[k+1] = 0.;
	break;
      }
    }
  }

  /* compute global optimum */
  for (i=size-1;i--;) 
    if (o[i]>o[i+1]) o[i]=o[i+1];
    
  /* compute result if requested */
  if (out) {
    mw_change_fimage(out,ny,nx);
    for (adr=ny*nx;adr--;) 
      out->gray[adr] = o[ (int)rank->gray[adr] ];
  }

  /* export ubar */
  if (r) {
    r = mw_change_flist(r,size,size,2);
    for (i=size;i--;) {
      r->values[i*2  ] = values->values[i];
      r->values[i*2+1] = ubar[i];
    }
  }

  /* export contrast change */
  if (g) {
    g = mw_change_flist(g,size,size,2);
    for (i=size;i--;) {
      g->values[i*2  ] = values->values[i];
      g->values[i*2+1] = o[i];
    }
  }

  free (o); free(b); free(a); free(t);
  free(unum); free(ubar);
}
