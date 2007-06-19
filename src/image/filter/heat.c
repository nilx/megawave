/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {heat};
 author = {"Lionel Moisan"};
 version = {"1.3"};
 function = {"Heat Equation (finite differences scheme with 4 neighbors)"};
 usage = {
           'n':[n=10]->n     "number of iterations",
           's':[s=0.1]->s    "time step (can be negative)",
	   in->in            "input image",
	   out<-out          "output image"
};
*/
/*----------------------------------------------------------------------
 v1.2: syntax change (LM)
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

/* NOTE: calling this module with in=out is possible */

#include <stdio.h>
#include "mw.h"

/*--- One iteration of u<-u+s*Laplacian ---*/
void iter(u,v,s)
     Fimage u,v;
     float  s;
{
  int   i,j,im,i1,jm,j1;
  float laplacian;
  
  for (i=0;i<u->ncol;i++) for (j=0;j<u->nrow;j++) {
    if (j==0) jm=1; else jm=j-1;
    if (j==u->nrow-1) j1=u->nrow-2; else j1=j+1;
    if (i==0) im=1; else im=i-1;
    if (i==u->ncol-1) i1=u->ncol-2; else i1=i+1;
    laplacian = 
      - 4.0  * u->gray[u->ncol * j  + i ]
             + u->gray[u->ncol * j  + im]
             + u->gray[u->ncol * j  + i1]
             + u->gray[u->ncol * jm + i]
             + u->gray[u->ncol * j1 + i];
    v->gray[ v->ncol*j+i ] = u->gray[ u->ncol*j+i ] + s * laplacian;
  }
}


/*------------------------------ MAIN MODULE ------------------------------*/

Fimage heat(in,out,n,s)
     Fimage in,out;
     int    *n;
     float  *s;
{
  Fimage w,*src,*dst,*new;
  int niter;

  if (*n<=0) mwerror(FATAL,1,"The -n option parameter must be positive.");
  else niter=*n;
  out = mw_change_fimage(out,in->nrow,in->ncol);
  if (!out) mwerror(FATAL,1,"heat: not enough memory");
  if (niter>1) {
    w = mw_change_fimage(NULL,in->nrow,in->ncol);
    if (!w) mwerror(FATAL,1,"heat: not enough memory");
  } else w=NULL;
  src=&in; 
  if (niter&1) {  /* if niter is odd */
    dst=&out;
    new=&w;
  } else {        /* if niter is even */
    dst=&w;
    new=&out;
  }

  /* main loop */
  while (niter--) {
    iter(*src,*dst,*s);
    src=dst; dst=new; new=src;
  }

  if (w) mw_delete_fimage(w);

  return(out);
}




