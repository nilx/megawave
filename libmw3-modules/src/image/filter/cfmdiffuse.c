/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cfmdiffuse};
version = {"1.1"};
author = {"Antonin Chambolle, Jacques Froment"};
function = {"Iterated Diffusion of a Color Float Image
            using Total Variation minimization"};
usage = {
  't':[deltat=10.0]->deltat    "Time of each diffusion",
  'n':[N=10]->N [1,1000]       "Number of diffused images ",
  'l':[epsilon=1.0]->epsilon [0.1,100.0]  "Lower bound for the RGB norm",
  in->in                       "original image (input cfimage)",
  out<-out                     "movie of diffused images (output cfmovie)"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for cfdiffuse() */

void cfmdiffuse(float *deltat, int *N, float *epsilon, Cfimage in,
                Cfmovie out)
{
    Fsignal MDiag0, MDiag1, U0;
    Cfimage Yimage, Vimage;
    Fimage L2h, L2v;
    Cfimage im0, im1;
    int i;

    /* Allocate the buffers arrays for cfdiffuse */
    MDiag0 = mw_change_fsignal(NULL, in->ncol);
    if (MDiag0 == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    MDiag1 = mw_change_fsignal(NULL, in->ncol);
    if (MDiag1 == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    U0 = mw_change_fsignal(NULL, in->ncol);
    if (U0 == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    Vimage = mw_change_cfimage(NULL, 1, in->ncol);
    if (Vimage == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    Yimage = mw_change_cfimage(NULL, 1, in->ncol);
    if (Yimage == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    L2h = mw_change_fimage(NULL, in->nrow, in->ncol);
    if (L2h == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");
    L2v = mw_change_fimage(NULL, in->nrow, in->ncol);
    if (L2v == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    im0 = in;
    for (i = 1; i <= *N; i++)
    {
        mwdebug("Computing image #%d\n", i);
        im1 = mw_change_cfimage(NULL, in->nrow, in->ncol);
        if (im1 == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");
        mw_copy_cfimage(im0, im1);
        cfdiffuse(deltat, epsilon, im0, im1, MDiag0, MDiag1, U0, Yimage,
                  Vimage, L2h, L2v);
        if (i == 1)
            out->first = im1;
        else
        {
            im0->next = im1;
            im1->previous = im0;
        }
        im0 = im1;
    }

    /* Deallocate the buffers arrays */
    mw_delete_fimage(L2v);
    mw_delete_fimage(L2h);
    mw_delete_cfimage(Yimage);
    mw_delete_cfimage(Vimage);
    mw_delete_fsignal(U0);
    mw_delete_fsignal(MDiag1);
    mw_delete_fsignal(MDiag0);
}
