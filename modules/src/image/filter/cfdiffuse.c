/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cfdiffuse};
version = {"1.1"};
author = {"Antonin Chambolle, Jacques Froment"};
function = {"One-step Diffusion of a Color Float Image
           using Total Variation minimization"};
usage = {
 't':[deltat=10.0]->deltat  "Time for the diffusion",
 'l':[epsilon=1.0]->epsilon [0.1,100.0] "Lower bound for the RGB norm",
 in->in              "original image (input cfimage)",
 out<-out            "diffused image (output cfimage)",
 notused -> MDiag0   "Diagonal of the matrix #0 (internal use)",
 notused -> MDiag1   "Diagonal of the matrix #1 (internal use)",
 notused -> U0       "Vector of real values (internal use)",
 notused -> Yimage   "Vector of RGB values (internal use)",
 notused -> Vimage   "Vector of RGB values (internal use)",
 notused -> L2h      "Horizontal Lambda matrix (internal use)",
 notused -> L2v      "Vertical Lambda matrix (internal use)"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

#define EPSILON 0.5
#define norm sqrt(dr*dr + dg*dg + db*db)
#define wgh(X) (((X)<epsilon)? (deltat/epsilon):(deltat/(X)))

/* Compute L2h, L2v */
static void set_lambda(Cfimage I, Fimage L2h, Fimage L2v, float deltat,
                       float epsilon)
{
    int sxy;
    int i, j;
    double dr, dg, db, x;

    sxy = I->ncol * I->nrow;
    for (j = 0; j < sxy - I->ncol; j += I->ncol)
    {
        for (i = 0; i < I->ncol - 1; i++)
        {
            dr = I->red[i + j + 1] - I->red[i + j];
            dg = I->green[i + j + 1] - I->green[i + j];
            db = I->blue[i + j + 1] - I->blue[i + j];
            x = norm;
            L2h->gray[i + j] = wgh(x);
            dr = I->red[i + j + I->ncol] - I->red[i + j];
            dg = I->green[i + j + I->ncol] - I->green[i + j];
            db = I->blue[i + j + I->ncol] - I->blue[i + j];
            x = norm;
            L2v->gray[i + j] = wgh(x);
        }
        L2h->gray[i + j] = 0.0;
        dr = I->red[i + j + I->ncol] - I->red[i + j];
        dg = I->green[i + j + I->ncol] - I->green[i + j];
        db = I->blue[i + j + I->ncol] - I->blue[i + j];
        x = norm;
        L2v->gray[i + j] = wgh(x);
    }
    for (i = 0; i < I->ncol - 1; i++)
    {
        dr = I->red[i + j + 1] - I->red[i + j];
        dg = I->green[i + j + 1] - I->green[i + j];
        db = I->blue[i + j + 1] - I->blue[i + j];
        x = norm;
        L2h->gray[i + j] = wgh(x);
        L2v->gray[i + j] = 0.0;
    }
    L2v->gray[i + j] = L2h->gray[i + j] = 0.0;
}

static void inverse(float *D0, float *D1, float *U0, float *Yr, float *Yg,
                    float *Yb, float *Vr, float *Vg, float *Vb, int size)
{
    /* D = LU avec Li,i = 1, Li+1,i != 0 ; Ui,i et Ui,i+1 != 0 */
    /* on a Uii = Dii - Li,i-1Ui-1,i ; Ui,i+1 = Di,i+1
     * et Li+1,i = Di+1,i/Uii = Di,i+1/Uii */

    float Uii, Lii_minus_1;
    int i;

    /* i = 0 ( index = 1 ) */
    Yr[0] = Vr[0];
    Yg[0] = Vg[0];
    Yb[0] = Vb[0];
    Uii = U0[0] = D0[0];

    /* D1[0] = D12 ou D21 ; U1[0] = U12 */
    /* Y[0] = V[0] puis Y[i] = V[i] - Lii_minus_1*Y[i-1] ... */
    for (i = 1; i < size; i++)
    {
        Lii_minus_1 = D1[i - 1] / Uii;
        Yr[i] = Vr[i] - Lii_minus_1 * Yr[i - 1];
        Yg[i] = Vg[i] - Lii_minus_1 * Yg[i - 1];
        Yb[i] = Vb[i] - Lii_minus_1 * Yb[i - 1];
        Uii = U0[i] = D0[i] - Lii_minus_1 * D1[i - 1];
    }                           /* now i = size */
    i--;
    Vr[i] = Yr[i] / U0[i];
    Vg[i] = Yg[i] / U0[i];
    Vb[i] = Yb[i] / U0[i];
    for (i--; i >= 0; i--)
    {
        Vr[i] = (Yr[i] - D1[i] * Vr[i + 1]) / U0[i];
        Vg[i] = (Yg[i] - D1[i] * Vg[i + 1]) / U0[i];
        Vb[i] = (Yb[i] - D1[i] * Vb[i + 1]) / U0[i];
    }
}

void cfdiffuse(float *deltat, float *epsilon, Cfimage in, Cfimage out,
               Fsignal MDiag0, Fsignal MDiag1, Fsignal U0, Cfimage Yimage,
               Cfimage Vimage, Fimage L2h, Fimage L2v)
{
    float *UNorth_r, *UNorth_g, *UNorth_b;
    float *USouth_r, *USouth_g, *USouth_b;
    float *U_r, *U_g, *U_b;
    float *V_r, *V_g, *V_b;

    float omega;
    float east, south, north;
    float var, absvar, maxvar;
    int i, j, k, count;

    /* Allocate the buffers if not allocated */
    MDiag0 = mw_change_fsignal(MDiag0, in->ncol);
    if (MDiag0 == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    MDiag1 = mw_change_fsignal(MDiag1, in->ncol);
    if (MDiag1 == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    U0 = mw_change_fsignal(U0, in->ncol);
    if (U0 == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    Vimage = mw_change_cfimage(Vimage, 1, in->ncol);
    if (Vimage == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    Yimage = mw_change_cfimage(Yimage, 1, in->ncol);
    if (Yimage == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    L2h = mw_change_fimage(L2h, in->nrow, in->ncol);
    if (L2h == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    L2v = mw_change_fimage(L2v, in->nrow, in->ncol);
    if (L2v == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    V_r = Vimage->red;
    V_g = Vimage->green;
    V_b = Vimage->blue;

    /* If the output image 'out' has not been used, set it to be 'in' */
    if (out->red == NULL)
    {
        mwdebug
            ("Null red plane for output image 'out': copy 'in' into 'out'\n");
        out = mw_change_cfimage(out, in->nrow, in->ncol);
        if (out == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");
        mw_copy_cfimage(in, out);
    }
    if ((out->ncol != in->ncol) || (out->nrow != in->nrow))
        mwerror(INTERNAL, 1,
                "[cfdiffuse] Invalide size (%d,%d) for output image\n",
                out->ncol, out->nrow);

    omega = 1.4;
    count = 0;
    set_lambda(in, L2h, L2v, *deltat, *epsilon);

    do
    {
        maxvar = 0.0;
        /* first line */
        U_r = out->red;
        U_g = out->green;
        U_b = out->blue;
        USouth_r = out->red + out->ncol;
        USouth_g = out->green + out->ncol;
        USouth_b = out->blue + out->ncol;

        j = 0;
        east = 0.0;
        for (i = 0; i < out->ncol; i++, USouth_r++, USouth_g++, USouth_b++)
        {
            MDiag0->values[i] = 1.0 + east;     /* i.e. west */
            east = L2h->gray[i + j];
            MDiag1->values[i] = -east;
            south = L2v->gray[i + j];
            V_r[i] = in->red[i + j] + south * (*USouth_r);
            V_g[i] = in->green[i + j] + south * (*USouth_g);
            V_b[i] = in->blue[i + j] + south * (*USouth_b);
            MDiag0->values[i] += south + east;
        }
        inverse(MDiag0->values, MDiag1->values, U0->values,
                Yimage->red, Yimage->green, Yimage->blue, V_r, V_g, V_b,
                out->ncol);

        for (k = out->ncol - 1; k >= 0; k--)
        {
            var = omega * (V_r[k] - U_r[k]);
            absvar = fabs((double) var);
            if (absvar > maxvar)
                maxvar = absvar;
            U_r[k] += var;
            var = omega * (V_g[k] - U_g[k]);
            absvar = fabs((double) var);
            if (absvar > maxvar)
                maxvar = absvar;
            U_g[k] += var;
            var = omega * (V_b[k] - U_b[k]);
            absvar = fabs(var);
            if (absvar > maxvar)
                maxvar = absvar;
            U_b[k] += var;
        }
        /* METTRE 0. DANS LA DERNIERE COLONNE DE l2h ET DANS LA
         * DERNIERE LIGNE DE l2v !! */

        j += out->ncol;
        U_r += out->ncol;
        U_g += out->ncol;
        U_b += out->ncol;
        UNorth_r = out->red;
        UNorth_g = out->green;
        UNorth_b = out->blue;

        for (; j < out->nrow * out->ncol; j += out->ncol,
             U_r += out->ncol, U_g += out->ncol, U_b += out->ncol)
        {
            east = 0.;
            for (i = 0; i < out->ncol; i++,
                 USouth_r++, USouth_g++, USouth_b++,
                 UNorth_r++, UNorth_g++, UNorth_b++)
            {
                MDiag0->values[i] = 1.0 + east;
                north = L2v->gray[i + j - out->ncol];
                south = L2v->gray[i + j];
                east = L2h->gray[i + j];
                MDiag1->values[i] = -east;
                if (south)
                {
                    V_r[i] =
                        in->red[i + j] + north * (*UNorth_r) +
                        south * (*USouth_r);
                    V_g[i] =
                        in->green[i + j] + north * (*UNorth_g) +
                        south * (*USouth_g);
                    V_b[i] =
                        in->blue[i + j] + north * (*UNorth_b) +
                        south * (*USouth_b);
                }
                else
                {
                    V_r[i] = in->red[i + j] + north * (*UNorth_r);
                    V_g[i] = in->green[i + j] + north * (*UNorth_g);
                    V_b[i] = in->blue[i + j] + north * (*UNorth_b);
                }
                MDiag0->values[i] += east + north + south;
            }
            inverse(MDiag0->values, MDiag1->values, U0->values,
                    Yimage->red, Yimage->green, Yimage->blue, V_r, V_g, V_b,
                    out->ncol);

            for (k = out->ncol - 1; k >= 0; k--)
            {
                var = omega * (V_r[k] - U_r[k]);
                absvar = fabs((double) var);
                if (absvar > maxvar)
                    maxvar = absvar;
                U_r[k] += var;
                var = omega * (V_g[k] - U_g[k]);
                absvar = fabs((double) var);
                if (absvar > maxvar)
                    maxvar = absvar;
                U_g[k] += var;
                var = omega * (V_b[k] - U_b[k]);
                absvar = fabs(var);
                if (absvar > maxvar)
                    maxvar = absvar;
                U_b[k] += var;
            }
        }
        mwdebug("maxvar = %f \n", maxvar);
        count++;
        set_lambda(out, L2h, L2v, *deltat, *epsilon);
    }
    while (maxvar > EPSILON);
    mwdebug("count = %d\n", count);
}
