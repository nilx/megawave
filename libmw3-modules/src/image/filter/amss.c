/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {amss};
version = {"1.6"};
author = {"Jacques Froment, Frederic Guichard, Lionel Moisan"};
function = { "Affine Morphological Scale Space
           (or Mean Curvature Motion) - by anisotropic diffusion"};
usage = {

 'i'->isotrop
      "flag to cancel isotropic diffusion in smooth area",
 'p'->power
      "flag to compute AMSS model (power 1/3) instead of MCM (power 1)",
 'S':[Step=0.1]->Step [0.0,0.5]
      "scale step for each iteration",
 'm':[MinGrad=0.5]->MinGrad [0.0,1e6]
      "Minimum of the gradient norm to compute the curvature",
 's':[outputStep=0.1]->outputStep [0.0, 10.0]
      "scale interval between two images (for movie outputs)",
 'f':[firstScale=0.0]->firstScale
      "first scale of diffusion",
 'l':[lastScale=2.0]->lastScale
      "last scale of diffusion",
 'n'->no_norm
      "flag to prevent curvature normalization",

 'd':cimageD<-imageD  "output fimage of the last diffusion",
 'g':cimageG<-imageG  "output fimage of the last gradient",
 'c':cimageC<-imageC  "output fimage of the last curvature",
 'D':cmovieD<-cmovieD "output cmovie of successive diffusions",
 'G':cmovieG<-cmovieG "output cmovie of successive gradients",
 'C':cmovieC<-cmovieC "output cmovie of successive curvatures",

 image->image         "original picture (input fimage)"

};
*/
/*----------------------------------------------------------------------
 v1.5: changes by L.Moisan
      - own gradient/curvature computation as in fderiv
      - allows non-coupled outputs (e.g. gradient OR curvature)
      - no_norm flag to cancel curvature normalization
 v1.6 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

/* NB: input image is destroyed */

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

#define IRAC8    0.35355339     /* 1/sqrt(1/8)   */
#define IRAC2P2  0.29289322     /* 1/(sqrt(2)+2) */
#define IRAC2    0.70710678     /* 1/sqrt(2) */
#define RAC8P4   6.8284271      /* sqrt(8)+4 */

/*------------------------- one step of evolution ----------------------*/
/*                                                                      */
/*     gradient, curvature and laplacian are computed as in fderiv()    */
/*         but it is faster to compute the evolution locally            */
/*   especially if the gradient norm is not needed (no sqrt to compute) */
/*----------------------------------------------------------------------*/
static void one_step(Fimage in, Fimage out, Fimage grad, Fimage curv,
                     float step, float MinGrad, char *isotrop, char *power,
                     char *no_norm)
{
    int y, nx, ny;
    register int x, xm, x1, Ym, Y0, Y1;
    float c1, d1, GK, MinGrad2;
    register float l0, ax, ay, axy, an2;
    register float a11, amm, am1, a1m, a00, a01, a10, a0m, am0;

    MinGrad2 = MinGrad / IRAC2P2;
    MinGrad2 *= MinGrad2;

    nx = in->ncol;
    ny = in->nrow;

    for (y = 0; y < ny; y++)
    {

        Y0 = y * nx;

        /* symmetry for borders */
        Y1 = (y < ny - 1 ? y + 1 : y - 1) * nx;
        Ym = (y > 0 ? y - 1 : y + 1) * nx;

        for (x = 0; x < nx; x++)
        {

            /* symmetry for borders */
            x1 = (x < nx - 1 ? x + 1 : x - 1);
            xm = (x > 0 ? x - 1 : x + 1);

            a11 = in->gray[x1 + Y1];
            a10 = in->gray[x1 + Y0];
            a1m = in->gray[x1 + Ym];
            a01 = in->gray[x + Y1];
            a00 = in->gray[x + Y0];
            a0m = in->gray[x + Ym];
            am1 = in->gray[xm + Y1];
            am0 = in->gray[xm + Y0];
            amm = in->gray[xm + Ym];

            /* compute gradient (8-neighbors) */
            c1 = a11 - amm;
            d1 = am1 - a1m;
            ax = a10 - am0 + IRAC8 * (c1 - d1);
            /* we don't normalize to save time */
            ay = a01 - a0m + IRAC8 * (c1 + d1);

            axy = ax * ay;
            ax *= ax;
            ay *= ay;
            an2 = ax + ay;      /* now, we have an2*(IRAC2P2)^2 = |Du|^2 */

            if (grad)
                grad->gray[x + Y0] = IRAC2P2 * (float) sqrt((double) an2);

            if (an2 > MinGrad2)
            {

                   /*----- NON-ZERO GRADIENT -----*/

                /* compute |Du|*curvature, ie GK = D2u ( Du+ , Du+ ) / |Du|^2 */
                ax /= an2;
                ay /= an2;
                axy /= an2;
                l0 = 0.5 - axy * axy;
                GK = a00 * (-4.0 * l0)
                    + (am0 + a10) * (2.0 * l0 - ax)
                    + (a0m + a01) * (2.0 * l0 - ay)
                    + (a11 + amm) * (-l0 + 0.5 * (ax + ay - axy))
                    + (am1 + a1m) * (-l0 + 0.5 * (ax + ay + axy));

                if (curv)
                {
                    if (grad)
                        curv->gray[x + Y0] = GK / grad->gray[x + Y0];
                    else
                        curv->gray[x + Y0] =
                            GK / (IRAC2P2 * (float) sqrt((double) an2));
                }
                if (power)
                {
                    /* AMSS diffusion */
                    GK *= an2 * IRAC2P2 * IRAC2P2;
                    if (GK >= 0.0)
                        /* here we use the fact that
                         * |Du|K^(1/3) = (|Du|^2*GK)^(1/3) */
                        out->gray[x + Y0] =
                            a00 + step * (float) pow((double) (GK),
                                                     .33333333333333333333);
                    else
                        out->gray[x + Y0] =
                            a00 - step * (float) pow((double) (-GK),
                                                     .33333333333333333333);
                }
                else
                    /* MCM diffusion */
                    out->gray[x + Y0] = a00 + step * GK;

            }
            else
            {

                  /*----- ZERO GRADIENT -----*/

                if (curv)
                    curv->gray[x + Y0] = 0.0;

                if (!isotrop)
                {

                    /* isotropic diffusion */
                    out->gray[x + Y0] = a00 + step *
                        /* Laplacian */
                        (IRAC2 * (amm + am1 + a1m + a11) +
                         (a0m + am0 + a10 + a01) - RAC8P4 * a00) / RAC8P4;

                }
                else
                    /* no diffusion */
                    out->gray[x + Y0] = a00;

            }
            /* RENORMALIZATION of curvature */
            if (!no_norm && curv)
                curv->gray[x + Y0] = curv->gray[x + Y0] * 100.0 + 128.0;
        }
    }
}

/* ------------------------------------------------------------------------*/
/* -------- This function returns the number of iterations         --------*/
/* -------- in order to go at scale = lastScale, with step = Step  --------*/
/* -------- from scale = firstScale.                               --------*/
/* ------------------------------------------------------------------------*/

static float number_iterations(char *power, float lastScale, float firstScale,
                               float Step)
{
    double cfs;

    if (!power)
    {
        /* ------- case : Power = 1 -------- */
        return (0.5 * (lastScale * lastScale - firstScale * firstScale) /
                Step);
    }
    else
    {
        /* ------- case : Power = 1/3 ------ */

        if (firstScale > 0.0)
            cfs = exp(4.0 * log((double) firstScale) / 3.0);
        else
            cfs = 0.0;
        return (0.75 * (exp(4.0 * log((double) lastScale) / 3.0) - cfs) /
                Step);
    }
}

/*----------------- Insert of an image into the output movie ----------------*/

static void FINAL(Fimage pict, Cmovie sortie)
{
    register int i;
    Cimage image1, image2;
    int dx, dy, taille;
    register unsigned char *a;
    register float *b;

    image1 = NULL;
    dx = pict->ncol;
    dy = pict->nrow;
    image1 = mw_change_cimage(image1, dy, dx);
    if (sortie->first == NULL)
    {
        sortie->first = image1;
    }
    else
    {
        image2 = sortie->first;
        while (image2->next != NULL)
        {
            image2 = image2->next;
        }
        image2->next = image1;
        image1->previous = image2;
    }
    a = image1->gray;
    b = pict->gray;
    taille = dx * dy;
    for (i = 0; i < taille; i++)
    {
        *a = (*b < 0.0 ? 0 : (*b > 255.0 ? 255 : (unsigned char) (*b)));
        a++;
        b++;
    }
}

/*--------------------- Main Program -------------------------------------*/

void amss(char *isotrop, char *power, float *Step, float *MinGrad,
          float *outputStep, float *firstScale, float *lastScale,
          Fimage image, Fimage * imageD, Fimage * imageG, Fimage * imageC,
          Cmovie cmovieD, Cmovie cmovieG, Cmovie cmovieC, char *no_norm)
                        /* isotropic diffusion if Grad==0 & isotrop != NULL */
                        /* power 1, if == NULL ; power 1/3, if != NULL      */
                        /* Step of the scale                                */
                        /* Minimum value of the gradient                    */
                        /* frequency of saving                              */
                        /* first scale, equal to zero if not used           */
                        /* scale of the last iteration                      */
                        /* Initial Picture                                  */
                        /* Final Picture                                    */
                        /* Final gradient picture                           */
                        /* Final curvature picture                          */
                        /* Output of images at differents scale             */
                        /* Output of gradients                              */
                        /* Output of curvatures                             */
                        /* if set, cancel curvature normalization           */
{
    int n_iter;                 /* number of iterations */
    float kkk, outputStepk;
    int k, i, need_movies;
    Fimage tmp, grad, curv, flip_img[2];

    /* check that options are correct */

    if (*firstScale >= *lastScale)
        mwerror(USAGE, 1, "lastScale must be bigger than firstScale");

    if ((!power && (*Step > 0.1000001)) || (power && (*Step > 0.5000001)))
        mwerror(USAGE, 1, "Step %f too big for the power.", *Step);

    need_movies = (cmovieD || cmovieG || cmovieC);

    /* allocate some images if necessary */

    tmp = mw_change_fimage(*imageD, image->nrow, image->ncol);
    if (tmp == NULL)
        mwerror(FATAL, 1, "Not enough memory\n");

    grad = *imageG;
    if (*imageG || cmovieG)
    {
        grad = mw_change_fimage(grad, image->nrow, image->ncol);
        if (grad == NULL)
            mwerror(FATAL, 1, "Not enough memory\n");
    }

    curv = *imageC;
    if (*imageC || cmovieC)
    {
        curv = mw_change_fimage(curv, image->nrow, image->ncol);
        if (curv == NULL)
            mwerror(FATAL, 1, "Not enough memory\n");
    }

    /* initializations */

    n_iter =
        (int) (number_iterations(power, *lastScale, *firstScale, *Step) +
               0.5);
    mwdebug("Real time = %f\n", *lastScale);
    mwdebug("Number of iterations = %i\n", n_iter);

    i = 0;
    flip_img[0] = image;
    flip_img[1] = tmp;
    kkk = 0.5 / *Step;
    outputStepk = *outputStep;

                      /*----- MAIN LOOP -----*/

    for (k = 1; k <= n_iter; k++)
    {

        /* these lines are temporarily removed (problems with fflush) */
        /*mwdebug("iteration= %d / %d"
         *        "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
         *        k, n_iter);
         * fflush(stdout); */

        /* one-step evolution */
        one_step(flip_img[i], flip_img[1 - i], grad, curv,
                 *Step, *MinGrad, isotrop, power, no_norm);
        i = 1 - i;

        if (need_movies && (k >= kkk || k == n_iter))
        {
            outputStepk = outputStepk + *outputStep;
            if (k < n_iter)
            {
                if (cmovieD)
                    FINAL(flip_img[i], cmovieD);
                if (cmovieG)
                    FINAL(grad, cmovieG);
                if (cmovieC)
                    FINAL(curv, cmovieC);
            }
            kkk = number_iterations(power, outputStepk, *firstScale, *Step);
        }

    }
    if (i == 1)
        mw_delete_fimage(image);
    else if (*imageD)
        *imageD = image;
    if (!*imageD)
        mw_delete_fimage(tmp);

    if (cmovieG && !*imageG)
        mw_delete_fimage(grad);
    if (cmovieC && !*imageC)
        mw_delete_fimage(curv);
}
