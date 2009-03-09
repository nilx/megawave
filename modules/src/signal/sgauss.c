/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {sgauss};
  version = {"1.2"};
  author = {"Lionel Moisan"};
  function = {"Create a Gaussian Fsignal with unit mass"};
  usage = {
   's':size->size      "set size directly ...",
   'p':[prec=4]->prec  "... or specify -log10 signal precision",
   out<-out            "output Fsignal",
   std->std            "standart deviation"
  };
*/
/*----------------------------------------------------------------------
 v1.1: added -p option and return output (L.Moisan)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

Fsignal sgauss(float std, Fsignal out, int *size, float *prec)
{
  int i,n;
  double sum,v;

  if (size) {
    n = *size;
    v = (0.5*(double)(n-1))/(double)(std);
    v = 0.5*v*v/log(10.);
    mwdebug("precision = %g\n",v);
  } else {
    n = 1+2*ceil((double)std*sqrt(*prec*2.*log(10.)));
    mwdebug("size = %d\n",n);
  }

  out = mw_change_fsignal(out, n);
  if (!out) mwerror(FATAL,1,"Not enough memory.");
  sprintf(out->cmt,"Gaussian (standart deviation %g)",std);
  out->shift = -0.5*(float)(n-1);

  if (n==1) {
    out->values[0]=1.0;
  } else {
    /* store Gaussian signal */
    for (i=(n+1)/2;i--;) {
      v = ((double)i+(double)out->shift)/(double)std;
      out->values[i] = out->values[n-1-i] = (float)exp(-0.5*v*v); 
    }
    /* normalize to get unit mass */
    for (sum=0.0,i=n;i--;) sum += (double)out->values[i];
    for (i=n;i--;) out->values[i] /= (float)sum;
  }

  return(out);
}







