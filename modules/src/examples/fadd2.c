/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {fadd2};
 author = {"Jacques Froment"};
 version = {"1.0"};
 function = {"Adds the pixel's gray-levels of two fimages (for demo #2)"};
 usage = {
   fimage1->A   "Input fimage #1", 
   fimage2->B   "Input fimage #2", 
   result<-C    "Output image"
};
*/

#include <stdio.h>
#include  "mw.h"

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])

void fadd2(A,B,C)

Fimage	A,B,C;

{ int x,y;

  if((A->nrow != B->nrow) || (A->ncol != B->ncol))
    mwerror(FATAL, 1, "The input images have not the same size!\n");

  if ((C = mw_change_fimage(C,A->nrow,A->ncol)) == NULL)
    mwerror(FATAL, 1, "Not enough memory !\n");  

  for (x=0;x<A->ncol;x++) for (y=0;y<A->nrow;y++)
    _(C,x,y) = _(A,x,y) + _(B,x,y);
}
