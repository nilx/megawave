/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fft2dpol};
 version = {"1.4"};
 author = {"Lionel Moisan"};
 function = {"Polar 2D FFT"};
 usage = {
    'i'->i_flag                  "to compute inverse FFT",
    'I':input_im->in_im          "imaginary input (Fimage)",
    'M':output_rho<-out1         "modulus output (Fimage)",
    'P':output_theta<-out2       "phase output (Fimage) in [-M_PI,M_PI]",
    input_re->in_re              "real input (Fimage)"
};
*/
/*----------------------------------------------------------------------
 v1.2: corrected missing i_flag bug (LM)
 v1.3: minor changes (LM)
 v1.4 (04/2007): change output names to increase readability (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for fft2d() */

/* NB : as for fft2d :
     * calling this module with in_im=NULL is possible and means
       in_im(x,y) = 0 everywhere
     * calling this module with out1=NULL or out2=NULL
       is possible (if you are not interested on one of these outputs)
*/

void fft2dpol(Fimage in_re, Fimage in_im, Fimage out1, Fimage out2,
              char *i_flag)
{
    int i;
    float rho, theta;
    Fimage drho_flag, dtheta_flag;
    double dx, dy;

    if ((!out1) && (!out2))
        mwerror(USAGE, 1, "At least one output needed\n");

  /***** allocate output images if necessary *****/
    drho_flag = out1;
    if (drho_flag == NULL)
        out1 = mw_new_fimage();
    dtheta_flag = out2;
    if (dtheta_flag == NULL)
        out2 = mw_new_fimage();
    if (!out1 || !out2)
        mwerror(FATAL, 1, "Not enough memory.");

  /*** Compute cartesian FFT ***/
    fft2d(in_re, in_im, out1, out2, i_flag);

  /*** Convert result to polar coordinates ***/
    for (i = out1->nrow * out1->ncol; i--;)
    {
        dx = out2->gray[i];
        dy = out1->gray[i];
        rho = (float) sqrt(dx * dx + dy * dy);
        theta = (float) atan2((double) out2->gray[i], (double) out1->gray[i]);
        out1->gray[i] = rho;
        out2->gray[i] = theta;
    }

  /***** free output images if necessary *****/
    if (drho_flag == NULL)
        mw_delete_fimage(out1);
    if (dtheta_flag == NULL)
        mw_delete_fimage(out2);
}
