/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name     = {fsample};
author   = {"Jacques Froment, Regis Monneau"};
function = {"sampling of a fimage"};
version  = {"1.4"};
usage    = {
   'd':[delta=0.]->delta  "offset in input image",
   'n'->norm              "normalize (multiply by step^2)",
   in->in                 "input image",
   out<-out               "sampled image",
   step->step             "new grid step (factor of sampling)"    
           };
*/
/*----------------------------------------------------------------------
 v1.1: return result (L.Moisan)
 v1.2: allow non-integer factor and remove step>=2 condition (L.Moisan)
 v1.3: added -d and -n option (L.Moisan)
 v1.4 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include "mw.h"

#define _(a,i,j)  ((a)->gray[(i)*(a)->ncol+(j)] )

Fimage fsample(in ,out,step,delta,norm)
     Fimage out,in;
     double step,*delta;
     int *norm;
{
  float coeff;
  register int i,j;
  int nr;
  int nc;
  int nr1;
  int nc1;

  coeff = (norm?(float)(step*step):1.);
  nr = in->nrow;
  nc = in->ncol;
  nr1 = nr; while ((int)(floor(*delta+(double)(nr1-1)*step))+1>nr) nr1--;
  nc1 = nc; while ((int)(floor(*delta+(double)(nc1-1)*step))+1>nc) nc1--;

  mwdebug("Input size: nr = %d \t nc = %d\n", nr,nc);
  mwdebug("Output size: nr1 = %d \t nc1 = %d\n", nr1,nc1);

  out = mw_change_fimage(out, nr1, nc1);
  if (out == NULL) mwerror(FATAL,1,"not enough memory.\n");

  for (i=0 ; i<  nr1; i++)
    for (j=0 ; j<  nc1; j++) 
      _(out,i,j) = coeff * _(in,(int)floor(*delta+(double)i*step),(int)floor(*delta+(double)j*step));

  return(out);
}

