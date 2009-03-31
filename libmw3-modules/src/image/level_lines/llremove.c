/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {llremove};
 version = {"1.1"};
 author = {"Jacques Froment"};
 function = {"Remove small level lines in an gray level image"};
 usage = {
   'i'->invert         "Invert: remove first gray levels local minima
                       and then local maxima",
   'l':[Lmin=10]->Lmin "Minimal level lines length (local gray levels minima)",
   'L':[Lmax=10]->Lmax "Minimal level lines length (local gray levels maxima)",
   input->Input        "Original fimage",
   output<-llremove    "Output fimage with missing level lines"
};
*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for ml_decompose(), ml_reconstruct(),
                                 * ll_remove() */

#define POINT_OK(P,Y,X)  (((P)->x>=0)&&((P)->x<=X)&&((P)->y>=0)&&((P)->y<=Y))
#define BAD_POINT(P,Y,X) (!POINT_OK(P,Y,X))

static void llcheck(Mimage mimage)
{
    Point_curve point;
    Morpho_line ll;
    int NC, NL;

    NC = mimage->ncol;
    NL = mimage->nrow;
    for (ll = mimage->first_ml; ll; ll = ll->next)
    {
        point = ll->first_point;
        while (point != NULL)
        {
            if (BAD_POINT(point, NL, NC))
            {
                mwdebug("point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",
                        point->x, NC, point->y, NL);
                mwerror(WARNING, 0, "[llcheck] Point out of image.\n");
            }
            point = point->next;
        }
    }
}

Fimage llremove(Fimage Input, int *Lmin, int *Lmax, char *invert)
{
    Fimage Output1, Output2 = NULL;
    Mimage mimage = NULL;
    int ml_dec;
    char ml_recons = 1;

    if (invert)
    {
        ml_dec = 1;
        mimage = ml_decompose((Mimage) NULL, &ml_dec, (char *) NULL, Input);
        if (mimage == NULL)
            mwerror(FATAL, 1,
                    "Cannot compute level lines (local minima) "
                    "of input image\n");
        mimage = ll_remove(mimage, (int *) Lmin);

        ml_dec = 0;
        Output1 = ml_reconstruct(&ml_recons, mimage);
        mimage = ml_decompose((Mimage) NULL, &ml_dec, (char *) NULL, Output1);
        if (mimage == NULL)
            mwerror(FATAL, 1,
                    "Cannot compute level lines (local maxima) "
                    "of input image\n");
        mimage = ll_remove(mimage, (int *) Lmax);
        Output2 = ml_reconstruct((char *) NULL, mimage);
        mw_delete_mimage(mimage);
    }
    else
    {
        ml_dec = 0;
        mimage = ml_decompose((Mimage) NULL, &ml_dec, (char *) NULL, Input);
        if (mimage == NULL)
            mwerror(FATAL, 1,
                    "Cannot compute level lines (local maxima) "
                    "of input image\n");
        mimage = ll_remove(mimage, (int *) Lmax);

        fprintf(stderr, "Checking mimage in llremove...\n");
        llcheck(mimage);
        fprintf(stderr, "End of checking mimage\n");

        Output1 = ml_reconstruct((char *) NULL, mimage);
        mw_delete_mimage(mimage);

        ml_dec = 1;
        mimage = ml_decompose((Mimage) NULL, &ml_dec, (char *) NULL, Output1);
        if (mimage == NULL)
            mwerror(FATAL, 1,
                    "Cannot compute level lines (local minima) "
                    "of input image\n");
        mimage = ll_remove(mimage, (int *) Lmin);
        Output2 = ml_reconstruct(&ml_recons, mimage);
    }

    mw_delete_mimage(mimage);
    mw_delete_fimage(Output1);

    return (Output2);
}
