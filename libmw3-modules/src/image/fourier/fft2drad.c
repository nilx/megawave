/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fft2drad};
 version = {"1.2"};
 author = {"Lionel Moisan"};
 function = {"compute average spectrum as a radial function"};
 usage = {
   'l'->l_flag          "take log10",
   's':size->size       "output size (number of cells), default min(nx,ny)",
   'I':input_im->in_im  "imaginary input (Fimage)",
   input_re->in_re      "real input (Fimage)",
   out<-out             "output |FFT|=f(r) (Fsignal)"
};
*/
/*----------------------------------------------------------------------
 v1.2: minor changes (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for fft2dpol() */

/* NB : as for fft2d :
     * calling this module with in_im=NULL is possible and means
       in_im(x,y) = 0 everywhere
*/

Fsignal fft2drad(Fimage in_re, Fimage in_im, Fsignal out, char *l_flag,
                 int *size)
{
    Fimage rho;
    int n, x, y, xx, yy, r, nx, ny, *count;
    double cx, cy;

    rho = mw_new_fimage();
    fft2dpol(in_re, in_im, rho, NULL, NULL);
    nx = rho->ncol;
    ny = rho->nrow;

    if (size)
        n = *size;
    else
        n = ceil((double) (nx < ny ? nx : ny) * sqrt(0.5));
    out = mw_change_fsignal(out, n + 1);
    if (!out)
        mwerror(FATAL, 1, "Not enough memory\n");

    /* scale is in lines per pixel */
    out->scale = (float) sqrt(0.5) / (float) n;
    mw_clear_fsignal(out, 0.);
    count = (int *) calloc(n + 1, sizeof(int));

    cx = 1. / (double) (nx * nx);
    cy = 1. / (double) (ny * ny);
    for (x = nx; x--;)
        for (y = ny; y--;)
        {
            xx = (x > nx / 2 ? nx - x : x);
            yy = (y > ny / 2 ? ny - y : y);
            r = (int) (0.5 +
                       sqrt(cx * (double) (xx * xx) +
                            cy * (double) (yy * yy)) / (double) out->scale);
            if (r > n - 1)
                r = n - 1;
            out->values[r] += rho->gray[y * nx + x];
            count[r]++;
        }

    for (r = n; r--;)
        if (count[r])
            out->values[r] /= (float) count[r];
    if (l_flag)
        for (r = n; r--;)
            if (out->values[r])
                out->values[r] = (float) log10((double) out->values[r]);

    free(count);
    mw_delete_fimage(rho);

    return (out);
}
