/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fkplot};
   version = {"1.3"};
   author = {"Thierry Cohignac, Lionel Moisan"};
   function = {"Plot Fcurves on a Cimage"};
   usage = {
     's'->s_flag      "shift Fcurves to fit exactly in the image",
     in->cs           "input Fcurves",
     out<-out         "output Cimage"
   };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"         /* for fkbox() */

#define BLACK 0
#define WHITE 255

Cimage fkplot(Fcurves cs, Cimage out, char *s_flag)
{
    Fcurve c;
    Point_fcurve p;
    long nx, ny, x, y;
    float xmax, ymax, xmin, ymin;

    fkbox(cs, &xmin, &ymin, &xmax, &ymax, NULL, NULL);

    if (!s_flag)
    {
        xmin = 0.0;
        ymin = 0.0;
    }
    nx = floor(xmax - xmin + .5) + 1;
    ny = floor(ymax - ymin + .5) + 1;
    out = mw_change_cimage(out, ny, nx);
    if (!out)
        mwerror(FATAL, 1, "cannot allocate a %d x %d image\n", nx, ny);
    mw_clear_cimage(out, WHITE);

    for (c = cs->first; c; c = c->next)
        for (p = c->first; p; p = p->next)
        {

            x = floor(p->x - xmin + .5);
            y = floor(p->y - ymin + .5);

            if (x >= 0 && y >= 0)
                out->gray[nx * y + x] = BLACK;
        }

    return out;
}
