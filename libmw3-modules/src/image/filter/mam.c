/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
  name = {mam};
  version = {"2.25"};
  author = {"Frederic Guichard, Lionel Moisan"};
  function = {"Multiscale Analysis of Movies
             (restoration by using selective directional
             diffusion and motion)"};
  usage = {
     't':[time=0.4]->ptime        "time step",
     'n':[niter=1]->n_iter        "number of iterations",
     'r':[power=0.5]->ppower      "accel power",
     'q':[MAXvit=10]->pMAXvit     "maximal velocity",
     'w':[MINvit=0]->pMINvit      "minimal velocity",
     'a':[fmxa=1]->pfmxa          "maximal acceleration",
     input->in                    "input movie",
     output<-out                  "output movie"
          };
*/
/*----------------------------------------------------------------------
 v2.24: bug corrected in CALCULB (i,j indexes) (JF)
 v2.25 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

#define MinAccel 1.0
#define MaxAccel 256.0
#define SEUIL 0.000001
#define SEUIL2 1.0
#define MAXvitesse 10
#define MAXvitesse2 20          /* 2*MAXvitesse */

#define FPOW(x,y) ((float)pow((double)(x),(double)(y)))

#define MAX(x,y) ( (x)>(y) ? (x) : (y) )
#define MIN(x,y) ( (x)<(y) ? (x) : (y) )
#define ABS(x) ( (x)>0 ? (x) : (-(x)) )

               /* GLOBAL VARIABLES */

static int B[MAXvitesse][MAXvitesse];   /* allowed speeds */
static int BB[MAXvitesse2][MAXvitesse2];

static float *a;                /* original picture  */
static float *curv;             /* courbure spatiale */
static float *grad;             /* gradient (in fact 16xgradient^2) */
static float *inter;            /* stabilisateur */
static float *accel;            /* accel */
static short *ani;              /* ani is a boolean array */
                        /* if ani[...]=0 we use isotropic diffusion, */
                        /* if ani[...]=1 anisotropic diffusion */

static int nx, ny, nz;          /* movie dimensions */
static long adrxyz, adrxy;

                /* AUXILIARY VARIABLES */

static float alpha1, alpha2, alpha3;    /* different powers */
static float q;                 /* accel power */
static short MAXvit2, MINvit2;
static float time1, time2;
static short MAXvit, MINvit, fmxa;

static void RESOLUTION2(void), EVOL(void), CALCULB(void), CALC_CURV(void),
CALC_ACCEL(void);

/*---------------------------------------------------------------------------
                                   M A M
---------------------------------------------------------------------------*/

void mam(Cmovie in, Cmovie out, float *ptime, float *ppower, int *n_iter,
         short int *pMAXvit, short int *pMINvit, short int *pfmxa)
{
    Cimage u, ud;
    int k, l;
    long adr;
    float val;

    time1 = *ptime;
    q = *ppower;
    MAXvit = *pMAXvit;
    MINvit = *pMINvit;
    fmxa = *pfmxa;

    /* Check parameters  */
    if (time1 < 0 || q < 0 || q > 1 || *n_iter < 0)
        mwerror(FATAL, 1, "Parameters are inconsistent\n");

    /* Initialize parameters */
    MINvit2 = MINvit + MINvit + 1;
    MAXvit2 = MAXvit + MAXvit;
    alpha3 = q;
    alpha1 = (1. - q) / 6.;
    alpha2 = (1. - q) / 3.;
    time2 = time1;

    /* Compute allowed velocities */
    CALCULB();

    /* Allocate output movie */
    u = in->first;
    nx = u->ncol;
    ny = u->nrow;
    out->first = ud = mw_change_cimage(NULL, ny, nx);
    ud->previous = NULL;
    nz = 1;
    while (u->next)
    {
        ud->next = mw_change_cimage(NULL, ny, nx);
        (ud->next)->previous = ud;
        ud = ud->next;
        u = u->next;
        nz++;
    }
    ud->next = NULL;

    /* Print parameters */
    mwdebug("Film traite' : %d images %dx%d\n", nz, nx, ny);
    mwdebug("q=%f\n", q);
    mwdebug("Nombre iterations = %i\n", *n_iter);
    mwdebug("time = %f\n", time1);
    mwdebug("vitesse minimale = %i\n", MINvit);
    mwdebug("vitesse maximale = %i\n", MAXvit);
    mwdebug("acceleration maximale = %i\n", fmxa);

    /* Allocate arrays */
    adrxy = (long) nx *(long) ny;
    adrxyz = adrxy * (long) nz;
    a = (float *) malloc(adrxyz * sizeof(float));
    curv = (float *) malloc(adrxyz * sizeof(float));
    grad = (float *) malloc(adrxyz * sizeof(float));
    inter = (float *) malloc(adrxyz * sizeof(float));
    accel = (float *) malloc(adrxyz * sizeof(float));
    ani = (short *) malloc(adrxyz * sizeof(short));

    if (!a || !curv || !grad || !inter || !accel || !ani)
        mwerror(FATAL, 1, "Not enough memory.\n");

    /* Copy input movie into a[] */
    u = in->first;
    for (l = 0; l < nz; l++, u = u->next)
        for (adr = 0; adr < adrxy; adr++)
            a[adrxy * l + adr] = (float) u->gray[adr];

    /* MAIN LOOP */

    for (k = 1; k <= *n_iter; k++)
    {
        printf("iteration= %d / %i\n", k, *n_iter);
        RESOLUTION2();
    }

    /* Save output movie and free memory */

    u = out->first;
    for (l = 0; l < nz; l++, u = u->next)
        for (adr = 0; adr < nx * ny; adr++)
        {
            val = a[l * (nx * ny) + adr];
            if (val < 0.0)
                val = 0.0;
            if (val > 255.0)
                val = 255.0;
            u->gray[adr] = floor(val + .5);
        }

    free(ani);
    free(accel);
    free(inter);
    free(grad);
    free(curv);
    free(a);

}

/*-------------------------------------------------------------------------*/

static void RESOLUTION2(void)
{
    printf("Computing curvature ... ");
    fflush(stdout);
    CALC_CURV();
    printf("ok.\nComputing acceleration ... ");
    fflush(stdout);
    CALC_ACCEL();
    printf("ok.     \nEvolution ...");
    fflush(stdout);
    EVOL();
    printf("ok.\n");
}

/*----------------------------------------*/
/*              EVOL                      */
/*----------------------------------------*/

static void EVOL(void)
{
    register int j, i, l;
    long adr;
    float c;

    for (l = 1; l < nz - 1; l++)
        for (i = 1; i < nx - 1; i++)
            for (j = 1; j < ny - 1; j++)
            {
                adr = i + nx * (j + ny * l);
                c = curv[adr];
                /* Isotropic diffusion is grad(u) is near 0 */
                if (!ani[adr])
                {
                    mwdebug("isotropic: (%d,%d,%d) : %f (c=%f,ac=%f) -> ",
                            i, j, l, a[adr], c, accel[adr]);
                    if (c >= 0.0)
                    {
                        a[adr] += c * FPOW(accel[adr], alpha3) * time2;
                        a[adr] = MIN(a[adr], inter[adr]);
                    }
                    else
                    {
                        a[adr] += c * FPOW(accel[adr], alpha3) * time2;
                        a[adr] = MAX(a[adr], inter[adr]);
                    }
                    mwdebug("%f\n", a[adr]);
                }
                else
                {
                    if (c >= 0.0)
                    {
                        a[adr] += FPOW(grad[adr], alpha1) * FPOW(c, alpha2)
                            * FPOW(accel[adr], alpha3) * time1;
                        a[adr] = MIN(a[adr], inter[adr]);
                    }
                    else
                    {
                        a[adr] -= FPOW(grad[adr], alpha1) * FPOW(-c, alpha2)
                            * FPOW(accel[adr], alpha3) * time1;
                        a[adr] = MAX(a[adr], inter[adr]);
                    }
                }
            }
}

/*-------------------------------------------------------------------------*/

static short calcul(short int x, short int y)
{
    int t;

    t = ((int) x * x) + ((int) y * y);
    if (t < 1)
        return (0);
    else if (t < 4)
        return (1);
    else if (t < 8)
        return (2);
    else if (t < 14)
        return (3);
    else if (t < 21)
        return (4);
    else if (t < 30)
        return (5);
    else if (t < 42)
        return (6);
    else if (t < 54)
        return (7);
    else if (t < 70)
        return (8);
    else if (t < 90)
        return (9);
    else if (t < 110)
        return (10);
    else if (t < 132)
        return (11);
    else if (t < 156)
        return (12);
    else
        return (13);
}

static void CALCULB(void)
{
    register short i, j;
    short vit;

    for (i = 0; i < MAXvitesse; i++)
        for (j = 0; j < MAXvitesse; j++)
        {
            vit = calcul(i, j);
            if (vit >= MINvit && vit <= MAXvit)
                B[i][j] = 1;
            else
                B[i][j] = 0;
        }

    mwdebug("\n ----- Allowed velocities ----- \n\n");
    mwdebug("    ");
    for (i = 0; i < MAXvitesse; i++)
        mwdebug("%i ", i);
    mwdebug("\n \n");

    for (i = 0; i < MAXvitesse; i++)
    {
        mwdebug("%i - ", i);
        for (j = 0; j < (int) MAXvitesse; j++)
            mwdebug("%i ", B[i][j]);
        mwdebug("\n");
    }

    for (i = 0; i < MAXvitesse2; i++)
        for (j = 0; j < MAXvitesse2; j++)
        {
            vit = calcul(i, j);
            if (vit >= MINvit2 && vit <= MAXvit2)
                BB[i][j] = 1;
            else
                BB[i][j] = 0;
        }
}

/*-------------------------------------------------------------------------*/

#define RAC2 1.41421356
#define CONS 0.292893219

static void CALC_CURV(void)
{

    int i, j, l;
    float c1, d1, l0, l1, l2, l3, l4, li, ax, ax2, ay, ay2, az;
    register float a11, amm, am1, a1m, a00, a01, a10, a0m, am0;
    long adr;

    for (l = 0; l < nz; l++)
        for (i = 1; i < nx - 1; i++)
            for (j = 1; j < ny - 1; j++)
            {

                adr = i + nx * (j + ny * l);

                a11 = a[adr + 1 + nx];
                a10 = a[adr + 1];
                a1m = a[adr + 1 - nx];
                a01 = a[adr + nx];
                a00 = a[adr];
                a0m = a[adr - nx];
                am1 = a[adr - 1 + nx];
                am0 = a[adr - 1];
                amm = a[adr - 1 - nx];

                c1 = a11 - amm;
                d1 = am1 - a1m;
                ax = CONS * (a10 - am0 + RAC2 * (c1 - d1));
                ay = CONS * (a01 - a0m + RAC2 * (c1 + d1));
                grad[adr] = (float) sqrt((double) (ax * ax + ay * ay));
                if (grad[adr] < SEUIL2)
                {
                    /* isotropic diffusion */
                    ani[adr] = 0;
                    curv[adr] =
                        ((amm + a1m + am1 + a11) +
                         2. * (a01 + a0m + am0 + a10) - 12. * a00) / 12.;
                    grad[adr] = curv[adr];
                }
                else
                {
                    /* anisotropic diffusion */
                    ani[adr] = 1;
                    az = ax * ay;
                    ax2 = ax * ax;
                    ay2 = ay * ay;
                    li = 1. / (ax2 + ay2);
                    az *= li;
                    ax2 *= li;
                    ay2 *= li;
                    li = az * az;
                    l0 = -2. + 4. * li;
                    l1 = ay2 * (ay2 - ax2);
                    l2 = ax2 * (ax2 - ay2);
                    l3 = li - .5 * az;
                    l4 = l3 + az;
                    curv[adr] = l0 * a00 + l1 * (am0 + a10) + l2 * (a0m + a01)
                        + l3 * (a11 + amm) + l4 * (am1 + a1m);
                }
            }
}

/*-------------------------------------------------------------------------*/

static void CALC_ACCEL(void)
{
    int lx, ly, kx, ky, i, j, l;
    int lx2, ly2, lx1, ly1;
    float c, g, valeur_min, valeur_int, valeur_point;
    register float inter1, inter2, inter3;
    long adr, addr;

/*  Main loop variables

                      +(lx,ly) = (kx,ky)+[-fmxa,fmxa]x[-fmxa,fmxa]
                        /
                       /
                      /
        (i,j) +     (0.0)
                    /
                   /
                -(kx,ky)
*/

    for (l = 1; l < nz - 1; l++)
    {
        printf("%i.", l + 1);
        fflush(stdout);
        for (i = 0; i < nx; i++)
            for (j = 0; j < ny; j++)
            {
                adr = i + nx * (j + ny * l);
                valeur_point = a[adr];
                inter[adr] = valeur_point;
                c = curv[adr];
                g = grad[adr];
                valeur_min = MaxAccel;

                /* Compute accel only if necessary */

                if (ABS(c) < 0.1)
                {
                    accel[adr] = 0.0;
                    inter[adr] = valeur_point;
                }
                else
                {

                    for (kx = -MAXvit; kx <= MAXvit; kx++)
                        for (ky = -MAXvit; ky <= MAXvit; ky++)
                        {

                            /* Test if this velocity is allowed */
                            if (B[abs(kx)][abs(ky)])
                            {

                                if (i - kx >= 0 && i - kx < nx && j - ky >= 0
                                    && j - ky < ny)
                                    inter2 =
                                        a[i - kx +
                                          nx * (j - ky + ny * (l - 1))] -
                                        valeur_point;
                                else
                                    inter2 = 0.0;

                                lx2 = kx + fmxa;
                                if (lx2 + i > nx - 1)
                                    lx2 = nx - 1 - i;
                                ly2 = ky + fmxa;
                                if (ly2 + j > ny - 1)
                                    ly2 = ny - 1 - j;
                                lx1 = kx - fmxa;
                                if (lx1 + i < 0)
                                    lx1 = -i;
                                ly1 = ky - fmxa;
                                if (ly1 + j < 0)
                                    ly1 = -j;

                                for (ly = ly1; ly <= ly2; ly++)
                                {
                                    addr = adr + nx * (ly + ny);
                                    for (lx = lx1; lx <= lx2; lx++)
                                        if (B[abs(lx)][abs(ly)])
                                        {

                                            inter1 =
                                                a[addr + lx] - valeur_point;
                                            inter3 =
                                                g * (float) (abs(kx - lx) +
                                                             abs(ky - ly));
                                            valeur_int =
                                                ABS(inter3) + c >
                                                0.0 ? MAX(inter1,
                                                          inter2) :
                                                -MIN(inter1, inter2);
                                            if (valeur_int < valeur_min)
                                                valeur_min = valeur_int;
                                        }
                                }
                            }
                        }
                    if (valeur_min < 0.0)
                        valeur_min = 0.0;
                    accel[adr] = valeur_min;
                    inter[adr] =
                        c >
                        0.0 ? valeur_point + valeur_min : valeur_point -
                        valeur_min;
                    if (accel[adr] < MinAccel)
                        accel[adr] = 0.0;
                }
            }
    }
}

/*-------------------------------------------------------------------------*/
