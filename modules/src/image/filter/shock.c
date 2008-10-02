/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {shock};
 author = {"Lionel Moisan"};
 version = {"1.2"};
 function = {"Rudin shock filter"};
 usage = {
   'n':[n=10]->n               "number of iterations",
   's':[s=0.1]->s[0.0,1.0]     "scale step",
   in->in                      "input image",
   out<-shock                  "output image"
};
*/
/*----------------------------------------------------------------------
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

void iter(u,v,s)
     Fimage u,v;
     float s;
{
  int   i,j,im,i1,jm,j1;
  float new,val;
  float laplacian;

  for (i=0;i<u->ncol;i++) 
    for (j=0;j<u->nrow;j++) {

      if (j==0) jm=1; else jm=j-1;
      if (j==u->nrow-1) j1=u->nrow-2; else j1=j+1;
      if (i==0) im=1; else im=i-1;
      if (i==u->ncol-1) i1=u->ncol-2; else i1=i+1;
      
      laplacian=(u->gray[u->ncol * j  + i1 ]+
		 u->gray[u->ncol * j  + im ]+
		 u->gray[u->ncol * j1 + i  ]+
		 u->gray[u->ncol * jm + i  ]-
	     4.0*u->gray[u->ncol * j  + i ]);
      
      new = u->gray[u->ncol * j  + i  ];
      if (laplacian > 0.0) {
	/* erosion */
	val = u->gray[u->ncol * j  + i1 ]; if (val<new) new = val;
	val = u->gray[u->ncol * j  + im ]; if (val<new) new = val;
	val = u->gray[u->ncol * j1 + i  ]; if (val<new) new = val;
	val = u->gray[u->ncol * jm + i  ]; if (val<new) new = val;
      } else if (laplacian < 0.0) {
	/* dilation */
	val = u->gray[u->ncol * j  + i1 ]; if (val>new) new = val;
	val = u->gray[u->ncol * j  + im ]; if (val>new) new = val;
	val = u->gray[u->ncol * j1 + i  ]; if (val>new) new = val;
	val = u->gray[u->ncol * jm + i  ]; if (val>new) new = val;
      }
      v->gray[u->ncol*j+i] = s * new + (1.0-s) * u->gray[u->ncol*j+i];
    }
}
    

/*-------------------- MAIN MODULE --------------------*/

Fimage shock(in,n,s)
     Fimage in;
     int    *n;
     float  *s;
{
  Fimage u,*old,*new,*tmp;
  int    i;

  u = mw_change_fimage(NULL,in->nrow,in->ncol);
  old = &in;
  new = &u;

  for (i=*n;i--;) {
    iter(*old,*new,*s);
    tmp=old; old=new; new=tmp;
  }
  
  if (*old == in) mw_copy_fimage(in,u);

  return u;
}



