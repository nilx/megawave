/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cfsharpen};
author = {"Jacques Froment"};
version = {"1.2"};
function = {"Sharpening of a color image using linear filtering"};
usage = {
   'p':[p=75]->p [0,99]  "percent of sharpening",
   'l'->LumOnly          "perform sharpening on the luminance component only",
   A->A                  "Input Cfimage",
   B<-B                  "Output sharpened Cfimage"
};
*/
/*----------------------------------------------------------------------
 v1.1: added -l option (JF)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for cfchgchannels() */

#define SUM(in,out) \
    (*out = (v * *in - u * (*(in-1) + *(in+1) \
                            + *(in-A->ncol) +  *(in+A->ncol) \
                            + *(in-1-A->ncol) + *(in-1+A->ncol) \
                            + *(in+1-A->ncol) + *(in+1+A->ncol))) / d)

Cfimage cfsharpen(Cfimage A, Cfimage B, float *p, char *LumOnly)
{
    float *ar, *ag, *ab, *br, *bg, *bb;
    int x, y, Conv;
    float u, v, d;
    Cfimage T;

    if ((A->nrow < 3) || (A->ncol < 3))
        mwerror(FATAL, 1, "Image too small !\n");
    B = mw_change_cfimage(B, A->nrow, A->ncol);
    if (B == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");

    u = *p / 100.0;
    v = 9.0 - u;
    d = 9.0 * (1.0 - u);

    if (!LumOnly)
    {
        ar = A->red;
        ag = A->green;
        ab = A->blue;
        br = B->red;
        bg = B->green;
        bb = B->blue;

        for (y = 0; y < A->nrow; y++)
            for (x = 0; x < A->ncol; x++)
            {
                if ((x == 0) || (x == A->ncol - 1) || (y == 0)
                    || (y == A->nrow - 1))
                {
                    *br = *ar;
                    *bg = *ag;
                    *bb = *ab;
                }
                else
                {
                    SUM(ar, br);
                    SUM(ag, bg);
                    SUM(ab, bb);
                }
                ar++;
                ag++;
                ab++;
                br++;
                bg++;
                bb++;
            }
    }
    else
    {
        if (A->model != MODEL_RGB)
            mwerror(WARNING, 1,
                    "Input image does not use RGB color model !\n");

        Conv = 2;               /* RGB<->HSV */
        T = mw_new_cfimage();
        cfchgchannels(&Conv, (int *) NULL, (int *) NULL, A, T);
        mw_copy_cfimage(T, B);
        ab = T->blue;
        bb = B->blue;
        for (y = 0; y < T->nrow; y++)
            for (x = 0; x < T->ncol; x++)
            {
                if ((x == 0) || (x == T->ncol - 1) || (y == 0)
                    || (y == T->nrow - 1))
                    *bb = *ab;
                else
                {
                    SUM(ab, bb);
                }
                ab++;
                bb++;
            }
        cfchgchannels(&Conv, &Conv, (int *) NULL, B, T);
        mw_copy_cfimage(T, B);
        mw_delete_cfimage(T);
    }

    return (B);
}
