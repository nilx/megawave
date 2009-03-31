/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fftconvol};
 version = {"1.2"};
 author = {"Lionel Moisan"};
 function = {"2D Fourier-convolution of a fimage"};
 usage = {
    in->in           "input Fimage",
    filter->filter   "convolution filter in Fourier domain (Fimage)",
    out<-out         "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.2: allow any image size (not only powers of two !) (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"         /* for fft2d() */

/* NB : calling this module with out=in is possible */

Fimage fftconvol(Fimage in, Fimage filter, Fimage out)
{
    int i, nx, ny;
    Fimage re, im;

    nx = in->ncol;
    ny = in->nrow;

    if (filter->ncol != nx || filter->nrow != ny)
        mwerror(USAGE, 1,
                "Input image and filter dimensions do not match !\n");

    re = mw_new_fimage();
    im = mw_new_fimage();

    /* Fourier transform */
    fft2d(in, NULL, re, im, NULL);

    /* multiplication in Fourier domain */
    for (i = nx * ny; i--;)
    {
        re->gray[i] *= filter->gray[i];
        im->gray[i] *= filter->gray[i];
    }

    out = mw_change_fimage(out, ny, nx);
    if (!out)
        mwerror(FATAL, 1, "Not enough memory\n");

    /* inverse Fourier transform */
    fft2d(re, im, out, NULL, (char *) 1);

    /* free memory */
    mw_delete_fimage(im);
    mw_delete_fimage(re);

    return (out);
}
