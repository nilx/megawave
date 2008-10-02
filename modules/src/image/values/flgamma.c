/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {flgamma};
 version = {"1.2"};
 author = {"Lionel Moisan"};
 function = {"Create a x^gamma signal, 0 <= x/s < n (2-Flist)"};
 usage = {   
   'g':[g=2.]->g      "gamma power",
   'n':[n=256]->n     "number of samples",
   's':[s=1.]->s      "step on x",
   'f':f->f           "apply scale factor to ensure fixed point : out(f)=f",
   out<-out           "output Flist"
};
*/
/*----------------------------------------------------------------------
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

Flist flgamma(out,g,s,n,f)
     Flist out;
     float *g,*s,*f;
     int *n;
{
  int i;
  double x,scale;

  out = mw_change_flist(out,*n,*n,2);
  scale = (f?(double)(*f)/pow((double)(*f),(double)(*g)):1.);
  for (i=0;i<*n;i++) {
    x = *s*(float)i;
    out->values[2*i  ] = x;
    out->values[2*i+1] = (float)(scale*pow((double)x,(double)(*g)));
  }

  return(out);
}
