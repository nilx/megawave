/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {sr_normalize};
 version = {"1.3"};
 author = {"Thierry Cohignac, Lionel Moisan"};
 function = {"Normalize a shape using the moments"};
 usage = {
     in->in               "input shape (Fcurves)",
     out<-sr_normalize    "output shape (Fcurves)"
};
*/
/*----------------------------------------------------------------------
 v1.3: uses fkzrt, fkcenter, fkplot (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"         /* for fkplot(), fkzrt(),
                                 * fkcenter(), opening() */

/*** to interpret a Cimage ***/
#define BLACK 0
#define WHITE 255

/*** size of normalization ***/
#define SIZE_NORMALIZATION 30.0

/* add a point to a fcurve */

static void add_fpoint(Fcurve c, float x, float y)
{
    Point_fcurve p;

    p = mw_new_point_fcurve();
    if (!p)
        mwerror(FATAL, 1, "not enough memory \n");
    p->x = x;
    p->y = y;

    p->previous = NULL;
    p->next = c->first;
    if (c->first)
        c->first->previous = p;
    c->first = p;
}

/* put the non-white pixels of a cimage into a fcurve */

static Fcurve cimage_fcurve(Cimage in)
{
    int w, h, i, j;
    Fcurve curve;

    w = in->ncol;
    h = in->nrow;

    curve = mw_new_fcurve();
    if (!curve)
        mwerror(FATAL, 1, "not enough memory\n");

    curve->first = NULL;
    curve->next = NULL;

    for (i = 0; i < h; i++)
    {
        for (j = 0; j < w; j++)
        {

            if (in->gray[i * w + j] != WHITE)

                add_fpoint(curve, (float) j, (float) i);
        }
    }

    return curve;
}

/* Compute the size of a Fcurves */

static int size_fcvs(Fcurves cs)
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

/*** sum of the directions ***/

static void sum_xy(Fcurves cs, double *sx, double *sy)
{
    Fcurve c;
    Point_fcurve p;
    double h;

    *sx = *sy = 0.0;

    for (c = cs->first; c; c = c->next)
        for (p = c->first; p; p = p->next)
            if ((h = sqrt((double) p->x * (double) p->x
                          + (double) p->y * (double) p->y)) != 0.0)
            {
                *sx += (double) p->x / h;
                *sy += (double) p->y / h;
            }
}

/* moments of order p,q */

static float moment_pq(Fcurves cs, int p, int q)
{
    Fcurve c;
    Point_fcurve pp;
    double x, y;
    float result;

    result = 0.0;

    for (c = cs->first; c; c = c->next)
        for (pp = c->first; pp; pp = pp->next)
        {
            x = (double) pp->x;
            y = (double) pp->y;
            if ((x != 0.0 || p != 0.0) && (y != 0.0 || q != 0.0))
                result += (float) (pow(x, (double) p) * pow(y, (double) q));
        }

    return result;
}

/*** Sampling ***/

static Fcurves Sample(Fcurves cs, float close)
{
    Cimage image, image_close;
    Fcurves css;
    int white, n;

    image = fkplot(cs, NULL, (char *) 1);

    if (close != 0.0)
    {

        /* closing with a disc of radius=close */
        n = 1;
        image_close = opening(image, NULL, &close, NULL, &n, NULL);

        mw_delete_cimage(image);
    }
    else
        image_close = image;

    css = mw_new_fcurves();
    white = WHITE;
    css->first = cimage_fcurve(image_close);

    mw_delete_cimage(image_close);

    return css;
}

/*** normalization step 1 ***/

static int Normalize_affine_step1(Fcurves cs, float *determinant)
{
    Fcurve cu;
    Point_fcurve point;
    float x, y;
    float a, b, c, surface;
    float A, B, C;

    surface = size_fcvs(cs);

    a = moment_pq(cs, 2, 0);
    b = moment_pq(cs, 0, 2);
    c = moment_pq(cs, 1, 1);

    if (a == 0.0)
        return 0;

    A = (float) sqrt((double) a);
    A = SIZE_NORMALIZATION / A;
    A *= (float) sqrt((double) surface);

    B = (float) sqrt((double) (b - ((c / a) * c)));
    B = SIZE_NORMALIZATION / B;
    B *= (float) sqrt((double) surface);

    C = c / a;

    *determinant = A * B;

    for (cu = cs->first; cu; cu = cu->next)
        for (point = cu->first; point; point = point->next)
        {

            x = point->x;
            y = point->y;

            point->x = A * x;
            point->y = B * (y - C * x);

        }

    return 1;
}

/*** normalization step 2 ***/

static void Normalize_affine_step2(Fcurves cs)
{
    double alpha;
    double sx, sy;

    sum_xy(cs, &sx, &sy);
    alpha = -atan2(sy, sx);

    fkzrt(cs, 1.0, (float) alpha * 180.0 / M_PI, 0.0, 0.0, NULL);
}

/*------------------------------ MAIN MODULE ------------------------------*/

Fcurves sr_normalize(Fcurves in)
{
    float close;
    float xg = 0.0, yg = 0.0;
    float determinant;

    fkcenter(in, &xg, &yg);
    fkzrt(in, 1.0, 0.0, -xg, -yg, NULL);

    if (Normalize_affine_step1(in, &determinant) == 0)
    {
        /* shape impossible to normalize */
        mwerror(WARNING, 0, "Impossible to normalize this shape\n");
        return in;
    }

    Normalize_affine_step2(in);
    /* close according to the determinant of the affine transformation */
    if (determinant > 1.0)
        close = 2.0;
    else
        close = 1.0;
    return Sample(in, close);
}
