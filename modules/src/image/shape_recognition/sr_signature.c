/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {sr_signature};
 version = {"1.5"};
 author = {"Thierry Cohignac, Lionel Moisan"};
 function = {"Compute the signature of a shape (density on rings)"};
 usage = {
     'n':[n=20]->n        "number of parameters",
     'a':base->base       "initial set of parameters (Fimage)",
     in->in               "input shape (Fcurves)",
     out<-sr_signature    "final set of parameters"
};
*/
/*----------------------------------------------------------------------
 v1.4: use fkcenter (L.Moisan)
 v1.5 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"         /* for fkcenter(), fkbox() */

/*----- GLOBAL VARIABLE -----*/

static int NPARAM;

/* Compute the size of a Fcurves */

static int size_fcurves(Fcurves cs)
{
    Fcurve c;
    Point_fcurve p;
    int n;

    n = 0;

    for (c = cs->first; c; c = c->next)
        for (p = c->first; p; p = p->next)
            n++;

    return n;
}

/*---------------- Parameters evaluation ---------------*/

static void thickness(Fcurves cs, float *thick, int **Distance)
{
    Point_fcurve p;
    Fcurve c;
    float d1, d2, d3, d4, xg, yg, xmin, ymin, xmax, ymax, radius, distf;
    long surf, diametre, distance;
    int *pdist;
    long i, nb, dx, dy, XG, YG;
    double x, y;

    fkbox(cs, &xmin, &ymin, &xmax, &ymax, NULL, NULL);

    dx = floor(xmax - xmin + .5) + 1;
    dy = floor(ymax - ymin + .5) + 1;

    fkcenter(cs, &xg, &yg);

    XG = floor(xg + .5);
    YG = floor(yg + .5);

    surf = size_fcurves(cs);

    d1 = (float) (XG * XG + YG * YG);
    d2 = (float) (XG * XG + (dy - YG) * (dy - YG));
    d3 = (float) ((dx - XG) * (dx - XG) + YG * YG);
    d4 = (float) ((dx - XG) * (dx - XG) + (dy - YG) * (dy - YG));

    if ((d1 >= d2) && (d1 >= d3) && (d1 >= d4))
        radius = d1;
    if ((d2 >= d1) && (d2 >= d3) && (d2 >= d4))
        radius = d2;
    if ((d3 >= d1) && (d3 >= d2) && (d3 >= d4))
        radius = d3;
    if ((d4 >= d1) && (d4 >= d2) && (d4 >= d3))
        radius = d4;

    radius = (float) sqrt((double) radius);
    diametre = floor(radius + .5);

    if ((*Distance = (int *) malloc((diametre + 1) * sizeof(int))) == NULL)
        mwerror(FATAL, 1, "thickness() : not enough memory\n");

    pdist = *Distance;
    for (i = 0; i <= diametre; i++)
        *(pdist++) = 0.0;

    for (c = cs->first; c; c = c->next)
        for (p = c->first; p; p = p->next)
        {

            x = (double) p->x - (double) xg;
            y = (double) p->y - (double) yg;
            distf = (float) sqrt(x * x + y * y);
            distance = (int) distf + 1;

            (*(*Distance + distance))++;

        }

    nb = 0;
    for (i = 0; i <= diametre; i++)
    {
        nb += *(*Distance + i);
        if ((float) nb / (float) surf >= 0.95)
        {
            *thick = (float) i / (float) NPARAM;
            return;
        }
    }
    *thick = (float) diametre / (float) NPARAM;
}

static void Init_param(float thick, int *surf_cour, float *param,
                       int *treshold)
{
    int i, j, k, r, r2, cpt, distance;
    float s;

    for (i = 0; i <= NPARAM; i++)
    {
        s = (float) i *thick;
        treshold[i] = floor(s + .5);
    }

    for (i = 0; i < NPARAM; i++)
        param[i] = 0.0;

    surf_cour[0] = 0;
    for (k = 1; k <= NPARAM; k++)
    {
        cpt = 0;
        r = treshold[k];
        r2 = treshold[k - 1];
        for (i = 0; i <= r; i++)
            for (j = 0; j <= r; j++)
            {
                s = (float) sqrt((double) (i * i + j * j));
                distance = (int) s + 1;
                if ((distance < r) && (distance >= r2))
                    cpt++;
            }
        if (k == 1)
            surf_cour[k] = 4 * cpt - 4 * (r - r2 - 1) + 1;
        else
            surf_cour[k] = 4 * cpt - 4 * (r - r2);
    }
}

static void Normalize_param(float *param, int *surf_cour)
{
    int i;

    for (i = 0; i < NPARAM; i++)
    {
        param[i] /= (float) surf_cour[i + 1];
        param[i] *= 100.0;
    }
}

static void Density(float *param, int *treshold, int *Distance)
{
    int i, j;

    for (i = 0; i < NPARAM; i++)
        for (j = treshold[i]; j < treshold[i + 1]; j++)
            param[i] += Distance[j];
}

/*------------- MAIN MODULE --------------*/

Fimage sr_signature(Fcurves in, int *n, Fimage base)
{
    float *ptr;
    float thick;
    int *curr_surf, *treshold, *Distance, pn;

    /* default value for n (C call) */
    if (!n)
    {
        pn = 20;
        n = &pn;
    }

    if (base)
    {
        NPARAM = base->ncol;
        base = mw_change_fimage(base, base->nrow + 1, NPARAM);
    }
    else
    {
        NPARAM = *n;
        base = mw_change_fimage(NULL, 1, NPARAM);
    }
    ptr = base->gray + NPARAM * (base->nrow - 1);

    curr_surf = (int *) malloc((NPARAM + 1) * sizeof(int));
    treshold = (int *) malloc((NPARAM + 1) * sizeof(int));

    thickness(in, &thick, &Distance);

    Init_param(thick, curr_surf, ptr, treshold);

    Density(ptr, treshold, Distance);
    Normalize_param(ptr, curr_surf);

    free(Distance);
    free(treshold);
    free(curr_surf);

    return base;
}
