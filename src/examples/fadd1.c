/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {fadd1};
 author = {"Jacques Froment"};
 version = {"1.0"};
 function = {"Adds the pixel's gray-levels of two fimages (for demo #1)"};
 usage = {
   fimage1->A 
      "Input fimage #1", 
   fimage2->B
      "Input fimage #2", 
   result<-C
      "Output image"
};
*/
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include  "mw.h"

void fadd1(A,B,C)

Fimage	A,B,C;

{ int x,y;
  float a,b;

  if((A->nrow != B->nrow) || (A->ncol != B->ncol))
    mwerror(FATAL, 1, "The input images have not the same size!\n");

  if ((C = mw_change_fimage(C,A->nrow,A->ncol)) == NULL)
    mwerror(FATAL, 1, "Not enough memory !\n");  

  for (x=0;x<A->ncol;x++) for (y=0;y<A->nrow;y++)
    {
      a = mw_getdot_fimage(A,x,y);
      b = mw_getdot_fimage(B,x,y);
      mw_plot_fimage(C,x,y,a+b);
    }
}
