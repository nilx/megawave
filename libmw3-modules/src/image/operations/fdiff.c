/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fdiff};
 author = {"Jacques Froment"};
 function = {"Computes the difference between pixel's
             gray-levels of two fimages"};
 version = {"1.0"};
 usage = {
   'a'->absd  "flag to compute the absolute difference",
   A->A       "input fimage A",
   B->B       "input fimage B",
   O<-O       "output O=A-B"
};
*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

void fdiff(char *absd, Fimage A, Fimage B, Fimage O)
{
    long l;
    long dx, dy;                /* Size of image */
    register float *ptr1, *ptr2, *ptr3;
    register int i;

    if ((A->nrow != B->nrow) || (A->ncol != B->ncol))
        mwerror(FATAL, 1, "Image1 and Image2 have not the same size!\n");

    dx = A->ncol;
    dy = A->nrow;
    if ((O = mw_change_fimage(O, dy, dx)) == NULL)
        mwerror(FATAL, 1, "Not enough memory\n");

    l = dx * dy;
    if (absd)
        for (ptr1 = A->gray, ptr2 = B->gray, ptr3 = O->gray, i = 0;
             i < dx * dy; ptr1++, ptr2++, ptr3++, i++)
            *ptr3 = fabs((double) *ptr1 - *ptr2);
    else
        for (ptr1 = A->gray, ptr2 = B->gray, ptr3 = O->gray, i = 0;
             i < dx * dy; ptr1++, ptr2++, ptr3++, i++)
            *ptr3 = *ptr1 - *ptr2;

    /*
     * strcpy(O->cmt, "Absolute diff. of ");
     * strcat(O->cmt, A->name);
     * strcat(O->cmt, " and ");
     * strcat(O->cmt, B->name);
     */
}
