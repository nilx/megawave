/* ---------------- M E G A W A V E 2  h e a d e r ------------------------- */
/* mwcommand
 name = {fabso};
 author = {"Jacques Froment"};
 function = {"Computes the absolute gray level values of a fimage"};
 version = {"1.00"};
 usage = {
 A->A       "input fimage", 
 O<-O       "output O=fabs(A)"
};
*/
/* ------------------------------------------------------------------------- */

#include <stdio.h>
#include <math.h>
#include  "mw.h"

void fabso(A,O)

Fimage	A,O;

{
  long l;
  long	dx, dy;		/* Size of image */
  register float *ptr1,*ptr2;
  register int i;

  dx = A->ncol;
  dy = A->nrow;
  if ((O = mw_change_fimage(O,dy,dx)) == NULL)
    mwerror(FATAL, 1, "Not enough memory\n");  

  l = dx*dy;
  for (ptr1=A->gray, ptr2=O->gray, i=0;
       i < dx*dy; ptr1++, ptr2++, i++)
    *ptr2 = fabs((double) *ptr1);
}
