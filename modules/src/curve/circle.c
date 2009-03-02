/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {circle};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"build a circle (Dlist)"};
  usage = {
     'r':[r=1.0]->r   "circle radius",
     'n':[n=100]->n   "number of points",
     out<-out         "output circle (Dlist)"
          };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <math.h>
#include "mw.h"
#include "mw-modules.h"

Dlist circle(Dlist out, double *r, int *n)
{
  int k;
  double *p;

  out = mw_change_dlist(out,*n+1,*n+1,2);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");
  p = out->values;
  for (k=0;k<*n;k++) {
    *(p++) = *r * cos(2.*M_PI*(double)k/(double)(*n));
    *(p++) = *r * sin(2.*M_PI*(double)k/(double)(*n));
  }
  /* close curve */
  *(p++) = out->values[0];
  *(p++) = out->values[1];

  return(out);
}

