/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {circle};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"build a circle (Dlist)"};
  usage = {
     'r':[r=1.0]->r   "circle radius (default: 1.0)",
     'n':[n=100]->n   "number of points (default: 100)",
     out<-out         "output circle (Dlist)"
          };
*/

#include <math.h>
#include "mw.h"

Dlist circle(out,r,n)
     Dlist   out;
     double  *r;
     int     *n;
{
  int k;
  double *p;

  out = mw_change_dlist(out,*n+1,*n+1,2);
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

