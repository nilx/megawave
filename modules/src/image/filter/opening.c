/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {opening};
  version = {"2.2"};
  author = {"Lionel Moisan"};
  function = {"opening/closing of a cimage"};
  usage = {
     'i'->i          "if set, a closing is applied instead of an opening",
     's':s->s        "if set, the shape s is taken as structuring element",
     'r':[r=1.0]->r  "otherwise, a disc of radius r is used",
     'n':[n=1]->n    "number of iterations",
     in->u           "input image",
     out<-v          "output image"
          };
*/
/*----------------------------------------------------------------------
 v2.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for erosion() */

Cimage opening(Cimage u, Cimage v, float *r, Curve s, int *n, char *i)
{
  Cimage w;
  
  v = mw_change_cimage(v,u->nrow,u->ncol);
  w = mw_change_cimage(NULL,u->nrow,u->ncol);
  if (!v || !w) mwerror(FATAL,1,"Not enough memory.");
  
  erosion(u,w,r,s,n,i);
  erosion(w,v,r,s,n,(char *)(!i));
  
  mw_delete_cimage(w);
  
  return v;
}
