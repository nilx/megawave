/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {ridgelet};
 version = {"1.0"};
 author = {"Claire Jonchery, Amandine Robin"};
 function ={"Ridgelet transform of an image"};
 usage = {
  'I':in_im->in_im        "imaginary input (Fimage N*N)",
  np->np                  "resolution np",
  in_re->in_re            "real input (Fimage N*N)",
  out_re<-out_re          "real ridgelets coefficients (Fimage 2N*2N)",
  out_im<-out_im          "imaginary ridgelets coefficients (Fimage 2N*2N)"
};
*/
/*----------------------------------------------------------------------
 v1.0: initial revision before publication (J.Froment)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "mw3.h"
#include "mw3-modules.h"         /* for stkwave1(), ridgrecpol(),
                                 * fline_extract(), fft2d() */

void ridgelet(Fimage in_re, Fimage in_im, int np, Fimage out_re,
              Fimage out_im)
{
    long N, i, j;
    Fimage pol_re = NULL, pol_im = NULL, fin_re = NULL, fin_im = NULL;
    Fsignal ligne = NULL, t_ond = NULL;

    N = (in_re->nrow);

    fin_re = mw_change_fimage(fin_re, N, N);
    if (!fin_re)
        mwerror(FATAL, 1, "not enough memory !\n");

    fin_im = mw_change_fimage(fin_im, N, N);
    if (!fin_im)
        mwerror(FATAL, 1, "not enough memory !\n");

    out_re = mw_change_fimage(out_re, 2 * N, 2 * N);
    if (!out_re)
        mwerror(FATAL, 1, "not enough memory !\n");

    out_im = mw_change_fimage(out_im, 2 * N, 2 * N);
    if (!out_im)
        mwerror(FATAL, 1, "not enough memory !\n");

    pol_re = mw_change_fimage(pol_re, 2 * N, N);
    if (!pol_re)
        mwerror(FATAL, 1, "not enough memory !\n");

    pol_im = mw_change_fimage(pol_im, 2 * N, N);
    if (!pol_im)
        mwerror(FATAL, 1, "not enough memory !\n");

    fft2d(in_re, in_im, fin_re, fin_im, NULL);

    /* rectangular -> polar grid */
    ridgrecpol(fin_re, fin_im, pol_re, pol_im);

    ligne = mw_change_fsignal(ligne, N);
    if (!ligne)
        mwerror(FATAL, 1, "not enough memory !\n");

    t_ond = mw_change_fsignal(t_ond, 2 * N);
    if (!t_ond)
        mwerror(FATAL, 1, "not enough memory !\n");

    for (i = 0; i < (2 * N); i++)
    {
        fline_extract(NULL, pol_re, ligne, i);
        stkwave1(np, ligne, t_ond);
        for (j = 0; j < (2 * N); j++)
            out_re->gray[i * N * 2 + j] = t_ond->values[j];
    }

    for (i = 0; i < (2 * N); i++)
    {
        fline_extract(NULL, pol_im, ligne, i);
        stkwave1(np, ligne, t_ond);
        for (j = 0; j < (2 * N); j++)
            out_im->gray[i * N * 2 + j] = t_ond->values[j];
    }

    mw_delete_fimage(pol_re);
    mw_delete_fimage(pol_im);
    mw_delete_fsignal(ligne);
    mw_delete_fsignal(t_ond);
    mw_delete_fimage(fin_re);
    mw_delete_fimage(fin_im);
}
