/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fmask};
 version = {"2.1"};
 author = {"Lionel Moisan"};
 function = {"Choose between two Fimages according to a mask value"};
 usage = {
   'v':[v=0]->v  "mask transparency value",
   'i'->i_flag   "invert: the test mask==v selects A (instead of B)",
   'c':c->c      "take float constant c as value of B",
   out<-out      "output Fimage (at each point, equals A if mask!=v, B else)",
   mask->mask    "mask Cimage",
   A->A          "first Fimage",
 { B->B          "second Fimage, not needed if -c option is selected" }
};
*/
/*----------------------------------------------------------------------
 v2.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

void fmask(Fimage mask, Fimage A, Fimage B, Fimage out, char *i_flag, int *v, float *c)
{
  int i;
  float a,b;
  unsigned char m;

  if ((c?1:0)+(B?1:0) != 1) 
    mwerror(USAGE,1,"Please specify image B or use -c option.");
     
  if (A->nrow!=mask->nrow || A->ncol!=mask->ncol) 
     mwerror(USAGE,1,"Image A and mask must have the same size.");

  if (B) if (B->nrow!=mask->nrow || B->ncol!=mask->ncol) 
     mwerror(USAGE,1,"Image B and mask must have the same size.");

  /* prepare output */
  out = mw_change_fimage(out,A->nrow,A->ncol);
  if (!out) mwerror(FATAL, 1, "Not enough memory.");

  /* main loop */

  for (i=A->nrow*A->ncol;i--;) {

    m = mask->gray[i];
    a = A->gray[i]; 
    b = (B?B->gray[i]:*c);
    if (i_flag) 
         out->gray[i] = ((m==*v)?a:b);
    else out->gray[i] = ((m!=*v)?a:b);

  }
}




