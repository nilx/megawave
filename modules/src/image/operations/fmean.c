/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fmean};
 version = {"1.0"};
 author = {"Jacques Froment"};
 function = {"Compute the mean gray level of an image"};
 usage = {
   A->A      "input fimage",
   m<-fmean  "output mean gray level value"
};
*/

#include <stdio.h>
#include  "mw.h"

float fmean(Fimage A)
{
  int s;
  register float *ptr;
  register int i;
  float m;

  m=0.0;
  s = A->ncol*A->nrow;
  for (i=0, ptr = A->gray;i<s;i++,ptr++) m += *ptr;
  m /= s;
      
  return(m);
}
