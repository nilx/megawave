/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {precond1d};
version = {"1.01"};
author = {"Jean-Pierre D'Ales"};
function = {"Preconditionning of an image"};
usage = {
 'i'->Inverse         "Inverse preconditionning",
 Signal->Signal       "Input signal",
 PrecSignal<-Output   "Preconditionned signal",
 FicEdgeRi->Edge_Ri   "Preconditionning matrices"
        };
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

/* Floating point filter bank */
typedef struct filbank {
    short size;
    float **ri;
    float cnormleft, cnormright;
} *Filbank;

static void INIT_PREC(Fimage edge_ri, Filbank * precleftfil,
                      Filbank * precrightfil, Filbank * unprecleftfil,
                      Filbank * unprecrightfil)
{
    short N;
    short i, j;

    *precleftfil = (Filbank) malloc(sizeof(struct filbank));
    *precrightfil = (Filbank) malloc(sizeof(struct filbank));
    *unprecleftfil = (Filbank) malloc(sizeof(struct filbank));
    *unprecrightfil = (Filbank) malloc(sizeof(struct filbank));

    N = (*precleftfil)->size = (*precrightfil)->size =
        (*unprecleftfil)->size = (*unprecrightfil)->size =
        (long) edge_ri->nrow / 8;

    (*precleftfil)->ri = (float **) malloc(sizeof(float) * N);
    for (i = 0; i < N; i++)
        (*precleftfil)->ri[i] = (float *) malloc(sizeof(float) * N);

    (*precrightfil)->ri = (float **) malloc(sizeof(float) * N);
    for (i = 0; i < N; i++)
        (*precrightfil)->ri[i] = (float *) malloc(sizeof(float) * N);

    (*unprecleftfil)->ri = (float **) malloc(sizeof(float) * N);
    for (i = 0; i < N; i++)
        (*unprecleftfil)->ri[i] = (float *) malloc(sizeof(float) * N);

    (*unprecrightfil)->ri = (float **) malloc(sizeof(float) * N);
    for (i = 0; i < N; i++)
        (*unprecrightfil)->ri[i] = (float *) malloc(sizeof(float) * N);

    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
        {
            (*precleftfil)->ri[i][j] =
                edge_ri->gray[edge_ri->ncol * (4 * N + i) + j];
            (*precrightfil)->ri[i][j] =
                edge_ri->gray[edge_ri->ncol * (5 * N + i) + j];
            (*unprecleftfil)->ri[i][j] =
                edge_ri->gray[edge_ri->ncol * (6 * N + i) + j];
            (*unprecrightfil)->ri[i][j] =
                edge_ri->gray[edge_ri->ncol * (7 * N + i) + j];
        }
}

static void
PRECOND(Fsignal signal, Fsignal output, Filbank LeftMat, Filbank RightMat)
        /*--- (Un)preconditionning of image's edges ---*/
                                /* Input signal */
                                /* Result of preconditionning */
                                        /*--- Matrix of (un)preconditionning
                                 * for left and right edges of signal */
{

    short c, k, N;
    long size;
    double t;
    double *edgeres;

    N = LeftMat->size;
    edgeres = (double *) malloc(sizeof(double) * N);

    size = signal->size;

    for (c = N; c < size - N; c++)
        output->values[c] = signal->values[c];

    /*--- preconditionning of left edge ---*/

    for (c = 0; c < N; c++)
    {
        t = 0.0;
        for (k = 0; k < N; k++)
            t += signal->values[k] * LeftMat->ri[c][k];
        edgeres[c] = t;
    }

    for (c = 0; c < N; c++)
        output->values[c] = edgeres[c];

    /*--- preconditionning of right edge ---*/

    for (c = 0; c < N; c++)
    {
        t = 0.0;
        for (k = 0; k < N; k++)
            t += signal->values[size + k - N] * RightMat->ri[c][k];
        edgeres[c] = t;
    }

    for (c = 0; c < N; c++)
        output->values[size + c - N] = edgeres[c];
}

void precond1d(int *Inverse, Fsignal Signal, Fsignal Output, Fimage Edge_Ri)
        /*----- Applies (un)preconditionning to edges of `Signal` -----*/
                                /* Equal 0 if normal preconditionning
                                 *       1 if inverse       "           */
                                /* Input (Un)preconditionned signal */
                                /* Output signal */
                                /* Buffer containing coefficients of
                                 * preconditionning matrices */
{
    Filbank Precleftfil, Precrightfil, Unprecleftfil, Unprecrightfil;
    /* Coefficients of matrices */

    Output = mw_change_fsignal(Output, Signal->size);
    if (Output == NULL)
        mwerror(FATAL, 1, "Not enough memory for output!\n");

    /*--- Initialisation of preconditionning filter banks ---*/

    INIT_PREC(Edge_Ri, &Precleftfil, &Precrightfil, &Unprecleftfil,
              &Unprecrightfil);

    /*--- (Un)Preconditionning of signal's edges ---*/

    if (Inverse)
        PRECOND(Signal, Output, Unprecleftfil, Unprecrightfil);
    else
        PRECOND(Signal, Output, Precleftfil, Precrightfil);
}
