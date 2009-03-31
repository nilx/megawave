/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {tjpoint};
 version = {"1.2"};
 author = {"Vicent Caselles, Bartomeu Coll, Jacques Froment, Jose-Luis Lisani"};
 labo = {"Universitat de les Illes Balears and CEREMADE"};
 function = {"Return the type of junction of a point in a cimage"};
 usage = {
   'c'->connex8 "8-connexity (default : 4)",
   'a':[tarea=40]->tarea[0,5000] "area threshold",
   'q':[tquant=2]->tquant[0,255] "quantization threshold",
   'l':lambda<-lambda "returns lambda such that {x / U(x) <= lambda}
                       is significant at the junction",
   'm':mu<-mu "returns mu such that {x / U(x) >= mu}
              is significant at the junction",
   'x':xlambda<-xlambda "returns the T-junction point xlambda
                        belonging to the border of {x / U(x) <= lambda}",
   'y':ylambda<-ylambda "returns the T-junction point ylambda
                        belonging to the border of {x / U(x) <= lambda}",
   'X':xmu<-xmu "returns the T-junction point xmu
                belonging to the border of {x / U(x) >= mu}",
   'Y':ymu<-ymu "returns the T-junction point ymu
                belonging to the border of {x / U(x) >= mu}",
   U->U "input cimage U",
   x0->x0 "x coordinate of the point",
   y0->y0 "y coordinate of the point",
   tj<-tjpoint "type of junction at (x0,y0)",
   notused->M  "M array (library call)",
   notused->P  "P array (library call)"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mw.h"
#include "mw-modules.h"

/*----- Module mscarea customised to speed up the computations -----*/

static void compute_area(char *connex8, unsigned char *U, unsigned char *M,
                         int *P, int nrow, int ncol, int a, int b, int x,
                         int y, int l, int stoparea, int *area_sz)
{
    int k;

    if (stoparea <= *area_sz)
        return;

    P[(*area_sz)++] = l;
    /* The area - 1 is obtained at the point (x,y)=l */
    M[l] = 1;
    /* This point is marked */

    /* Left neighbor */
    k = l - 1;
    if ((x > 0) && (M[k] == 0) && (a <= U[k]) && (U[k] <= b))
        compute_area(connex8, U, M, P, nrow, ncol, a, b, x - 1, y, k,
                     stoparea, area_sz);

    /* Upper neighbor */
    k = l - ncol;
    if ((y > 0) && (M[k] == 0) && (a <= U[k]) && (U[k] <= b))
        compute_area(connex8, U, M, P, nrow, ncol, a, b, x, y - 1, k,
                     stoparea, area_sz);

    /* Right neighbor */
    k = l + 1;
    if ((x < ncol - 1) && (M[k] == 0) && (a <= U[k]) && (U[k] <= b))
        compute_area(connex8, U, M, P, nrow, ncol, a, b, x + 1, y, k,
                     stoparea, area_sz);

    /* Lower neighbor */
    k = l + ncol;
    if ((y < nrow - 1) && (M[k] == 0) && (a <= U[k]) && (U[k] <= b))
        compute_area(connex8, U, M, P, nrow, ncol, a, b, x, y + 1, k,
                     stoparea, area_sz);

    if (connex8 != NULL)
    {
        /* Upper left neighbor */
        k = l - ncol - 1;
        if ((x > 0) && (y > 0) && (M[k] == 0) && (a <= U[k]) && (U[k] <= b))
            compute_area(connex8, U, M, P, nrow, ncol, a, b, x - 1, y - 1, k,
                         stoparea, area_sz);
        /* Upper right neighbor */
        k = l - ncol + 1;
        if ((x < ncol - 1) && (y > 0) && (M[k] == 0) && (a <= U[k])
            && (U[k] <= b))
            compute_area(connex8, U, M, P, nrow, ncol, a, b, x + 1, y - 1, k,
                         stoparea, area_sz);
        /* Lower left neighbor */
        k = l + ncol - 1;
        if ((x > 0) && (y < nrow - 1) && (M[k] == 0) && (a <= U[k])
            && (U[k] <= b))
            compute_area(connex8, U, M, P, nrow, ncol, a, b, x - 1, y + 1, k,
                         stoparea, area_sz);
        /* Lower right neighbor */
        k = l + ncol + 1;
        if ((x < ncol - 1) && (y < nrow - 1) && (M[k] == 0) && (a <= U[k])
            && (U[k] <= b))
            compute_area(connex8, U, M, P, nrow, ncol, a, b, x + 1, y + 1, k,
                         stoparea, area_sz);
    }
}

static int fast_mscarea(char *connex8, Cimage U, unsigned char *M, int *P,
                        int stoparea, int a, int b, int x0, int y0)
{
    int area_sz, c;
    int l;

    l = y0 * U->ncol + x0;
    c = U->gray[l];
    if ((a > c) || (c > b))
        return (0);

    area_sz = 0;
    compute_area(connex8, U->gray, M, P, U->nrow, U->ncol,
                 a, b, x0, y0, l, stoparea, &area_sz);

    /* Reset the marks */
    for (l = 0; l < stoparea; l++)
        if (P[l] > 0)
            M[P[l]] = 0;

    return (area_sz);
}

/*----- (end of mscarea)                                       -----*/

/* Return the number of different neighbors relatively to the pixel (x,y) */
/*  + .
    . .             + : pixel (x,y)  . : neigbors considered
*/
/* compute the pixel (xmin,ymin) with minimal gray level min */
/* and the pixel (xmax,ymax) with maximal gray level max     */

static int diff_neighbors(unsigned char *U, int x, int y, int l, int ncol,
                          int q, int *min, int *max, int *xmin, int *ymin,
                          int *xmax, int *ymax)
{
    int nd, d;

    nd = 0;
    d = U[l] - U[l + 1];
    if (d < 0)
        d = -d;
    if (d >= q)
        nd++;
    d = U[l] - U[l + ncol];
    if (d < 0)
        d = -d;
    if (d >= q)
        nd++;
    d = U[l] - U[l + ncol + 1];
    if (d < 0)
        d = -d;
    if (d >= q)
        nd++;
    d = U[l + 1] - U[l + ncol + 1];
    if (d < 0)
        d = -d;
    if (d >= q)
        nd++;
    d = U[l + 1] - U[l + ncol];
    if (d < 0)
        d = -d;
    if (d >= q)
        nd++;
    d = U[l + ncol] - U[l + ncol + 1];
    if (d < 0)
        d = -d;
    if (d >= q)
        nd++;
    if (nd <= 4)
        return (nd);            /* cannot be a junction */

    *min = *max = U[l];
    *xmin = *xmax = x;
    *ymin = *ymax = y;
    if (U[l + 1] < *min)
    {
        *min = U[l + 1];
        *xmin = x + 1;
        *ymin = y;
    }
    if (U[l + 1] > *max)
    {
        *max = U[l + 1];
        *xmax = x + 1;
        *ymax = y;
    }
    if (U[l + ncol + 1] < *min)
    {
        *min = U[l + ncol + 1];
        *xmin = x + 1;
        *ymin = y + 1;
    }
    if (U[l + ncol + 1] > *max)
    {
        *max = U[l + ncol + 1];
        *xmax = x + 1;
        *ymax = y + 1;
    }
    if (U[l + ncol] < *min)
    {
        *min = U[l + ncol];
        *xmin = x;
        *ymin = y + 1;
    }
    if (U[l + ncol] > *max)
    {
        *max = U[l + ncol];
        *xmax = x;
        *ymax = y + 1;
    }
    return (nd);
}

/* Return the type of junction in (x,y) : */
/* 0 : not a junction */
/* 1 : T-junction     */
/* 2 : X-junction     */
/* If (x,y) is a junction computes lambda such that {x/ U(x) <= lambda}
   and {x / U(x) >= mu} are significant at the junction.
*/

static int get_junction(char *connex8, Cimage U, unsigned char *M, int *P,
                        int x, int y, int dn, int ta, int q, int min, int max,
                        int xmin, int ymin, int xmax, int ymax, int *lambda,
                        int *mu)
{
    int a, g, gmin, gmax, xmaxmin1, xmaxmin2, ymaxmin1, ymaxmin2, acc1, acc2;

    if (lambda)
        *lambda = -1;
    if (mu)
        *mu = -1;

    /* Compute gmin = Argmin
     * { min <= g <= max / area({x / U(x) <= g}) >= ta } */
    g = min - 1;
    do
        a = fast_mscarea(connex8, U, M, P, ta, 0, ++g, xmin, ymin);
    while ((a < ta) && (g < max));

    if (a < ta)
        return (0);             /* gmin does not exist : not a T-junction */
    gmin = g;

    /* Compute gmax = Argmax
     * { min <= g <= max / area({x / U(x) >= g}) >= ta } */
    g = max + 1;
    do
        a = fast_mscarea(connex8, U, M, P, ta, --g, 255, xmax, ymax);
    while ((a < ta) && (g > min));

    if (a < ta)
        return (0);             /* gmax does not exist : not a T-junction */
    gmax = g;

    if (gmax - gmin < 2 * q)
        return (0);

    if (lambda)
        *lambda = gmin;
    if (mu)
        *mu = gmax;

    if (xmax == xmin)
        if (x == xmax)
        {
            xmaxmin1 = x + 1;
            xmaxmin2 = x + 1;
        }
        else
        {
            xmaxmin1 = x;
            xmaxmin2 = x;
        }
    else
    {
        xmaxmin1 = x;
        xmaxmin2 = x + 1;
    }

    if (ymax == ymin)
        if (y == ymax)
        {
            ymaxmin1 = y + 1;
            ymaxmin2 = y + 1;
        }
        else
        {
            ymaxmin1 = y;
            ymaxmin2 = y;
        }
    else
    {
        if (((x == xmax) && (y == ymax)) || ((x == xmin) && (y == ymin)))
        {
            ymaxmin1 = y + 1;
            ymaxmin2 = y;
        }
        else
        {
            ymaxmin1 = y;
            ymaxmin2 = y + 1;
        }
    }

    /* Seek area of 1st m.s. at the point (xmaxmin1,ymaxmin1) */
    acc1 =
        fast_mscarea(connex8, U, M, P, ta, gmin + q, gmax - q, xmaxmin1,
                     ymaxmin1);

    if ((acc1 >= ta) && (dn < 6))
        return (1);             /* This is a T-junction */

    /* Seek area of 2nd m.s. at the point (xmaxmin2,ymaxmin2) */
    acc2 =
        fast_mscarea(connex8, U, M, P, ta, gmin + q, gmax - q, xmaxmin2,
                     ymaxmin2);

    if ((acc1 >= ta) && (acc2 >= ta) && (dn == 6))
        return (2);             /* This is a X-junction */

    if ((acc1 >= ta) || (acc2 >= ta))
        return (1);             /* This is a T-junction */
    return (0);                 /* areas of 1st and 2nd m.s.
                                 * to small : not a junction */
}

int tjpoint(char *connex8, int *tarea, int *tquant, Cimage U, int x0, int y0,
            int *lambda, int *mu, int *xlambda, int *ylambda, int *xmu,
            int *ymu, unsigned char *M, int *P)
                  /* Tab to mark the pixels in mscarea */
                  /* Tab to index area -> pixel in mscarea */
{
    int l, tj, dn;
    int min, max, xmin = 0, ymin = 0, xmax = 0, ymax = 0;
    unsigned char delM, delP;

    /* Initialisations, if needed */

    delM = 0;
    if (M == NULL)
    {
        M = (unsigned char *) malloc(U->nrow * U->ncol);
        if (M == NULL)
            mwerror(FATAL, 1, "Not enough memory.\n");
        memset(M, 0, U->nrow * U->ncol);
        delM = 1;

        if ((x0 < 0) || (x0 >= U->ncol - 1))
            mwerror(FATAL, 1,
                    "coordinate x0=%d out of image U or in the right border.\n",
                    x0);
        if ((y0 < 0) || (y0 >= U->nrow - 1))
            mwerror(FATAL, 1,
                    "coordinate y0=%d out of image U or in the low border.\n",
                    y0);
    }

    delP = 0;
    if (P == NULL)
    {
        P = (int *) malloc(*tarea * sizeof(int));
        if (P == NULL)
            mwerror(FATAL, 1, "Not enough memory.\n");
        for (l = 0; l < *tarea; l++)
            P[l] = -1;
        delP = 1;
    }

    l = y0 * U->ncol + x0;
    dn = diff_neighbors(U->gray, x0, y0, l, U->ncol, *tquant,
                        &min, &max, &xmin, &ymin, &xmax, &ymax);
    if (dn > 4)                 /* May be a junction */
        tj = get_junction(connex8, U, M, P, x0, y0, dn, *tarea, *tquant,
                          min, max, xmin, ymin, xmax, ymax, lambda, mu);
    else
        tj = 0;

    if (delP == 1)
        free(P);
    if (delM == 1)
        free(M);

    if (xlambda)
    {
        if (tj > 0)
            *xlambda = xmin;
        else
            *xlambda = -1;
    }
    if (ylambda)
    {
        if (tj > 0)
            *ylambda = ymin;
        else
            *ylambda = -1;
    }
    if (xmu)
    {
        if (tj > 0)
            *xmu = xmax;
        else
            *xmu = -1;
    }
    if (ymu)
    {
        if (tj > 0)
            *ymu = ymax;
        else
            *ymu = -1;
    }

    return (tj);
}
