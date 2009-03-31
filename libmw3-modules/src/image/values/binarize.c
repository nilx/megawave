/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {binarize};
 version = {"1.3"};
 author = {"Lionel Moisan"};
 function = {"Binarize an image"};
 usage = {
     't':[t=128.]->t    "threshold value",
     'i'->i             "apply video inverse to output",
     input->in          "input Fimage",
     output<-out        "output Cimage (grey levels 0 and 255 only)"
};
*/
/*----------------------------------------------------------------------
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"

void binarize(Fimage in, Cimage out, float *t, char *i)
{
    int ptr;

    out = mw_change_cimage(out, in->nrow, in->ncol);
    if (!out)
        mwerror(FATAL, 1, "Not enough memory\n");
    if (i)
        for (ptr = in->nrow * in->ncol; ptr--;)
            *(out->gray + ptr) = (*(in->gray + ptr) > *t ? 0 : 255);
    else
        for (ptr = in->nrow * in->ncol; ptr--;)
            *(out->gray + ptr) = (*(in->gray + ptr) > *t ? 255 : 0);
}
