/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fksmooth};
   author = {"Lionel Moisan"};
   version = {"1.2"};
   function = {"Apply Euclidean heat equation to a curve"};
   usage = {
  'n':[n=10]->n       "number of iterations",
  's':[std=2.]->std   "standart deviation for Gaussian kernel",
  't':[t=1.]->t       "space quantization step",
  'P'->P              "to prevent Euclidean normalization",
  in->in              "input curve (Flist)",
  out<-fksmooth       "output smoothed curve (Flist, modified input)"
   };
*/
/*----------------------------------------------------------------------
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for sgauss() */

/* distance between two points */
static double dist(const float *p, const float *q)
{
    double x, y;
    x = (double) (*p - *q);
    y = (double) (*(p + 1) - *(q + 1));

    return sqrt(x * x + y * y);
}

static void fksample(Flist in, Flist out, float t)
{
    double per, cur, step, d, l;
    int i, j, k, n, max;

    /* compute global perimeter */
    for (i = 1, per = 0.; i < in->size; i++)
        per += dist(in->values + i * 2 - 2, in->values + i * 2);

    max = (int) (per / (double) t);
    step = per / (double) max;
    out = mw_change_flist(out, max + 3, 0, 2);
    out->values[0] = in->values[0];
    out->values[1] = in->values[1];
    j = 1;
    cur = 0.;
    for (i = 1; i < in->size; i++)
    {
        d = dist(in->values + i * 2 - 2, in->values + i * 2);
        if (cur + d > step)
        {
            n = (int) ((cur + d) / step);
            for (k = 0; k < n; k++)
            {
                l = (step - cur + (double) k * step) / d;
                out->values[j * 2] =
                    l * in->values[i * 2] + (1. - l) * in->values[i * 2 - 2];
                out->values[j * 2 + 1] =
                    l * in->values[i * 2 + 1] + (1. - l) * in->values[i * 2 -
                                                                      1];
                j++;
            }
            cur -= (double) n *step;
        }
        cur += d;
    }
    if (dist(out->values + j * 2 - 2, out->values) > step * 0.1)
    {
        out->values[j * 2 - 2] = out->values[0];
        out->values[j * 2 - 1] = out->values[1];
    }
    else
    {
        out->values[j * 2] = out->values[0];
        out->values[j * 2 + 1] = out->values[1];
        j++;
    }
    out->size = j;
}

static void convol(Flist in, Flist out, Fsignal s)
{
    int i, j, n;
    float v, *ptr;

    n = in->size;
    out = mw_change_flist(out, n, n, 2);
    mw_clear_flist(out, 0.);
    for (i = 0; i < s->size; i++)
    {
        v = s->values[i];
        for (j = 0; j < n; j++)
        {
            ptr = out->values + ((j - i + n * s->size) % n) * 2;
            *(ptr++) += v * in->values[j * 2];
            *ptr += v * in->values[j * 2 + 1];
        }
    }
}

/*------------------------------ MAIN MODULE ------------------------------*/

Flist fksmooth(Flist in, int *n, float *std, float *t, char *P)
{
    Flist tmp;
    Fsignal g;
    int i;
    float precision;

    if (!in || !in->size)
        return (in);
    precision = 1.;
    g = sgauss(*std, NULL, NULL, &precision);
    tmp = mw_change_flist(NULL, in->size, 0, 2);
    for (i = *n; i--;)
    {
        if (!P)
            fksample(in, tmp, *t);
        else
            mw_copy_flist(in, tmp);
        convol(tmp, in, g);
    }
    mw_delete_flist(tmp);
    mwdebug("final #points: %d\n", in->size);

    return (in);
}
