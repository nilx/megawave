/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {heat};
 author = {"Lionel Moisan"};
 version = {"1.1"};
 function = {"Heat Equation (finite differences scheme with 4 neighbors)"};
 usage = {
           'n':[n=10]->n     "number of iterations (default: 10)",
           's':[s=0.1]->s    "time step (default: 0.1), can be negative",
	   in->in            "input image",
	   out<-heat         "output image"
};
*/
/*--------------------------------------------------------------------------*/

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


/*--- WARNING : the original image (in) is destroyed by this module -----*/

Fimage heat(in,n,s)
Fimage in;
int    *n;
float  *s;
{
  Fimage u,*old,*new,*tmp;
  int    i;

  u = mw_change_fimage(NULL,in->nrow,in->ncol);
  if (!u) mwerror(FATAL,1,"heat: not enough memory");
  old = &in;
  new = &u;

  for (i=*n;i--;) {
    iter(*old,*new,*s);
    tmp=old; old=new; new=tmp;
  }
  
  if (*old == in) mw_copy_fimage(in,u);
  mw_delete_fimage(in);

  return u;
}




