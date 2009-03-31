/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fpsnr255};
 author = {"Jacques Froment"};
 function = {"Returns the additive factor to get the
             '255' PSNR from the 'max-min' PSNR"};
 version = {"1.0"};
 usage = {
   'n'->Norm           "flag to normalize the image",
   Image->image        "original image",
   ADDPSNR<-fpsnr255   "additive PSNR factor"
};
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

static void NORM_IMG(Fimage image)
        /*--- Normalize `image` to 0.0 mean and 1.0 variance ---*/
{
    long l, c;                  /* Index of current point in `image` */
    long dx, dy;                /* Size of image */
    double mean, var;           /* Mean and variance of `image` */

    dx = image->ncol;
    dy = image->nrow;

    mean = 0.0;
    for (l = 0; l < dy; l++)
        for (c = 0; c < dx; c++)
            mean += image->gray[dx * l + c];
    mean /= (double) dx *dy;
    for (l = 0; l < dy; l++)
        for (c = 0; c < dx; c++)
            image->gray[dx * l + c] -= mean;

    var = 0.0;
    for (l = 0; l < dy; l++)
        for (c = 0; c < dx; c++)
            var += image->gray[dx * l + c] * image->gray[dx * l + c];
    var = sqrt(((double) dx * dy - 1.0) / var);
    for (l = 0; l < dy; l++)
        for (c = 0; c < dx; c++)
            image->gray[dx * l + c] *= var;

}

double fpsnr255(int *Norm, Fimage image)
                                /* Normalisation to 0 mean and 1.0 variance */
{
    long l, c;                  /* Index of current point in image */
    long ldx;
    long dx, dy;                /* Size of image */
    double min, max;            /* Minimum and maximum of `image` values */

    dx = image->ncol;
    dy = image->nrow;

    if (Norm)
        NORM_IMG(image);

    min = 1e30;
    max = -min;

    ldx = 0;
    for (l = 0; l < dy; l++)
    {
        for (c = 0; c < dx; c++)
        {
            if (image->gray[ldx + c] < min)
                min = image->gray[ldx + c];
            if (image->gray[ldx + c] > max)
                max = image->gray[ldx + c];
        }
        ldx += dx;
    }

    return (10.0 * log10((255.0 * 255.0) / ((max - min) * (max - min))));
}
