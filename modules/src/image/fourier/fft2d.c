/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {fft2d};
 version = {"2.1"};
 author = {"Lionel Moisan"};
 function = {"Rectangular 2D Fast Fourier Transform"};
 usage = {
   'i'->i_flag            "to compute inverse FFT",
   'I':input_im->in_im    "imaginary input (Fimage)",
   'A':output_re<-out_re  "real output (Fimage)",
   'B':output_im<-out_im  "imaginary output (Fimage)",
    input_re->in_re       "real input (Fimage)"
};
*/
/*----------------------------------------------------------------------
 v2.0: allow any image size (not only powers of two !) (Said Ladjal)
 v2.1: improved code, less memory used (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"         /* for fft1d() */

/* NB :
     * calling this module with in_im=NULL is possible and means
       in_im(x,y) = 0 everywhere
     * calling this module with out_re=NULL or out_im=NULL
       is possible (if you are not interested on one of these outputs)
*/

void fft2d(Fimage in_re, Fimage in_im, Fimage out_re, Fimage out_im,
           char *i_flag)
{
    int x, y, nx, ny;
    Fsignal f1_re, f1_im, f2_re, f2_im;
    Fimage dre_flag, dim_flag;

    if ((!out_re) && (!out_im))
        mwerror(USAGE, 1, "At least one output needed\n");

  /***** allocate output images if necessary *****/
    dre_flag = out_re;
    if (dre_flag == NULL)
        out_re = mw_new_fimage();
    dim_flag = out_im;
    if (dim_flag == NULL)
        out_im = mw_new_fimage();

    ny = in_re->nrow;
    nx = in_re->ncol;

    out_re = mw_change_fimage(out_re, ny, nx);
    mw_copy_fimage(in_re, out_re);

    out_im = mw_change_fimage(out_im, ny, nx);
    if (in_im)
    {
        if (in_im->nrow != ny || in_im->ncol != nx)
            mwerror(FATAL, 1, "Input images have no compatible sizes");
        mw_copy_fimage(in_im, out_im);
    }
    else
        mw_clear_fimage(out_im, 0.);

    if (!out_re || !out_im)
        mwerror(FATAL, 1, "Not enough memory.");

  /***** 1D FFT on lines *****/
    f1_re = mw_change_fsignal(NULL, nx);
    f1_im = mw_change_fsignal(NULL, nx);
    f2_re = mw_change_fsignal(NULL, nx);
    f2_im = mw_change_fsignal(NULL, nx);
    if (!f1_re || !f1_im || !f2_re || !f2_im)
        mwerror(FATAL, 1, "Not enough memory.");

    for (y = 0; y < ny; y++)
    {
        for (x = 0; x < nx; x++)
        {
            f1_re->values[x] = out_re->gray[nx * y + x];
            f1_im->values[x] = out_im->gray[nx * y + x];
        }
        fft1d(f1_re, f1_im, f2_re, f2_im, i_flag);
        for (x = 0; x < nx; x++)
        {
            out_re->gray[nx * y + x] = f2_re->values[x];
            out_im->gray[nx * y + x] = f2_im->values[x];
        }
    }

  /***** 1D FFT on columns *****/
    f1_re = mw_change_fsignal(f1_re, ny);
    f1_im = mw_change_fsignal(f1_im, ny);
    f2_re = mw_change_fsignal(f2_re, ny);
    f2_im = mw_change_fsignal(f2_im, ny);
    for (x = 0; x < nx; x++)
    {
        for (y = 0; y < ny; y++)
        {
            f1_re->values[y] = out_re->gray[nx * y + x];
            f1_im->values[y] = out_im->gray[nx * y + x];
        }
        fft1d(f1_re, f1_im, f2_re, f2_im, i_flag);
        for (y = 0; y < ny; y++)
        {
            out_re->gray[nx * y + x] = f2_re->values[y];
            out_im->gray[nx * y + x] = f2_im->values[y];
        }
    }

  /***** free signals *****/
    mw_delete_fsignal(f2_re);
    mw_delete_fsignal(f2_im);
    mw_delete_fsignal(f1_im);
    mw_delete_fsignal(f1_re);

  /***** free output images if necessary *****/
    if (dre_flag == NULL)
        mw_delete_fimage(out_re);
    if (dim_flag == NULL)
        mw_delete_fimage(out_im);
}
