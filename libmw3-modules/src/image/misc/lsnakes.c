/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {lsnakes};
 version = {"1.1"};
 author = {"Francoise Dibos, Jacques Froment, Kamal Lakhiari"};
 function = {"Level Set implementation of the Snakes Model"};
 usage = {
   'n':[Niter=1]->Niter       "number of iterations",
   't':[thre=1.0]->thre       "threshold to binarize mask images",
   'f':[force=0.0001]->force  "force term",
   in->in                     "input contour (Fimage mask)",
   ref->ref                   "input reference Fimage",
   out<-lsnakes               "output contour (Fimage mask)"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

#define min(A,B)     (((A)>(B)) ? (B) : (A))
#define max(A,B)     (((A)>(B)) ? (A) : (B))

/* Note: (i,j) has the same meaning as (x,y) */
#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)] )

#define __(a,p) ((a)->gray[(p)])

/* Constants of the algorithm */
#define ht 5.0

/* Compute z6 & z7 constants from the value of ht using the formula: */
/* z6 = 1.0/(1.0+2.0*ht) and  z7 = 2.0*ht/(1.0+2.0*ht) */
/* WARNING: because of instability of this computation from one
 * architecture  to another (e.g. sun4/hp), do not let the computer to
 * do that ! */
#define z6 0.090910
#define z7 0.909091
#define h 0.208
#define e1 2300

/* Smooth the input image along the x and y directions. Output is image */
static void smooth_image(Fimage image)
{
    int i, j;
    float s0 = 0.0, s1 = 0.0;

    /* x direction */

    for (j = 0; j < image->nrow; j++)
    {
        for (i = 1; i < image->ncol; i++)
        {
            if (i < image->ncol - 1)
                s1 = (_(image, i - 1, j) + _(image, i, j) +
                      _(image, i + 1, j)) / 3.0;
            if (i > 1)
                _(image, i - 1, j) = s0;
            s0 = s1;
        }
        _(image, 0, j) = _(image, 1, j);
        _(image, i - 1, j) = _(image, i - 2, j);
    }

    /* y direction */

    for (i = 0; i < image->ncol; i++)
    {
        for (j = 1; j < image->nrow; j++)
        {
            if (j < image->nrow - 1)
                s1 = (_(image, i, j - 1) + _(image, i, j) +
                      _(image, i, j + 1)) / 3.0;
            if (j > 1)
                _(image, i, j - 1) = s0;
            s0 = s1;
        }
        _(image, i, 0) = _(image, i, 1);
        _(image, i, j - 1) = _(image, i, j - 2);
    }
}

/* Calcul of gradient */

static void fgradient(Fimage g, Fimage d, Fimage e)
{
    short i, j, k, l;
    int im = g->ncol;
    int jm = g->nrow;
    float c1, d1;
    for (i = 1; i < im - 1; i++)
    {
        for (j = 1; j < jm - 1; j++)
        {
            c1 = _(g, i + 1, j + 1) - _(g, i - 1, j - 1);
            d1 = _(g, i - 1, j + 1) - _(g, i + 1, j - 1);
            _(d, i, j) =
                _(g, i + 1, j) - _(g, i - 1, j)
                + 0.707106781186547129 * (c1 - d1);
            _(e, i, j) =
                _(g, i, j + 1) - _(g, i, j - 1)
                + 0.707106781186547129 * (c1 + d1);
        }
    }

    for (k = 0; k < im; k++)
    {
        for (l = 0; l < jm; l++)
        {
            _(d, k, 0) = _(e, k, 0) = 0.;
            _(d, k, jm - 1) = _(e, k, jm - 1) = 0.;
            _(d, im - 1, l) = _(e, im - 1, l) = 0.;
            _(d, 0, l) = _(e, 0, l) = 0.;
        }
    }
}

/*Determination of contours*/

static void fsqrtmod(Fimage d, Fimage e, Fimage f)
{
    short i, j;
    int im = d->ncol;
    int jm = d->nrow;
    float c2, d2;
    for (i = 0; i < im; i++)
    {
        for (j = 0; j < jm; j++)
        {
            c2 = _(d, i, j) * _(d, i, j);
            d2 = _(e, i, j) * _(e, i, j);
            _(f, i, j) = h * sqrt(c2 + d2);
        }
    }
}

/* Return the squared modulus of the smoothed gradient of image */

static Fimage get_mod2_smooth_gradient(Fimage image, Fimage Dx, Fimage Dy)
{
    register int i, j;
    Fimage mod;
    float dx2, dy2;

    mod = mw_change_fimage(NULL, image->nrow, image->ncol);
    if (mod == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");

    fgradient(image, Dx, Dy);

    smooth_image(Dx);
    smooth_image(Dy);
    /* Attention: if faudrait peut-etre filtrer d'abord */
    /* / y puis / x. */

    for (i = 0; i < image->ncol; i++)
        for (j = 0; j < image->nrow; j++)
        {
            dx2 = _(Dx, i, j);
            dx2 *= dx2;
            dy2 = _(Dy, i, j);
            dy2 *= dy2;
            _(mod, i, j) = dx2 + dy2;
        }

    return (mod);
}

/* Return the "force" matrix */

static Fimage get_force_matrix(Fimage image, float A)
{
    register int i, j;
    Fimage f;
    float y, z;

    f = mw_change_fimage(NULL, image->nrow, image->ncol);
    if (f == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");

    y = A * ht;
    z = A * 0.25;
    for (i = 0; i < image->ncol; i++)
        for (j = 0; j < image->nrow; j++)
            if (_(image, i, j) < e1)
                _(f, i, j) = y;
            else
                _(f, i, j) = z;

    return (f);
}

/* Build internal coefficients Dxy, Dyx, Dx, Dy and g */

static void build(Fimage a, Fimage g0, Fimage Dx, Fimage Dy, Fimage Dxy,
                  Fimage Dyx, Fimage g)
{
    register int i, j;
    float z, z1, z2, z3, z4, z5, z9;
    float w1, w2, w3, f0, f1;

    mwdebug("\t Enter build fct...\n");

    for (i = 0; i < Dx->ncol; i++)
        for (j = 0; j < Dy->nrow; j++)
        {
            z1 = _(Dx, i, j);
            z1 *= z1;
            z2 = _(Dy, i, j);
            z2 *= z2;
            z = z1 + z2;
            if (z < 1)
            {
                if (_(g0, i, j) < e1)
                    z9 = ht;
                else
                    z9 = 0.25;
                w1 = 1.0 + 3 * z9;
                w2 = 0.5 * z9 / w1;
                w3 = 0.5 * w2;
                _(Dx, i, j) = _(Dy, i, j) = w2;
                _(Dxy, i, j) = _(Dyx, i, j) = w3;
                _(g, i, j) = _(a, i, j) / w1;
            }
            else                /* z >= 1 */
            {
                z3 = _(Dx, i, j) * _(Dy, i, j) / z;
                if (_(g0, i, j) < e1)
                    z9 = ht;
                else
                    z9 = 0.25;
                if (z3 <= 0.0)
                {
                    f0 = 1.0 + 2.0 * z3;
                    f0 *= f0;
                    f1 = -2.0 * z3;
                    f1 *= f1;
                    if ((f0 + f1) > 0.8)
                    {
                        if (f0 >= f1)
                        {
                            f0 = 1.0;
                            f1 = 0.0;
                        }
                        else
                        {
                            f0 = 0.0;
                            f1 = 1.0;
                        }
                    }
                    z4 = 1.0 + z9 * (2.0 * f0 + f1);
                    _(g, i, j) = _(a, i, j) / z4;
                    z5 = z9 / z4;
                    _(Dxy, i, j) = z5 * f1 * 0.5;
                    _(Dyx, i, j) = 0.0;

                    if (z1 <= z2)
                    {
                        _(Dx, i, j) = f0 * z5;
                        _(Dy, i, j) = 0.0;
                    }
                    else
                    {
                        _(Dx, i, j) = 0.0;
                        _(Dy, i, j) = f0 * z5;
                    }
                }
                else            /* z3 > 0 */
                {
                    f0 = 1.0 - 2.0 * z3;
                    f0 *= f0;
                    f1 = 2.0 * z3;
                    f1 *= f1;
                    if ((f0 + f1) > 0.8)
                    {
                        if (f0 >= f1)
                        {
                            f0 = 1.0;
                            f1 = 0.0;
                        }
                        else
                        {
                            f0 = 0.0;
                            f1 = 1.0;
                        }
                    }
                    z4 = 1.0 + z9 * (2.0 * f0 + f1);
                    _(g, i, j) = _(a, i, j) / z4;
                    z5 = z9 / z4;
                    _(Dxy, i, j) = 0.0;
                    _(Dyx, i, j) = z5 * f1 * 0.5;

                    if (z1 <= z2)
                    {
                        _(Dx, i, j) = f0 * z5;
                        _(Dy, i, j) = 0.0;
                    }
                    else
                    {
                        _(Dx, i, j) = 0.0;
                        _(Dy, i, j) = f0 * z5;
                    }
                }
            }
        }

    mwdebug("\t Exit build fct...\n");
}

/* Solve the system */

static void solve(Fimage a, Fimage Dx, Fimage Dy, Fimage Dxy, Fimage Dyx,
                  Fimage g, Fimage f, Fimage ng, int iter, int Niter)
{
    float u;
    register int i, j;
    register float *ptr1, *ptr2, *ptr3;
    int im, jm, l, n_sist;
    int P, P1, P2, P3, P4, P5, P6, P7, P8;

    mwdebug("\t Enter solve fct...\n");

    u = ht;
    n_sist = sqrt(u + 1);

    im = g->ncol - 1;
    jm = g->nrow - 1;

    /* Border Initialization */
    for (i = 0; i <= im; i++)
    {
        _(g, i, 0) = _(a, i, 0) * z6;
        _(g, i, jm) = _(a, i, jm) * z6;
    }
    for (j = 0; j <= jm; j++)
    {
        _(g, 0, j) = _(a, 0, j) * z6;
        _(g, im, j) = _(a, im, j) * z6;
    }

    for (l = 1; l <= n_sist; l++)
    {

        /* Border */
        for (i = 0; i <= im; i++)
        {
            _(a, i, 0) = _(g, i, 0) + z7 * _(a, i, 1);
            _(a, i, jm) = _(g, i, jm) + z7 * _(a, i, jm - 1);
        }
        for (j = 0; j <= jm; j++)
        {
            _(a, im, j) = _(g, im, j) + z7 * _(a, im - 1, j);
            _(a, 0, j) = _(g, 0, j) + z7 * _(a, 1, j);
        }

        /* Inside */

        for (i = 1; i <= im - 1; i++)
            for (j = 1; j <= jm - 1; j++)
            {
                P = j * a->ncol + i;    /* i,j */
                P1 = P + 1;     /* i+1,j */
                P2 = P1 + a->ncol;      /* i+1,j+1 */
                P3 = P + a->ncol;       /* i,j+1 */
                P4 = P3 - 1;    /* i-1,j+1 */
                P5 = P - 1;     /* i-1, j */
                P6 = P5 - a->ncol;      /* i-1, j-1 */
                P7 = P - a->ncol;       /* i,j-1 */
                P8 = P7 + 1;    /* i+1.j-1 */

                __(a, P) = __(g, P) +
                    __(Dx, P) * (__(a, P1) + __(a, P5)) +
                    __(Dy, P) * (__(a, P3) + __(a, P7)) +
                    __(Dxy, P) * (__(a, P2) + __(a, P6)) +
                    __(Dyx, P) * (__(a, P8) + __(a, P4));
            }

/*      if (iter == Niter) IT=2; else IT=1; */
/* LAST ITERATION is a special case */
/*      for (it=1;it<=IT;it++) */
        for (i = im - 1; i >= 1; i--)
            for (j = jm - 1; j >= 1; j--)
            {
                P = j * a->ncol + i;    /* i,j */
                P1 = P + 1;     /* i+1,j */
                P2 = P1 + a->ncol;      /* i+1,j+1 */
                P3 = P + a->ncol;       /* i,j+1 */
                P4 = P3 - 1;    /* i-1,j+1 */
                P5 = P - 1;     /* i-1, j */
                P6 = P5 - a->ncol;      /* i-1, j-1 */
                P7 = P - a->ncol;       /* i,j-1 */
                P8 = P7 + 1;    /* i+1.j-1 */

                __(a, P) = __(g, P) +
                    __(Dx, P) * (__(a, P1) + __(a, P5)) +
                    __(Dy, P) * (__(a, P3) + __(a, P7)) +
                    __(Dxy, P) * (__(a, P2) + __(a, P6)) +
                    __(Dyx, P) * (__(a, P8) + __(a, P4));
            }

        /* Border */
        for (i = 0; i <= im; i++)
        {
            _(a, i, 0) = _(g, i, 0) + z7 * _(a, i, 1);
            _(a, i, jm) = _(g, i, jm) + z7 * _(a, i, jm - 1);
        }
        for (j = 0; j <= jm; j++)
        {
            _(a, im, j) = _(g, im, j) + z7 * _(a, im - 1, j);
            _(a, 0, j) = _(g, 0, j) + z7 * _(a, 1, j);
        }

        if (iter < Niter)       /* LAST ITERATION is a special case */
        {
            for (i = 0, ptr1 = a->gray, ptr2 = f->gray, ptr3 = ng->gray;
                 i <= im * jm; i++, ptr1++, ptr2++, ptr3++)
                *ptr1 += *ptr2 * *ptr3;
        }

    }

    mwdebug("\t Exit solve fct...\n");
}

/* Binarize the fimage a, by putting 255 for gray levels >= c, 0 elsewhere */

static void fbinarize(Fimage a, float c)
{
    register float *ptr;
    register int i;

    for (i = 0, ptr = a->gray; i < a->ncol * a->nrow; i++, ptr++)
        if (*ptr >= c)
            *ptr = 255.0;
        else
            *ptr = 0.0;

}

/*------------------------------ Main function ------------------------------*/

Fimage lsnakes(Fimage in, Fimage ref, int *Niter, float *thre, float *force)
                /* Original contour mask and new one */
                 /* Original image */
{
    Fimage g0;                  /* squared modulus smoothed gradient
                                 * of the image ref */
    Fimage ng;                  /* weighted modulus gradient of the image in */
    Fimage f;                   /* force matrix */
    Fimage Dx, Dy;              /* Gradient vector. For use in
                                 * gradient procedures */
    Fimage Dxy, Dyx, g;         /* For use in build() and solve() */

    int i;                      /* # of the step */

    /* Create user's image buffers */

    ng = mw_change_fimage(NULL, in->nrow, in->ncol);
    if (ng == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");
    Dx = mw_change_fimage(NULL, in->nrow, in->ncol);
    if (Dx == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");
    Dy = mw_change_fimage(NULL, in->nrow, in->ncol);
    if (Dy == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");
    Dxy = mw_change_fimage(NULL, in->nrow, in->ncol);
    if (Dxy == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");
    Dyx = mw_change_fimage(NULL, in->nrow, in->ncol);
    if (Dyx == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");
    g = mw_change_fimage(NULL, in->nrow, in->ncol);
    if (g == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");

    /* Begin */

    g0 = get_mod2_smooth_gradient(ref, Dx, Dy);
    f = get_force_matrix(g0, *force);

    for (i = 1; i <= *Niter; i++)
    {
        mwdebug("Begin iteration : %d\n", i);

        fgradient(in, Dx, Dy);
        fsqrtmod(Dx, Dy, ng);
        build(in, g0, Dx, Dy, Dxy, Dyx, g);
        solve(in, Dx, Dy, Dxy, Dyx, g, f, ng, i, *Niter);

        mwdebug("End of iteration : %d\n", i);
    }

    /* End */

    fbinarize(in, *thre);

    mw_delete_fimage(f);
    mw_delete_fimage(g0);
    mw_delete_fimage(g);
    mw_delete_fimage(Dyx);
    mw_delete_fimage(Dxy);
    mw_delete_fimage(Dy);
    mw_delete_fimage(Dx);
    mw_delete_fimage(ng);

    return (in);
}
