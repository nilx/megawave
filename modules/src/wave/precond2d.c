/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {precond2d};
version = {"1.1"};
author = {"Jean-Pierre D'Ales"};
function = {"(Un)Preconditionning of an image"};
usage = {
    'i'->Inverse       "Inverse preconditionning",
    Image->Image       "Input image",
    PrecImage<-Output  "Preconditionned image",
    FicEdgeRi->Edge_Ri "Preconditionning matrices"
        };
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

/* Floating point filter bank */
typedef struct filbank {
    short size;
    float **ri;
    float cnormleft, cnormright;
} *Filbank;

static void
INIT_PREC(Fimage edge_ri, Filbank * precleftfil, Filbank * precrightfil,
          Filbank * unprecleftfil, Filbank * unprecrightfil)
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

static void PRECOND(Fimage image, Fimage output, Filbank LeftMat,
                    Filbank RightMat)
/* (Un)preconditionning of image's edges ---*/
/* input image ---*/
/* (un)preconditionned image ---*/
/* Matrix of (un)preconditionning for left (upper) and right
 * (lower) edges of image ---*/
{

    short c, l, k, N;
    short dx, dy;
    double tl, tr;
    double *LeftResult, *RightResult;

    N = LeftMat->size;
    LeftResult = (double *) malloc(sizeof(double) * N);
    RightResult = (double *) malloc(sizeof(double) * N);

    dx = output->ncol;
    dy = output->nrow;

    /*--- preconditionning of lower and upper edges of image ---*/

    for (c = 0; c < dx; c++)
    {
        for (l = 0; l < N; l++)
        {
            tl = tr = 0.0;
            for (k = 0; k < N; k++)
            {
                tl += image->gray[(long) dx * k + c] * LeftMat->ri[l][k];
                tr +=
                    image->gray[(long) dx * (dy - N + k) +
                                c] * RightMat->ri[l][k];
            }
            LeftResult[l] = tl;
            RightResult[l] = tr;
        }

        for (l = 0; l < N; l++)
        {
            output->gray[(long) dx * l + c] = LeftResult[l];
            output->gray[(long) dx * (dy - N + l) + c] = RightResult[l];
        }

        for (l = N; l < dy - N; l++)
            output->gray[(long) dx * l + c] = image->gray[(long) dx * l + c];

    }

    /*--- preconditionning on left anf right edges of image ---*/

    for (l = 0; l < dy; l++)
    {
        for (c = 0; c < N; c++)
        {
            tl = tr = 0.0;
            for (k = 0; k < N; k++)
            {
                tl += output->gray[(long) dx * l + k] * LeftMat->ri[c][k];
                tr +=
                    output->gray[(long) dx * (l + 1) + k -
                                 N] * RightMat->ri[c][k];
            }
            LeftResult[c] = tl;
            RightResult[c] = tr;
        }

        for (c = 0; c < N; c++)
        {
            output->gray[(long) dx * l + c] = LeftResult[c];
            output->gray[(long) dx * (l + 1) + c - N] = RightResult[c];
        }

    }

}

void precond2d(int *Inverse, Fimage Image, Fimage Output, Fimage Edge_Ri)
/* Equal 0 if normal preconditionning
 *       1 if inverse       "           */
/* Input image */
/* Output image */
/* Buffer containing coefficients of
 * preconditionning matrices */
{
    Filbank Precleftfil, Precrightfil, Unprecleftfil, Unprecrightfil;

    /*--- Memory allocation for preconditionning Result ---*/

    Output = mw_change_fimage(Output, Image->nrow, Image->ncol);
    if (Output == NULL)
        mwerror(FATAL, 1, "Not enough memory for output!\n");

    /*--- Initialisation of preconditionning filter banks ---*/

    INIT_PREC(Edge_Ri, &Precleftfil, &Precrightfil, &Unprecleftfil,
              &Unprecrightfil);

    /*--- Preconditionning of image's edges (if selected) ---*/

    if (!Inverse)
        PRECOND(Image, Output, Precleftfil, Precrightfil);
    else
        PRECOND(Image, Output, Unprecleftfil, Unprecrightfil);

}
