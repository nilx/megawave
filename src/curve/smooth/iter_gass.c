/*--------------------------- MegaWave2 Command -----------------------------*/
/* mwcommand
   name = {iter_gass};
   author = {"Lionel Moisan"};
   version = {"1.0"};
   function = {"Iterated gass"};
   usage = {    
        
'S':[scale=1.]->scale  "scale increment (default 1)",
'N':[niter=10]->niter  "number of iterations (default 10)",
'e':[e=3.]->e[2.,13.]  "(for gass) sampling precision (default 3.)",
's':s->s               "(for gass) maximal scale step",
'n':[n=5]->n           "(for gass) or minimal # of iterations (default 5)",
in->in                 "input curve (Dlist)",
out<-out               "output curves (Dlists)"

	    };
*/

#include <stdio.h>
#include "mw.h"

extern Dlists gass();

Dlists iter_gass(in,out,scale,niter,e,s,n)
     Dlist   in;
     Dlists  out;
     double  *scale,*e,*s;
     int     *niter,*n;
{
  Dlists aux,res;
  double scale1,scale2;
  int i;

  out = mw_change_dlists(out,*niter+1,*niter+1);
  out->list[0] = mw_copy_dlist(in,NULL);
  aux = mw_change_dlists(NULL,1,1);
  res = mw_new_dlists();
  scale2 = 0.;
  for (i=0;i<*niter;i++) {
    scale1 = scale2;
    scale2 += *scale;
    aux->list[0] = out->list[i];
    res = gass(aux,res,&scale1,&scale2,e,s,n,NULL,NULL);
    out->list[i+1] = mw_copy_dlist(res->list[0],NULL);
  }
  free(aux);
  mw_delete_dlists(res);
}

    
