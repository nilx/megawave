/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {fadd3};
 author = {"Jacques Froment"};
 version = {"1.0"};
 function = {"Adds the pixel's gray-levels of two fimages (demo #3)"};
 usage = {
   fimage1->A    "Input fimage #1", 
   fimage2->B    "Input fimage #2", 
   result<-C     "Output image"
};
*/

#include <stdio.h>
#include  "mw.h"

void fadd3(A,B,C)

Fimage	A,B,C;

{
  register float *ptr1,*ptr2,*ptr3;
  register int i;

  if((A->nrow != B->nrow) || (A->ncol != B->ncol))
    mwerror(FATAL, 1, "The input images have not the same size!\n");

  if ((C = mw_change_fimage(C,A->nrow,A->ncol)) == NULL)
    mwerror(FATAL, 1, "Not enough memory !\n");  

  for (ptr1=A->gray, ptr2=B->gray, ptr3=C->gray, i=0;
       i < A->nrow*A->ncol; ptr1++, ptr2++, ptr3++, i++)
    *ptr3 = *ptr1 + *ptr2;
}
