/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
  name = {fderiv};
  version = {"1.4"};
  author = {"Lionel Moisan"};
  function = {"1st and 2nd order derivatives of an image (3x3 stencil)"};
  usage = {
'k':curv<-curv
      "curvature:      D2u ( Du+ , Du+ ) / |Du|^3",
'a':anti<-anti
      "anticurvature:  D2u ( Du  , Du+ ) / |Du|^3",
'c':canny<-canny_img
      "Canny operator: D2u ( Du  , Du  ) / |Du|^3",
'l':laplacian<-laplacian
      "Laplacian:      Trace ( D2u )",
'x':gradx<-gradx
      "gradient (x coordinate)",
'y':grady<-grady
      "gradient (y coordinate)",
'n':gradn<-gradn
      "gradient norm |Du|",
'p':gradp<-gradp
      "gradient phase (degrees) in [-180,180] U {mw_not_an_argument (=1.0e9)}",
'm':[MinGrad=0.0]->MinGrad
      "minimum of |Du| to compute 2nd order derivatives",
's':[nsize=8]->nsize
      "neighborhood size used for the gradient: 8, 4, 3 or 2",
in->in
      "input Fimage"
  };
*/
/*----------------------------------------------------------------------
 v1.3: removed unary + (L.Moisan)
 v1.4 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

#define IRAC8    0.35355339     /* 1/sqrt(8)   */
#define IRAC2P2  0.29289322     /* 1/(sqrt(2)+2) */
#define IRAC2    0.70710678     /* 1/sqrt(2) */
#define RAC8P4   6.8284271      /* sqrt(8)+4 */
#define RADIANS_TO_DEGREES (180.0/M_PI)

void fderiv(Fimage in, Fimage curv, Fimage anti, Fimage canny_img,
            Fimage laplacian, Fimage gradx, Fimage grady, Fimage gradn,
            Fimage gradp, float *MinGrad, int *nsize)
{
    int y, nx, ny;
    register int x, xm, x1, Ym, Y0, Y1;
    float c1, d1, l0, ax = 0.0, ay = 0.0, axy, an;
    /* fixme: x86 architecture can't keep all these values as registers... */
    register float a11, amm, am1, a1m, a00, a01, a10, a0m, am0;

    nx = in->ncol;
    ny = in->nrow;

    if (curv)
    {
        if (!mw_change_fimage(curv, ny, nx))
            mwerror(FATAL, 1, "Not enough Memory.\n");
        else
            mw_clear_fimage(curv, 0.0);
    }
    if (anti)
    {
        if (!mw_change_fimage(anti, ny, nx))
            mwerror(FATAL, 1, "Not enough Memory.\n");
        else
            mw_clear_fimage(anti, 0.0);
    }
    if (canny_img)
    {
        if (!mw_change_fimage(canny_img, ny, nx))
            mwerror(FATAL, 1, "Not enough Memory.\n");
        else
            mw_clear_fimage(canny_img, 0.0);
    }
    if (laplacian)
    {
        if (!mw_change_fimage(laplacian, ny, nx))
            mwerror(FATAL, 1, "Not enough Memory.\n");
        else
            mw_clear_fimage(laplacian, 0.0);
    }
    if (gradx)
    {
        if (!mw_change_fimage(gradx, ny, nx))
            mwerror(FATAL, 1, "Not enough Memory.\n");
        else
            mw_clear_fimage(gradx, 0.0);
    }
    if (grady)
    {
        if (!mw_change_fimage(grady, ny, nx))
            mwerror(FATAL, 1, "Not enough Memory.\n");
        else
            mw_clear_fimage(grady, 0.0);
    }
    if (gradn)
    {
        if (!mw_change_fimage(gradn, ny, nx))
            mwerror(FATAL, 1, "Not enough Memory.\n");
        else
            mw_clear_fimage(gradn, 0.0);
    }
    if (gradp)
    {
        if (!mw_change_fimage(gradp, ny, nx))
            mwerror(FATAL, 1, "Not enough Memory.\n");
        else
            mw_clear_fimage(gradp, mw_not_an_argument);
    }
    /* MAIN LOOP */

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

            /* gradient */

            switch (*nsize)
            {
            case 8:
                c1 = a11 - amm;
                d1 = am1 - a1m;
                ax = IRAC2P2 * (a10 - am0 + IRAC8 * (c1 - d1));
                ay = IRAC2P2 * (a01 - a0m + IRAC8 * (c1 + d1));
                break;
            case 4:
                ax = 0.5 * (a10 - am0);
                ay = 0.5 * (a01 - a0m);
                break;
            case 3:
                c1 = a11 - a00;
                d1 = a10 - a01;
                ax = 0.5 * (c1 + d1);
                ay = 0.5 * (c1 - d1);
                break;
            case 2:
                ax = a10 - a00;
                ay = a01 - a00;
                break;
            default:
                mwerror(FATAL, 1, "Unrecognized neighborhood size.\n");
            }

            if (gradx)
                gradx->gray[x + Y0] = ax;
            if (grady)
                grady->gray[x + Y0] = ay;
            if (gradp)
                if (ax != 0.0 || ay != 0.0)
                    gradp->gray[x + Y0] = RADIANS_TO_DEGREES *
                        (float) atan2((double) ay, (double) ax);

            if (laplacian)
                laplacian->gray[x + Y0] =
/* Laplacian : Trace ( D2u ) */
                    (IRAC2 * (amm + am1 + a1m + a11) +
                     (a0m + am0 + a10 + a01) - RAC8P4 * a00) / RAC8P4;

            if (gradn || canny_img || curv || anti)
            {
                an = sqrt((double) ax * (double) ax
                          + (double) ay * (double) ay);
                if (gradn)
                    gradn->gray[x + Y0] = an;
                if ((an > *MinGrad) && (canny_img || curv || anti))
                {
                    ax /= an;
                    ay /= an;
                    axy = ax * ay;
                    ax *= ax;
                    ay *= ay;
                    l0 = 0.5 - axy * axy;

                    if (curv)
                        curv->gray[x + Y0] =
/* curvature:      D2u ( Du+ , Du+ ) / |Du|^3  */
                            (a00 * (-4.0 * l0)
                             + (am0 + a10) * (2.0 * l0 - ax)
                             + (a0m + a01) * (2.0 * l0 - ay)
                             + (a11 + amm) * (-l0 + 0.5 * (ax + ay - axy))
                             + (am1 + a1m) * (-l0 +
                                              0.5 * (ax + ay + axy))) / an;

                    if (anti)
                        anti->gray[x + Y0] =
/* anticurvature:  D2u ( Du  , Du+ ) / |Du|^3 */
                            (a00 * (-4.0 * l0)
                             + (am0 + a10) * (2.0 * l0 - (-axy))
                             + (a0m + a01) * (2.0 * l0 - axy)
                             + (a11 + amm) * (-l0 + 0.5 * ((ax - ay)))
                             + (am1 + a1m) * (-l0 + 0.5 * (-(ax - ay)))) / an;

                    if (canny_img)
                        canny_img->gray[x + Y0] =
/* Canny operator: D2u ( Du  , Du  ) / |Du|^3 */
                            (a00 * (-4.0 * l0)
                             + (am0 + a10) * (2.0 * l0 - ay)
                             + (a0m + a01) * (2.0 * l0 - ax)
                             + (a11 + amm) * (-l0 + 0.5 * (ay + ax - axy))
                             + (am1 + a1m) * (-l0 +
                                              0.5 * (ay + ax + axy))) / an;

                }
            }
        }
    }
}
