/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {fvar};
author = {"Jacques Froment"};
function = {"Compute the variance of the gray level of an image"};
usage = {
A->A "input fimage",
v<-fvar "output variance"
};
version = {"1.1"};
*/

#include <stdio.h>
#include  "mw.h"

float fvar(A)

Fimage A;

{
  int s;
  register float *ptr;
  register int i;
  double m,v,vr;

  s = A->ncol*A->nrow;
  if (s <= 1) return(0.);

  for (m=0., i=0, ptr = A->gray; i<s; i++,ptr++) m += *ptr;
  m /= (double)s;
      
  vr = 0.0;
  for (i=0, ptr = A->gray;i<s;i++,ptr++) 
    {
      v = *ptr - m;  
      vr += v*v;
    }
  vr /=  (double)s - 1.;
  return((float) vr);
}
