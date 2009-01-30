/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {iter_gass};
   author = {"Lionel Moisan"};
   version = {"1.1"};
   function = {"Iterated gass"};
   usage = {    
        
 'S':[scale=1.]->scale  "scale increment",
 'N':[niter=10]->niter  "number of iterations",
 'e':[e=3.]->e[2.,13.]  "(for gass) sampling precision",
 's':s->s               "(for gass) maximal scale step",
 'n':[n=5]->n           "(for gass) or minimal # of iterations",
 in->in                 "input curve (Dlist)",
 out<-out               "output curves (Dlists)"

	    };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for gass() */

Dlists iter_gass(Dlist in, Dlists out, double *scale, int *niter, double *e, double *s, int *n)
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
  
  return out;
}

    
