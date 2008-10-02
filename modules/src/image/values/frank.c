/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {frank};
 version = {"1.1"};
 author = {"Lionel Moisan"};
 function = {"Generalized rank of a Fimage"};
 usage = { 
   'c'->c            "normalize as a Cimage (into [0..256[)",
   'w':[w=0.5]->w    "weight: g = w*H + (1-w)*H-",
   'g':g<-g          "output contrast change",
   'r':rank<-rank    "output rank Fimage = g(u)",
   u->u              "input Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

extern Fsignal fvalues();

void frank(u,rank,g,w,c)
     Fimage u,rank;
     Flist g;
     float *w;
     int *c;
{
  Fsignal s,mult;
  double sum;
  float norm;
  int i,n;

  mult = mw_new_fsignal();
  s = fvalues(NULL,mult,rank,u);
  n = s->size;
  g = mw_change_flist(g,n,n,2);
  norm = 1./(float)(u->ncol*u->nrow);
  if (c) norm *= 256.; 
  sum = 0.;
  for (i=0;i<n;i++) {
    sum += (*w)*mult->values[i];
    g->values[i*2  ] = s->values[i];
    g->values[i*2+1] = (float)sum*norm;
    sum += (1.-*w)*mult->values[i];
  }
  if (rank) 
    for (i=rank->nrow*rank->ncol;i--;)
      rank->gray[i] = g->values[2*(int)rank->gray[i]+1];
}
