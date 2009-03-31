/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {iter_gcsf};
   author = {"Frederic Cao, Lionel Moisan"};
   version = {"1.3"};
   function = {"Iterated gcsf"};
   usage = {

 'g':[g=1.]-> gam          "power of curvature",
 'l':[l=1.]->last          "last scale",
 'e':[e=3.]->eps[2.,13.]   "(for gcsf) sampling precision",
 'a':[a=4.]->er_area[2.,13.]  "(for gcsf) erosion area ",
 'n':[n=1]->n              "(for gcsf) or minimal # of iterations",
 'N':[N=5]->N              "number of output curves",
 'v'->v                    "verbose mode",
 in->in                    "input curve (Dlist)",
 out<-out                  "output curves (Dlists)"

            };
*/
/*----------------------------------------------------------------------
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"         /* for gcsf() */

Dlists iter_gcsf(Dlist in, Dlists out, double *gam, double *last,
                 double *er_area, double *eps, int *n, int *N, char *v)
{
    Dlists aux, res;
    double scale1, scale2, rad, rmax, x, y;
    int i;

    rmax = 0;
    for (i = 0; i < in->size; i++)
    {
        x = in->values[2 * i];
        y = in->values[2 * i + 1];
        rad = x * x + y * y;
        if (rad > rmax)
            rmax = rad;
    }
    rad = sqrt(rmax);

    out = mw_change_dlists(out, *N + 1, *N + 1);
    out->list[0] = mw_copy_dlist(in, NULL);
    aux = mw_change_dlists(NULL, 1, 1);
    res = mw_new_dlists();
    scale2 = 0.;
    for (i = 0; i < *N; i++)
    {
        scale1 = scale2;
        scale2 += *last / (double) *N;
        aux->list[0] = out->list[i];
        res =
            gcsf(aux, res, gam, &scale1, &scale2, eps, er_area, n, &rad, v,
                 NULL, NULL);
        out->list[i + 1] = mw_copy_dlist(res->list[0], NULL);
    }
    free(aux);
    mw_delete_dlists(res);

    return out;
}
