/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {fadd4};
 author = {"Jacques Froment"};
 version = {"1.0"};
 function = {"Adds the pixel's gray-levels of two fimages (for demo #4)"};
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

void fadd4(A,B,C)

Fimage	A,B,C;

{ int x,y;
  float **TA,**TB,**TC;

  if((A->nrow != B->nrow) || (A->ncol != B->ncol))
    mwerror(FATAL, 1, "The input images have not the same size!\n");

  if ((C = mw_change_fimage(C,A->nrow,A->ncol)) == NULL)
    mwerror(FATAL, 1, "Not enough memory !\n");  

  if ( ((TA = mw_newtab_gray_fimage(A)) == NULL) ||
       ((TB = mw_newtab_gray_fimage(B)) == NULL) ||
       ((TC = mw_newtab_gray_fimage(C)) == NULL) )
    mwerror(FATAL, 1, "Not enough memory !\n");  

  for (x=0;x<A->ncol;x++) for (y=0;y<A->nrow;y++)
    TC[y][x] = TA[y][x] + TB[y][x];

  free(TC);
  free(TB);
  free(TA); 
}
