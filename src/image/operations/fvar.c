/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {fvar};
author = {"Jacques Froment"};
function = {"Compute the variance of the gray level of an image"};
usage = {
A->A "input fimage",
v<-fvar "output variance"
};
version = {"1.0"};
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

  m=0.0;
  s = A->ncol*A->nrow;
  mwdebug("Imput image of size (%d,%d)\n",A->ncol,A->nrow);

  if (s <= 1) mwerror(FATAL,1,"Illegal size for the image\n");

  for (i=0, ptr = A->gray;i<s;i++,ptr++) m += *ptr;
  m /= s;
      
  vr = 0.0;
  for (i=0, ptr = A->gray;i<s;i++,ptr++) 
    {
      v = *ptr - m;  
      vr += v*v;
    }
  vr /= (s-1);
  return((float) vr);
}
