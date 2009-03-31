/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fmse};
 author = {"Jean-Pierre D'Ales, Jacques Froment"};
 function = {"Computes the mean square difference between two fimages"};
 version = {"1.03"};
 usage = {
   'n'->Norm     "flag to normalize the images",
   'p'->PsnrFlg  "flag to compute PSNR with max_Image1 - min_Image1 = 255",
   Image1->Img1  "input image #1",
   Image2->Img2  "input image #2",
   SNR<-SNR      "signal to noise ratio / `Image1` (SNR)",
   PSNR<-PSNR    "peak signal to noise ratio / `Image1` (PSNR)",
   MSE<-MSE      "mean square error between Image1 and Image2 (MSE)",
   MRD<-MRD      "maximal relative difference (MRD)"
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

void fmse(Fimage Img1, Fimage Img2, int *Norm, char *PsnrFlg, double *SNR,
          double *PSNR, double *MSE, double *MRD)
        /*--- Computes the mean square error between Img1 and Img2 ---*/
                                /* Input images */
                                /* Normalisation to 0 mean and 1.0 variance */
                                /* Alternative computation for the PSNR */
                                /* Signal to noise ratio / `Img1` */
                                /* Peak signal to noise ratio / `Img1` */
                                /* Mean square error between Img1 and Img2 */
                                /* Maximal relative difference */
{
    long l, c;                  /* Index of current point in `Img1`, `Img2` */
    long ldx;
    long dx, dy;                /* Size of image */
    double diff;                /* Difference between two values */
    double min1, max1;          /* Minimum and maximum of `Img1` values */
    double min, max;            /* Minimum and maximum of `Img1` and `Img2`
                                 * values */
    double mean1;               /* Mean value of `Img1` */
    double var1;                /* Empirical variance of `Img1` */
    double DMAX;                /* Absolute value of the maximum difference
                                 * between values of `Img1` and `Img2` */

        /*--- Verification of images' sizes ---*/

    if ((Img1->nrow != Img2->nrow) || (Img1->ncol != Img2->ncol))
        mwerror(FATAL, 1, "Image1 and Image2 have not the same size!\n");

    dx = Img1->ncol;
    dy = Img1->nrow;

        /*--- Normalisation of images (if selected) ---*/

    if (Norm)
    {
        NORM_IMG(Img1);
        NORM_IMG(Img2);
    }

        /*--- Computation of minimum and maximum values in `Img1` ---*/
                  /*--- and over `Img1` and `Img2` ---*/

    min = min1 = 1e30;
    max = max1 = -min1;

    ldx = 0;
    for (l = 0; l < dy; l++)
    {
        for (c = 0; c < dx; c++)
        {
            if (Img1->gray[ldx + c] < min1)
            {
                min1 = Img1->gray[ldx + c];
                if (Img1->gray[ldx + c] < min)
                    min = min1;
            }
            if (Img1->gray[ldx + c] > max1)
            {
                max1 = Img1->gray[ldx + c];
                if (Img1->gray[ldx + c] > max)
                    max = max1;
            }
            if (Img2->gray[ldx + c] < min)
                min = Img2->gray[ldx + c];
            if (Img2->gray[ldx + c] > max)
                max = Img2->gray[ldx + c];
        }
        ldx += dx;
    }

        /*--- Computation of variance of `Img1` ---*/

    mean1 = 0.0;
    if (!Norm)
    {
        ldx = 0;
        for (l = 0; l < dy; l++)
        {
            for (c = 0; c < dx; c++)
                mean1 += Img1->gray[ldx + c];
            ldx += dx;
        }
        mean1 /= (double) dx *dy;
    }

    var1 = 0.0;
    ldx = 0;
    for (l = 0; l < dy; l++)
    {
        for (c = 0; c < dx; c++)
            var1 +=
                (Img1->gray[ldx + c] - mean1) * (Img1->gray[ldx + c] - mean1);
        ldx += dx;
    }
    var1 /= ((double) dx * dy - 1.0);

        /*--- Computation of m.s.e. and s.n.r. ---*/

    DMAX = 0.0;
    *MSE = 0.0;
    ldx = 0;
    for (l = 0; l < dy; l++)
    {
        for (c = 0; c < dx; c++)
        {
            diff = fabs((double) Img1->gray[ldx + c] - Img2->gray[ldx + c]);
            if (diff > DMAX)
                DMAX = diff;
            *MSE += diff * diff;
        }
        ldx += dx;
    }

    *MSE /= (double) dx *dy;
    if (var1 == 0.0)
    {
        mwerror(WARNING, 0,
                "First input image has a constant plane. Setting S.N.R.=0.\n");
        *SNR = 0.0;
    }
    else
        *SNR = 10.0 * log10(var1 / (*MSE));

    if (PsnrFlg)
    {
        mwdebug("PsnrFlg is active : compute 255-PSNR\n");
        *PSNR = 10.0 * log10(255.0 * 255.0 / (*MSE));
    }
    else
    {
        mwdebug("PsnrFlg is not active : does not compute 255-PSNR\n");
        if (max1 == min1)
        {
            mwerror(WARNING, 0,
                    "First input image has a constant plane. "
                    "Compute P.S.N.R. with max-min=255.\n");
            *PSNR = 10.0 * log10(255.0 * 255.0 / (*MSE));
        }
        else
            *PSNR = 10.0 * log10((max1 - min1) * (max1 - min1) / (*MSE));
        /*
         * printf("true PSNR = %lg\n",
         * 10.0 * log10((255.0*255.0)/((max1 - min1)* (max1 - min1))) + *PSNR);
         */
    }

    if (max == min)
        *MRD = 0.0;
    else
        *MRD = 100.0 * DMAX / (max - min);

        /*--- Printing of results ---*/

    mwdebug("-> Maximal relative difference = %3.1lf\n", *MRD);
    mwdebug("-> Mean square error : MSE = %lg\n", *MSE);
    mwdebug("-> Peak signal to noise ratio : PSNR = %lg db\n", *PSNR);
    mwdebug("-> Signal to noise ratio : SNR = %lg db\n", *SNR);

}
