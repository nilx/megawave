/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fvar};
 version = {"1.2"};
 author = {"Jacques Froment"};
 function = {"Compute the variance of the gray level of an image"};
 usage = {
   'e'->e  "compute the true empirical variance (not the unbiased estimate)",
   's'->s  "take square root (compute standart deviation instead of variance)",
   A->A    "input fimage",
   v<-fvar "output variance"
};
*/
/*----------------------------------------------------------------------
 v1.2: added -e and -s options (L.Moisan)
 ----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include  "mw.h"

float fvar(A,e,s)
     Fimage A;
     int *e,*s;
{
  int size;
  register float *ptr;
  register int i;
  double m,v,vr;

  size = A->ncol*A->nrow;
  if (size <= 1) return(0.);

  for (m=0., i=0, ptr = A->gray; i<size; i++,ptr++) m += *ptr;
  m /= (double)size;
      
  vr = 0.0;
  for (i=0, ptr = A->gray;i<size;i++,ptr++) 
    {
      v = *ptr - m;  
      vr += v*v;
    }
  vr /=  (double)size - (e?0.:1.);
  if (s) vr = sqrt(vr);

  return ((float)vr);
}
