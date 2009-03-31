/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name     = {fsample};
author   = {"Jacques Froment, Regis Monneau"};
function = {"sampling of a fimage"};
version  = {"1.4"};
usage    = {
   'd':[delta=0.]->delta  "offset in input image",
   'n'->norm              "normalize (multiply by step^2)",
   in->in                 "input image",
   out<-out               "sampled image",
   step->step             "new grid step (factor of sampling)"
           };
*/
/*----------------------------------------------------------------------
 v1.1: return result (L.Moisan)
 v1.2: allow non-integer factor and remove step>=2 condition (L.Moisan)
 v1.3: added -d and -n option (L.Moisan)
 v1.4 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

#define _(a,i,j)  ((a)->gray[(i)*(a)->ncol+(j)] )

Fimage fsample(Fimage in, Fimage out, double step, double *delta, int *norm)
{
    float coeff;
    register int i, j;
    int nr;
    int nc;
    int nr1;
    int nc1;
    int istep, jstep;

    coeff = (norm ? (float) (step * step) : 1.);
    nr = in->nrow;
    nc = in->ncol;
    nr1 = nr;
    while ((floor(*delta + (double) (nr1 - 1) * step)) + 1 > nr)
        nr1--;
    nc1 = nc;
    while ((floor(*delta + (double) (nc1 - 1) * step)) + 1 > nc)
        nc1--;

    mwdebug("Input size: nr = %d \t nc = %d\n", nr, nc);
    mwdebug("Output size: nr1 = %d \t nc1 = %d\n", nr1, nc1);

    out = mw_change_fimage(out, nr1, nc1);
    if (out == NULL)
        mwerror(FATAL, 1, "not enough memory.\n");

    for (i = 0; i < nr1; i++)
        for (j = 0; j < nc1; j++)
        {
            istep = floor(*delta + (double) i * step);
            jstep = floor(*delta + (double) j * step);
            _(out, i, j) = coeff * _(in, istep, jstep);
        }
    return (out);
}
