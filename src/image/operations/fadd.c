/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {fadd};
 author = {"Jacques Froment"};
 version = {"1.1"};
 function = {"Adds the pixel's gray-levels of two fimages"};
 usage = {
 'n'->norm    "Normalize output into [min,max]",
 'm':min->m0  "Force output minimal value",
 'M':max->m1  "Force output maximal value",
 'a'->a       "average: output is C=(A+B)/2",
 A->A 
   "Input fimage #1", 
 B->B
   "Input fimage #2", 
 C<-C
      "Output fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1: added -a option (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include  "mw.h"

extern void fthre();


void fadd(A,B,C,norm,m0,m1,a)

Fimage	A,B,C;
char *norm;
float *m0,*m1;
char *a;

{
  register float *ptr1,*ptr2,*ptr3;
  register int i;

  if (norm && (!m0 || !m1)) mwerror(USAGE,0,"Normalization needs selection of [min,max] values\n");

  if((A->nrow != B->nrow) || (A->ncol != B->ncol))
    mwerror(FATAL, 1, "The input images have not the same size!\n");

  if ((C = mw_change_fimage(C,A->nrow,A->ncol)) == NULL)
    mwerror(FATAL, 1, "Not enough memory !\n");  

  for (ptr1=A->gray, ptr2=B->gray, ptr3=C->gray, i=0;
       i < A->nrow*A->ncol; ptr1++, ptr2++, ptr3++, i++) 
    *ptr3 = *ptr1 + *ptr2;

  if (a) for (i=A->nrow*A->ncol;i--;) C->gray[i]*=0.5;

  if (m0 || m1) fthre(C,C,norm,NULL,m0,m1);
}
