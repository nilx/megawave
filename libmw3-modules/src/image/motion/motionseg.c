/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {motionseg};
 version = {"1.1"};
 author = {"Toni Buades"};
 function = {"Motion Segmentation (Aubert-Deriche-Kornprobst method)"};
 usage = {
   'n':[n=100]->n             "Maximum number of iterations",
   'p':[prec=0.001]->prec     "Numerical precision",
   'e':[e=0.0005]-> e         "Parameter of relaxation" ,
   'a':[alphac=1000]->alphac  "Parameter alphac of the functional",
   'b':[alphabr=100]->alphabr "Parameter alphabr of the fuctional",
   'c':[alphacr=10]->alphacr  "Parameter alphacr of the functional",
   's':[s=0.25]->seu          "threshold",
   fmov1->N                   "Original Fmovie (input)",
   fmov2<-C                   "Motion segmentation (output Fmovie)" ,
   fim<-B                     "Background estimation (output Fimage)"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

#define _(a,i,j)  ((a)->gray[(j)*(a)->ncol+(i)])

/*--------Definition fonction de la derivee de phi epsilon-----------*/
/*--- On Utilise phi(t)=sqrt(1+t*t)-1 ---------------*/
static float dphi(float x, float e)
{
    if (x <= e)
        return x / sqrt(1 + e * e);
    else if (x <= (1. / e))
        return (x / sqrt(1 + x * x));
    else
        return (e * x / sqrt(1 + e * e));
}

/*---------------------------------------------------*/

/*----------- Garde memoire pour le film   ------- */
static void Film_Allocate(Fmovie out, int n, int x, int y)
{
    Fimage image, image_prev;
    int l;

    out = mw_change_fmovie(out);
    if (out == NULL)
        mwerror(FATAL, 1, "Not enough memory\n");

    for (l = 0; l < n; l++)
    {
        if ((image = mw_change_fimage(NULL, y, x)) == NULL)
        {
            mw_delete_fmovie(out);
            mwerror(FATAL, 1, "Not enough memory\n");
        }
        if (l == 0)
            out->first = image;
        else
        {
            image_prev->next = image;
            image->previous = image_prev;
        }
        image_prev = image;
    }
    image->next = NULL;
}

/*-----------------------------------------*/

/*------Calcule d(i,j)=phi'(|gradB|)/(|gradB|)--*/

static void calcula_D(Fimage Orig, Fimage D, float e1)
{
    int x, y;
    float ux, uy, mgrad, a = 0.0005;

    for (y = 0; y < Orig->nrow; y++)
        for (x = 0; x < Orig->ncol; x++)
        {
            if (x == 0)
                ux = _(Orig, x + 1, y) - _(Orig, x, y);
            else if (x == Orig->ncol - 1)
                ux = _(Orig, x, y) - _(Orig, x - 1, y);
            else
                ux = (_(Orig, x + 1, y) - _(Orig, x - 1, y)) * 0.5;

            if (y == 0)
                uy = _(Orig, x, y + 1) - _(Orig, x, y);
            else if (y == Orig->nrow - 1)
                uy = _(Orig, x, y) - _(Orig, x, y - 1);
            else
                uy = (_(Orig, x, y + 1) - _(Orig, x, y - 1)) * 0.5;

            mgrad = sqrt(ux * ux + uy * uy);
            _(D, x, y) = dphi(mgrad, e1) / (2 * mgrad + a);
        }

}

/*----------------------------------------------*/

static void calcula_B(Fimage Orig, Fimage D, Fmovie Nh, Fmovie Ch,
                      float alphabr1)
{
    int x, y;
    Fimage u1, u2;
    float dt, db, dr, dl;
    float bt, bb, br, bl;
    float suma, suma1, suma2;
    for (y = 0; y < Orig->nrow; y++)
        for (x = 0; x < Orig->ncol; x++)
        {
            if (x == 0)
            {
                bl = 0;
                dl = 0;
                br = _(Orig, x + 1, y);
                dr = (_(D, x + 1, y) + _(D, x, y)) * 0.5;
            }
            else if (x == Orig->ncol - 1)
            {
                dr = 0;
                br = 0;
                bl = _(Orig, x - 1, y);
                dl = (_(D, x - 1, y) + _(D, x, y)) * 0.5;
            }
            else
            {
                br = _(Orig, x + 1, y);
                dr = (_(D, x + 1, y) + _(D, x, y)) * 0.5;
                bl = _(Orig, x - 1, y);
                dl = (_(D, x - 1, y) + _(D, x, y)) * 0.5;
            }

            if (y == 0)
            {
                bt = 0;
                dt = 0;
                bb = _(Orig, x, y + 1);
                db = (_(D, x, y) + _(D, x, y + 1)) * 0.5;
            }
            else if (y == Orig->nrow - 1)
            {
                bb = 0;
                db = 0;
                bt = _(Orig, x, y - 1);
                dt = (_(D, x, y) + _(D, x, y - 1)) * 0.5;
            }
            else
            {
                bt = _(Orig, x, y - 1);
                dt = (_(D, x, y - 1) + _(D, x, y)) * 0.5;
                bb = _(Orig, x, y + 1);
                db = (_(D, x, y + 1) + _(D, x, y)) * 0.5;
            }

            u1 = Nh->first;
            u2 = Ch->first;
            suma1 = 0;
            suma2 = 0;
            while (u1 != NULL)
            {
                suma1 += _(u1, x, y) * _(u2, x, y) * _(u2, x, y);
                suma2 += _(u2, x, y) * _(u2, x, y);
                u1 = u1->next;
                u2 = u2->next;
            }
            suma = suma2 + alphabr1 * (dt + db + dl + dr);
            _(Orig, x, y) =
                alphabr1 * (1. / suma) * (db * bb + dt * bt + dr * br +
                                          dl * bl) + (suma1 / suma);

        }

}

/*---------------------------*/

static void calcula_Ch(Fimage Ch, Fimage D, Fimage Fons, Fimage Nh,
                       float alphac1, float alphacr1)
{
    int x, y;
    float dt, db, dr, dl;
    float ct, cb, cr, cl;
    float suma;
    for (y = 0; y < Ch->nrow; y++)
        for (x = 0; x < Ch->ncol; x++)
        {
            if (x == 0)
            {
                cl = 0;
                dl = 0;
                cr = _(Ch, x + 1, y);
                dr = (_(D, x + 1, y) + _(D, x, y)) * 0.5;
            }
            else if (x == Ch->ncol - 1)
            {
                dr = 0;
                cr = 0;
                cl = _(Ch, x - 1, y);
                dl = (_(D, x - 1, y) + _(D, x, y)) * 0.5;
            }
            else
            {
                cr = _(Ch, x + 1, y);
                dr = (_(D, x + 1, y) + _(D, x, y)) * 0.5;
                cl = _(Ch, x - 1, y);
                dl = (_(D, x - 1, y) + _(D, x, y)) * 0.5;
            }

            if (y == 0)
            {
                ct = 0;
                dt = 0;
                cb = _(Ch, x, y + 1);
                db = (_(D, x, y) + _(D, x, y + 1)) * 0.5;
            }
            else if (y == Ch->nrow - 1)
            {
                cb = 0;
                db = 0;
                ct = _(Ch, x, y - 1);
                dt = (_(D, x, y) + _(D, x, y - 1)) * 0.5;
            }
            else
            {
                ct = _(Ch, x, y - 1);
                dt = (_(D, x, y - 1) + _(D, x, y)) * 0.5;
                cb = _(Ch, x, y + 1);
                db = (_(D, x, y + 1) + _(D, x, y)) * 0.5;
            }

            suma =
                alphac1 + (_(Fons, x, y) - _(Nh, x, y)) * (_(Fons, x, y) -
                                                           _(Nh, x,
                                                             y)) +
                alphacr1 * (db + dt + dr + dl);

            _(Ch, x, y) =
                alphacr1 * (dt * ct + db * cb + dl * cl + dr * cr) / suma +
                alphac1 / suma;

        }

}

/*------------------------------ MAIN MODULE ------------------------------*/

void motionseg(int *n, float *prec, float *e, float *alphac, float *alphabr,
               float *alphacr, float *seu, Fmovie N, Fmovie C, Fimage B)
{

    int nx, ny, n_images, l, k, it;
    Fimage u = NULL, Anterior = NULL, D1 = NULL, v = NULL;
    float *ptrB, *ptrAnt, *ptrfilm, diff, error;

    u = N->first;
    n_images = 0;
    while (u != NULL)
    {
        n_images++;
        u = u->next;
    }

    nx = N->first->ncol;
    ny = N->first->nrow;

    Film_Allocate(C, n_images, nx, ny);

    Anterior = mw_change_fimage(Anterior, ny, nx);
    if (Anterior == NULL)
        mwerror(FATAL, 1, "Not enough memory \n");

    B = mw_change_fimage(B, ny, nx);
    if (B == NULL)
        mwerror(FATAL, 1, "Not enough memory \n");

    D1 = mw_change_fimage(D1, ny, nx);
    if (D1 == NULL)
        mwerror(FATAL, 1, "Not enough memory \n");

  /*---Initialize B to N1-------------------*/

    ptrB = B->gray;

    for (l = 0; l < nx * ny; l++, ptrB++)
        *ptrB = 0;

  /*---------------------------------*/

  /*---Initialize C[ i ] to 1--------------------*/
    u = C->first;
    for (l = 0; l < n_images; l++, u = u->next)
    {
        ptrfilm = u->gray;
        for (k = 0; k < nx * ny; k++, ptrfilm++)
            *ptrfilm = 1;
    }
  /*------------------------------------*/

    error = *prec + 1;
    for (it = 0; it < *n && error > *prec; it++)
    {

        printf("iteration %d / %d\n", it + 1, *n);

        mw_copy_fimage(B, Anterior);

        calcula_D(B, D1, *e);

        calcula_B(B, D1, N, C, *alphabr);

        ptrB = B->gray;
        ptrAnt = Anterior->gray;
        error = 0;
        for (l = 0; l < nx * ny; l++, ptrB++, ptrAnt++)
        {
            if (*ptrB != 0)
                diff =
                    fabs((double) (*ptrB - *ptrAnt)) / fabs((double) *ptrB);
            else
                diff = fabs((double) (*ptrB - *ptrAnt));

            if (diff > error)
                error = diff;
        }

        u = C->first;
        v = N->first;
        for (l = 0; l < n_images; l++, u = u->next, v = v->next)
        {
            mw_copy_fimage(u, Anterior);
            calcula_D(u, D1, *e);
            calcula_Ch(u, D1, B, v, *alphac, *alphacr);
            ptrfilm = u->gray;
            ptrAnt = Anterior->gray;
            for (k = 0; k < nx * ny; k++, ptrfilm++, ptrAnt++)
            {
                if (*ptrfilm != 0)
                    diff =
                        fabs((double) (*ptrfilm - *ptrAnt)) /
                        fabs((double) *ptrfilm);
                else
                    diff = fabs((double) (*ptrfilm - *ptrAnt));

                if (diff > error)
                    error = diff;
            }

        }
    }

    u = C->first;
    for (l = 0; l < n_images; l++, u = u->next)
    {
        ptrfilm = u->gray;
        for (k = 0; k < nx * ny; k++, ptrfilm++)
            if (*ptrfilm > *seu)
                *ptrfilm = 255;
            else
                *ptrfilm = 0;
    }

}
