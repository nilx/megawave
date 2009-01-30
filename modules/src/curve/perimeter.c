/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {perimeter};
   author = {"Lionel Moisan"};
   version = {"1.0"};
   function = {"Compute the perimeter of a curve (Dlist)"};
   usage = {   
  'm':min<-min      "to get the minimum distance between 2 successive points",
  'M':max<-max      "to get the maximum distance between 2 successive points",
  in->in            "input Dlist",
  out<-perimeter    "result (double)"
   };
*/

#include <math.h>
#include "mw.h"


double perimeter(Dlist in, double *min, double *max)
{
  int i,d;
  double per, s, *p, d1, d2;

  if (min) *min=0.;
  if (max) *max=0.;
  if (!in || in->size<2) return(0.);
  p = in->values;
  per = 0.;
  d = in->dim;
  if (d<2) mwerror(FATAL,1,"Not a curve: dim < 2\n");
  for (i=0;i<in->size-1;i++,p+=d) {
    d1 = *p - *(p + d);
    d2 = *(p + 1) - *(p + d + 1);
    s = sqrt(d1 * d1 + d2 * d2);
    per += s;
    if (min && (i==0 || s<*min)) *min=s;
    if (max && s>*max) *max=s;
  }
  return(per);
}

