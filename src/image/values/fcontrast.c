/*--------------------------- MegaWave2 Command -----------------------------*/
/* mwcommand
name = {fcontrast};
version = {"1.0"};
author = {"Lionel Moisan"};
function = {"Apply a contrast change to a Fimage"};
usage = { 
 in->in           "input Fimage",
 g->g             "contrast change (2-Flist)",
 out<-fcontrast   "result Fimage (modified input)"
};
*/

#include <stdio.h>
#include "mw.h"

float apply_g(g,v)
     Flist g;
     float v;
{
  int i,j,k;
  float a,b,s;

  k = i = g->size-1; 
  if (v>=g->values[2*i] || v<=g->values[0]) 
    j = 0;
  else {    
    do {
      k = (k+1)/2; 
      j = i-k; 
      if (j<0) j=0;
      if (g->values[j*2]>v) i = j;
    } while (k>1);
    j = i-1;
    if (j<0) j=0;
  }
  a = g->values[j*2];
  b = g->values[i*2];
  s = (v-a)/(b-a);

  return ((1-s)*g->values[j*2+1]+s*g->values[i*2+1]);
}

Fimage fcontrast(in,g)
     Fimage in;
     Flist g;
{
  int adr;
  
  for (adr=in->nrow*in->ncol;adr--;) 
    in->gray[adr] = apply_g(g,in->gray[adr]);

  return(in);
}
