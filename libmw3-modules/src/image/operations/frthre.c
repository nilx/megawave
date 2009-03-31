/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {frthre};
 author = {"Jacques Froment"};
 version = {"1.0"};
 function = {"Thresold the pixel's gray-levels
             of a noisy fimage for optimal recovery"};
 usage = {
    'l':[noise=20.]->noise   "Absolute value of the additive noise",
    fimage->A                "Input fimage",
    result<-B                "Output image"
};
*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

void frthre(Fimage A, Fimage B, float *noise)
{
    register float *ptrA, *ptrB;
    register int i;

    if ((B = mw_change_fimage(B, A->nrow, A->ncol)) == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    for (ptrA = A->gray, ptrB = B->gray, i = 0; i < A->nrow * A->ncol;
         ptrA++, ptrB++, i++)
    {
        if (*ptrA > *noise)
            *ptrB = *ptrA - *noise;
        else if (*ptrA < -*noise)
            *ptrB = *ptrA + *noise;
        else
            *ptrB = 0.0;
    }
}
