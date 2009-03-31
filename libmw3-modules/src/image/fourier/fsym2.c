/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fsym2};
 version = {"1.0"};
 author = {"Lionel Moisan"};
 function = {"Symmetrize a Fimage in both directions"};
 usage = {
      'i'->i    "inverse (extract upleft corner)",
      in->in    "input Fimage",
      out<-out  "output Fimage"
};
*/

#include "mw3.h"
#include "mw3-modules.h"         /* for fextract() */

void fsym2(Fimage in, Fimage out, char *i)
{
    int x, y, nx, ny, z;
    float b;

    nx = in->ncol;
    ny = in->nrow;

    if (i)
    {

        /* croping */
        if (nx & 1 || ny & 1)
            mwerror(WARNING, 1, "Non-even image dimensions\n");
        z = 0;
        b = 0.;
        fextract(&b, in, NULL, out, 0, 0, nx / 2 - 1, ny / 2 - 1, &z, &z,
                 NULL);

    }
    else
    {

        /* symmetrization */
        mw_change_fimage(out, 2 * ny, 2 * nx);
        if (!out)
            mwerror(FATAL, 1, "Not enough memory\n");
        for (x = nx; x--;)
            for (y = ny; y--;)
                out->gray[y * nx * 2 + x]
                    = out->gray[y * nx * 2 + nx * 2 - 1 - x]
                    = out->gray[(ny * 2 - 1 - y) * nx * 2 + x]
                    = out->gray[(ny * 2 - 1 - y) * nx * 2 + nx * 2 - 1 - x]
                    = in->gray[y * nx + x];
    }
}
