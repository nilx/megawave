/*-------------------------- MegaWave2  Module ------------------------------*/
/* mwcommand
name     = {csample};
author   = {"Jacques Froment, Regis Monneau"};
function = {"sampling of a cimage"};
version  = {"1.2"};
usage    = {
             in->in         "input image",
             out<-out       "sampled image",
             step->step     "new grid step (factor of sampling)"    
           };
*/
/*----------------------------------------------------------------------
 v1.1: return result (L.Moisan)
 v1.2: allow non-integer factor and remove step>=2 condition (L.Moisan)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include "mw.h"

#define _(a,i,j)  ((a)->gray[(i)*(a)->ncol+(j)] )

Cimage csample(in ,out,step)
     Cimage out,in;
     double step;
{
  register int i,j;
  int nr;
  int nc;
  int nr1;
  int nc1;
  
  nr = in->nrow;
  nc = in->ncol;
  nr1 = nr; while ((int)(floor((double)(nr1-1)*step))+1>nr) nr1--;
  nc1 = nc; while ((int)(floor((double)(nc1-1)*step))+1>nc) nc1--;

  mwdebug("Input size: nr = %d \t nc = %d\n", nr,nc);
  mwdebug("Output size: nr1 = %d \t nc1 = %d\n", nr1,nc1);

  out = mw_change_cimage(out, nr1, nc1);
  if (out == NULL) mwerror(FATAL,1,"not enough memory.\n");

  for (i=0 ; i<  nr1; i++)
    for (j=0 ; j<  nc1; j++) 
      _(out,i,j) = _(in,(int)floor((double)i*step),(int)floor((double)j*step));
  
  return(out);
}


