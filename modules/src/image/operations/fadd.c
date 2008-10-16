/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fadd};
 author = {"Jacques Froment"};
 version = {"1.2"};
 function = {"Adds the pixel's gray-levels of two fimages"};
 usage = {
   'm':min->min  "Force output minimal value",
   'M':max->max  "Force output maximal value",
   'a'->a        "average: output is C=(A+B)/2",
   A->A          "Input fimage #1", 
   B->B          "Input fimage #2", 
   C<-C          "Output fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1: added -a option (L.Moisan)
 v1.2: removed fthre call and useless -n option, return result (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include  "mw.h"

Fimage fadd(A,B,C,min,max,a)
     Fimage  A,B,C;
     float   *min,*max;
     char    *a;
{
  float v;
  int i;

  if((A->nrow != B->nrow) || (A->ncol != B->ncol))
    mwerror(FATAL, 1, "Input images must have the same size\n");

  if ((C = mw_change_fimage(C,A->nrow,A->ncol)) == NULL)
    mwerror(FATAL, 1, "Not enough memory\n");  

  for (i=A->nrow*A->ncol;i--;) {
    v = A->gray[i]+B->gray[i];
    if (a) v *= 0.5;
    if (min && v<*min) v = *min;
    if (max && v>*max) v = *max;
    C->gray[i] = v;
  }
  return C;
}
