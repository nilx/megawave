/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {iter_fksmooth};
   author = {"Lionel Moisan"};
   version = {"1.1"};
   function = {"Iterated Euclidean heat equation (fksmooth)"};
   usage = {    
        
 'N':[niter=10]->niter "number of iterations",
 'n':[n=10]->n         "(for fksmooth) number of iterations",
 's':[std=2.]->std     "(for fksmooth) standart deviation for Gaussian kernel",
 't':[t=1.]->t         "(for fksmooth)space quantization step",
 'P'->P                "(for fksmooth) to prevent Euclidean normalization",
 in->in                "input curve (Dlist)",
 out<-out              "output curves (Dlists)"

	    };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

extern Flist fksmooth();

Flists iter_fksmooth(in,out,niter,n,std,t,P)
     Flist   in;
     Flists  out;
     int *niter,*n;
     float *std,*t;
     char *P;
{
  int i;

  out = mw_change_flists(out,*niter+1,*niter+1);
  out->list[0] = mw_copy_flist(in,NULL);
  for (i=0;i<*niter;i++) {
    fksmooth(in,n,std,t,P);
    out->list[i+1] = mw_copy_flist(in,NULL);
  }
  return(out);
}

    
