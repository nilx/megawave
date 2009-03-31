/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cfdiff};
 author = {"Jacques Froment"};
 function = {"Computes the difference between pixel's colors of two cfimages"};
 version = {"1.0"};
 usage = {
   'a'->absd  "flag to compute the absolute difference",
   A->A       "input cfimage A",
   B->B       "input cfimage B",
   O<-O       "output O=A-B"
};
*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

void cfdiff(char *absd, Cfimage A, Cfimage B, Cfimage O)
{
    long l;
    long dx, dy;                /* Size of image */
    float *red1, *red2, *red3;
    float *green1, *green2, *green3;
    float *blue1, *blue2, *blue3;
    int i;

    if ((A->nrow != B->nrow) || (A->ncol != B->ncol))
        mwerror(FATAL, 1, "Image1 and Image2 have not the same size!\n");

    if (A->model != B->model)
        mwerror(FATAL, 1,
                "Image1 and Image2 have not the same color model !\n");

    dx = A->ncol;
    dy = A->nrow;
    if ((O = mw_change_cfimage(O, dy, dx)) == NULL)
        mwerror(FATAL, 1, "Not enough memory\n");
    O->model = A->model;

    l = dx * dy;
    if (absd)
        for (red1 = A->red, red2 = B->red, red3 = O->red,
             green1 = A->green, green2 = B->green, green3 = O->green,
             blue1 = A->blue, blue2 = B->blue, blue3 = O->blue,
             i = 0;
             i < l;
             red1++, red2++, red3++, green1++, green2++, green3++,
             blue1++, blue2++, blue3++, i++)
        {
            *red3 = fabs((double) *red1 - *red2);
            *green3 = fabs((double) *green1 - *green2);
            *blue3 = fabs((double) *blue1 - *blue2);
        }
    else
        for (red1 = A->red, red2 = B->red, red3 = O->red,
             green1 = A->green, green2 = B->green, green3 = O->green,
             blue1 = A->blue, blue2 = B->blue, blue3 = O->blue,
             i = 0;
             i < l;
             red1++, red2++, red3++, green1++, green2++, green3++,
             blue1++, blue2++, blue3++, i++)
        {
            *red3 = *red1 - *red2;
            *green3 = *green1 - *green2;
            *blue3 = *blue1 - *blue2;
        }
}
